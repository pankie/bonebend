#include <stdlib.h>
#include <SDL3/SDL.h>

#include "mat4.h"
#include "mesh.h"
#include "skeleton.h"
#include "triangle.h"
#include "vec.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define BUFFER_SIZE WINDOW_WIDTH * WINDOW_HEIGHT

#define FPS 24
#define FRAME_TARGET_TIME (1000 / FPS)
static uint64_t previous_frame_time = 0;

#define RED 0xFFFF0000
#define GREEN 0xFF00FF00
#define BLUE 0xFF0000FF

static SDL_Window *window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* color_buffer_texture = NULL;
static uint32_t* color_buffer = NULL;
static bool is_running = false;
static float time = 0.0f;

#define MAX_TRIANGLES_TO_RENDER 1024
static triangle_t triangles_to_render[MAX_TRIANGLES_TO_RENDER];
static uint32_t num_triangles_to_render = 0;

static mat4_t world_matrix;
static float segment_length = 1.0f;

static bool debug_text_key_map = true;

enum DEBUG_DISPLAY_SETTINGS
{
    KEY_MAP = 0b001,
    BONES   = 0b010,
    MESH    = 0b100
};

static enum DEBUG_DISPLAY_SETTINGS display_settings = KEY_MAP | BONES | MESH;

static bool init_window(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Error initializing SDL: %s", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("bonebend", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window) {
        SDL_Log("Error creating window: %s", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("Error creating renderer: %s", SDL_GetError());
        return false;
    }

    color_buffer = malloc(sizeof(uint32_t) * BUFFER_SIZE);

    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    return true;
}

static void destroy_window(void)
{
    free(color_buffer);
    SDL_DestroyTexture(color_buffer_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static void put_pixel(const int32_t x, const int32_t y, const uint32_t color)
{
    if (x >= 0 && x < WINDOW_WIDTH && y >= 0 && y < WINDOW_HEIGHT)
    {
        color_buffer[y * WINDOW_WIDTH + x] = color;
    }
}

// Bresenham's line algorithm - draws directly into the pixel buffer.
static void draw_line(int32_t x0, int32_t y0, const int32_t x1, const int32_t y1, const uint32_t color) {
    const int32_t dx = abs(x1 - x0);
    const int32_t dy = -abs(y1 - y0);
    const int32_t sx = x0 < x1 ? 1 : -1;
    const int32_t sy = y0 < y1 ? 1 : -1;
    int32_t err = dx + dy;

    while (1) {
        put_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void draw_filled_circle(const int32_t cx, const int32_t cy, const int32_t radius, const uint32_t color)
{
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                put_pixel(cx + x, cy + y, color);
            }
        }
    }
}

static void clear_color_buffer(const uint32_t color)
{
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
        color_buffer[i] = color;
    }
}

// project assumes that we are looking down +Z from the origin, translates scene away from camera by z-component
static vec4_t screen_project(const vec4_t point)
{
    const float fov_factor = 500.0f;
    const float camera_z = 6.0f;

    float z = point.z + camera_z;
    z = fmaxf(z, 0.1f);

    vec4_t projected_point;
    projected_point.x = (fov_factor * point.x) / z + (float) WINDOW_WIDTH / 2;
    projected_point.y = (fov_factor * -point.y) / z + (float) WINDOW_HEIGHT / 2; // flips y so that screen y grows downwards
    projected_point.z = z;
    projected_point.w = point.w;
    return projected_point;
}

static void process_input(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            is_running = false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
        {
            is_running = false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_H)
        {
            display_settings ^= KEY_MAP;
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_B)
        {
            display_settings ^= BONES;
        }
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_M)
        {
            display_settings ^= MESH;
        }


    }
}


static void draw_triangle(const int32_t x0, const int32_t y0, const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2, const uint32_t color)
{
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

static void draw_type_triangle(const triangle_t* triangle, const uint32_t color)
{
    draw_triangle(
        (int32_t) triangle->points[0].x,
        (int32_t) triangle->points[0].y,
        (int32_t) triangle->points[1].x,
        (int32_t) triangle->points[1].y,
        (int32_t) triangle->points[2].x,
        (int32_t) triangle->points[2].y,
        color
    );
}

static void draw_bone(const mat4_t* global_bind_transform, const mat4_t* skin_matrix, const float length, const float width, const uint32_t color)
{
    const float shoulder_height = length * 0.1f;

    const vec3_t points[6] = {
        {0.0f, 0.0f, 0.0f},               // 0: origin
        {0.0f, length, 0.0f},             // 1: tip
        { width, shoulder_height, 0.0f},  // 2
        {-width, shoulder_height, 0.0f},  // 3
        {0.0f, shoulder_height,  width},  // 4
        {0.0f, shoulder_height, -width},  // 5
    };

    const int32_t edge_indices[8][2] = {
        {0, 2}, {0, 3}, {0, 4}, {0, 5},
        {2, 1}, {3, 1}, {4, 1}, {5, 1},
    };

    for (int32_t i = 0; i < 8; i++)
    {
        const vec4_t v_bind0 = mat4_mul_vec4(global_bind_transform, vec4_from_vec3(points[edge_indices[i][0]]));
        const vec4_t v_skinned0 = mat4_mul_vec4(skin_matrix, v_bind0);
        const vec4_t v_world0 = mat4_mul_vec4(&world_matrix, v_skinned0);

        const vec4_t v_bind1 = mat4_mul_vec4(global_bind_transform, vec4_from_vec3(points[edge_indices[i][1]]));
        const vec4_t v_skinned1 = mat4_mul_vec4(skin_matrix, v_bind1);
        const vec4_t v_world1 = mat4_mul_vec4(&world_matrix, v_skinned1);

        const vec4_t p0 = screen_project(v_world0);
        const vec4_t p1 = screen_project(v_world1);

        draw_line((int32_t)p0.x, (int32_t)p0.y, (int32_t)p1.x, (int32_t)p1.y, color);
    }
}

static void render(void)
{
    clear_color_buffer(0xFF000000);

    if (display_settings & MESH)
    {
        for (size_t i = 0; i < num_triangles_to_render; i++)
        {
            draw_type_triangle(&triangles_to_render[i], GREEN);
        }
    }

    if (display_settings & BONES)
    {
        const size_t last_idx = skeleton.bones_count - 1;
        for (size_t i = 0; i < last_idx; i++)
        {
            draw_bone(&skeleton.bones[i].global_bind_transform, &skeleton.skin_matrices[i], segment_length, 0.2f, RED);
        }

        const vec3_t tip = get_bone_position(&skeleton.bones[last_idx].global_bind_transform, &skeleton.skin_matrices[last_idx], &world_matrix);
        const vec4_t p = screen_project(vec4_from_vec3(tip));
        draw_filled_circle((int32_t) p.x,  (int32_t) p.y, 8, RED);
    }

    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, WINDOW_WIDTH * sizeof(uint32_t));
    SDL_RenderTexture(renderer, color_buffer_texture, NULL, NULL);

    if (display_settings & KEY_MAP)
    {
        SDL_SetRenderScale(renderer, 2.0f, 2.0f);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(renderer, 10, 5, "Q - Linear Blend Skinning");
        SDL_RenderDebugText(renderer, 10, 20, "W - Dual Quaternion Linear Blending");
        SDL_RenderDebugText(renderer, 10, 35, "H - Hide text");
        SDL_RenderDebugText(renderer, 10, 50, "B - Hide bones");
        SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    }

    SDL_RenderPresent(renderer);
}

void update(void)
{
    const int32_t time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
    if (time_to_wait > 0 && time_to_wait < FRAME_TARGET_TIME)
    {
        SDL_Delay(time_to_wait);
    }

    const uint64_t ticks = SDL_GetTicks();
    const float dt = (float) (ticks - previous_frame_time) / 1000.0f;
    time += dt;
    previous_frame_time = ticks;

    // mesh.rotation.z += sinf(time) * 0.025f;
    // mesh.translation.x += cosf(time) * 0.025f;

    // rotation and translate the box
    const mat4_t translate_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
    const mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

    world_matrix = mat4_identity();
    world_matrix = mat4_mul_mat4(&rotation_matrix_z, &world_matrix);
    world_matrix = mat4_mul_mat4(&translate_matrix, &world_matrix);

    num_triangles_to_render = 0;
    skeleton_update_pose(skeleton.skin_matrices, time);

    for (size_t i = 0; i < mesh.faces_count; i++)
    {
        const face_t mesh_face = mesh.faces[i];

        const int32_t
            idx_a = mesh_face.a - 1,
            idx_b = mesh_face.b - 1,
            idx_c = mesh_face.c - 1;

        const vec3_t face_vertices[] =
        {
            skin_vertex_lbs(mesh.vertices[mesh_face.a - 1], mesh.bone_a[idx_a], mesh.bone_b[idx_a], mesh.weight_a[idx_a], skeleton.skin_matrices),
            skin_vertex_lbs(mesh.vertices[mesh_face.b - 1], mesh.bone_a[idx_b], mesh.bone_b[idx_b], mesh.weight_a[idx_b], skeleton.skin_matrices),
            skin_vertex_lbs(mesh.vertices[mesh_face.c - 1], mesh.bone_a[idx_c], mesh.bone_b[idx_c], mesh.weight_a[idx_c], skeleton.skin_matrices)
        };

        vec4_t transformed_vertices[3];
        for (size_t j = 0; j < 3; j++)
        {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);
            transformed_vertex = mat4_mul_vec4(&world_matrix, transformed_vertex);
            transformed_vertices[j] = transformed_vertex;
        }

        // project points to screen space
        vec4_t projected_points[3];
        for (size_t j = 0; j < 3; j++)
        {
            projected_points[j] = screen_project(transformed_vertices[j]);
        }

        const triangle_t projected_triangle = {
            .points = {
                        { projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w },
                        { projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w },
                        { projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w }
            }
        };

        triangles_to_render[num_triangles_to_render++] = projected_triangle;
    }
}

int main(void)
{
    const int32_t bone_count = 4;
    const float length = 4.0f;

    segment_length = length / (float) (bone_count - 1);

    is_running = init_window();
    init_mesh(4, length, 0.5f, 0.5f);
    init_skeleton(bone_count, segment_length);
    assign_mesh_weights(bone_count, segment_length);

    if (!is_running)
    {
        return 1;
    }

    previous_frame_time = SDL_GetTicks(); // prevents delta time to be large meaningless spike on first frame
    while (is_running)
    {
        process_input();
        update();
        render();
    }

    destroy_window();
    return 0;
}
