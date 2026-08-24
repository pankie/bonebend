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
mat4_t bone_draw_matrices[MAX_BONES];

static chain_axis_t current_axis = AXIS_Y;

// radians per second
#define CAMERA_YAW_SPEED (3.1416f * 0.25f)
static float orbit_angle = 0.0f;
static bool LEFT = false, RIGHT = false;

#define RED 0xFFFF0000
#define GREEN 0xFF00FF00
#define BLUE 0xFF0000FF

enum DISPLAY_SETTINGS
{
    TEXT    = 1 << 0,
    BONES   = 1 << 1,
    MESH    = 1 << 2,
    LBS     = 1 << 3, // linear blending skinning, default turned on.
};

static enum DISPLAY_SETTINGS display_settings = TEXT | BONES | MESH | LBS;

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

static void reload_scene(const chain_axis_t axis)
{
    const int32_t bone_count = 4;
    const float length = 4.0f;
    segment_length = length / (float) (bone_count - 1);

    init_mesh(8, length, 0.5f, 0.5f, axis);
    init_skeleton(bone_count, segment_length, axis);
    assign_mesh_weights(bone_count, segment_length, axis);

    current_axis = axis;
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
    projected_point.x = fov_factor * point.x / z + (float) WINDOW_WIDTH / 2;
    projected_point.y = fov_factor * -point.y / z + (float) WINDOW_HEIGHT / 2; // flips y so that screen y grows downwards
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

        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            switch (event.key.key)
            {
                case SDLK_ESCAPE:
                    is_running = false;
                    break;
                case SDLK_H:
                    display_settings ^= TEXT;
                    break;
                case SDLK_B:
                    display_settings ^= BONES;
                    break;
                case SDLK_M:
                    display_settings ^= MESH;
                    break;
                case SDLK_L:
                    display_settings ^= LBS;
                    break;
                case SDLK_T:
                    reload_scene(current_axis == AXIS_Y ? AXIS_Z : AXIS_Y);
                    break;
                case SDLK_LEFT:
                    LEFT = true;
                    break;
                case SDLK_RIGHT:
                    RIGHT = true;
                    break;
                default:
                    break;
            }
        }
        else if (event.type == SDL_EVENT_KEY_UP)
        {
            switch (event.key.key)
            {
                case SDLK_LEFT:
                    LEFT = false;
                    break;
                case SDLK_RIGHT:
                    RIGHT = false;
                    break;
                default:
                    break;
            }
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

static vec3_t bone_point(const chain_axis_t chain_axis, const float along, const float perp1, const float perp2)
{
    return chain_axis == AXIS_Y
        ? (vec3_t){perp1, along, perp2}   // matches build_y_corners' (x, y, z) layout
    : (vec3_t){perp1, perp2, along};  // matches build_z_corners' (x, y, z) layout
}

static void draw_bone(const mat4_t* global_bind_transform, const mat4_t* skin_matrix, const float length, const float width, const uint32_t color)
{
    const float shoulder_height = length * 0.1f;

    const vec3_t points[6] = {
        bone_point(current_axis, 0.0f, 0.0f, 0.0f),              // 0: origin
        bone_point(current_axis, length, 0.0f, 0.0f),            // 1: tip
        bone_point(current_axis, shoulder_height, width, 0.0f),  // 2
        bone_point(current_axis, shoulder_height, -width, 0.0f), // 3
        bone_point(current_axis, shoulder_height, 0.0f, width),  // 4
        bone_point(current_axis, shoulder_height, 0.0f, -width), // 5
    };

    const size_t edge_indices[8][2] = {
        {0, 2}, {0, 3}, {0, 4}, {0, 5},
        {2, 1}, {3, 1}, {4, 1}, {5, 1},
    };

    for (size_t i = 0; i < 8; i++)
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
            draw_bone(&skeleton.bones[i].global_bind_transform, &bone_draw_matrices[i], segment_length, 0.2f, RED);
        }

        const vec3_t tip = get_bone_position(&skeleton.bones[last_idx].global_bind_transform, &bone_draw_matrices[last_idx], &world_matrix);
        const vec4_t p = screen_project(vec4_from_vec3(tip));
        draw_filled_circle((int32_t) p.x,  (int32_t) p.y, 8, RED);
    }

    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, WINDOW_WIDTH * sizeof(uint32_t));
    SDL_RenderTexture(renderer, color_buffer_texture, NULL, NULL);

    if (display_settings & TEXT)
    {
        const char* blend_mode = display_settings & LBS ? "Linear Blending Skinning" : "Dual Quaternion Linear Blending";
        const char* presentation = current_axis == AXIS_Y ? "Elbow" : "Candy Wrapper";
        const float scaling = 2.0f;
        SDL_SetRenderScale(renderer, scaling, scaling);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(renderer, 10, 5, "H - Text");
        SDL_RenderDebugText(renderer, 10, 15, "B - Bones");
        SDL_RenderDebugText(renderer, 10, 25, "L - Blending Mode");
        SDL_RenderDebugText(renderer, 10, 35, "T - Alignment Mode");
        SDL_RenderDebugTextFormat(renderer, 10, WINDOW_HEIGHT / scaling - 15, "Mode: %s", blend_mode);
        SDL_RenderDebugTextFormat(renderer, 10, WINDOW_HEIGHT / scaling - 25, "Presentation: %s", presentation);
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


    float turn_direction = 0.0f;
    if (LEFT)
    {
        turn_direction -= 1.0f;
    }

    if (RIGHT)
    {
        turn_direction += 1.0f;
    }
    orbit_angle += turn_direction * CAMERA_YAW_SPEED * dt;

    // mesh.rotation.z += sinf(time) * 0.025f;
    // mesh.translation.x += cosf(time) * 0.025f;

    // rotation and translate the box
    const mat4_t translate_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
    const mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

    const mat4_t object_world_matrix = mat4_mul_mat4(&translate_matrix, &rotation_matrix_z);

    const mat4_t view_rotation = mat4_make_rotation_y(-orbit_angle);
    world_matrix = mat4_mul_mat4(&view_rotation, &object_world_matrix);

    num_triangles_to_render = 0;
    if (display_settings & LBS)
    {
        skeleton_update_pose(skeleton.skin_matrices, time);
        for (size_t i = 0; i < skeleton.bones_count; i++)
        {
            bone_draw_matrices[i] = skeleton.skin_matrices[i];
        }
    }
    else
    {
        skeleton_update_pose_dual_quat(skeleton.dual_quaternions, time);
        for (size_t i = 0; i < skeleton.bones_count; i++)
        {
            bone_draw_matrices[i] = dual_quat_to_mat4(&skeleton.dual_quaternions[i]);
        }
    }

    for (size_t i = 0; i < mesh.faces_count; i++)
    {
        const face_t mesh_face = mesh.faces[i];

        const int32_t
            idx_a = mesh_face.a - 1,
            idx_b = mesh_face.b - 1,
            idx_c = mesh_face.c - 1;

        const vec3_t face_vertices[] =
        {
            display_settings & LBS
                ? skin_vertex_lbs(mesh.vertices[mesh_face.a - 1], mesh.bone_a[idx_a], mesh.bone_b[idx_a], mesh.weight_a[idx_a], skeleton.skin_matrices)
                : skin_vertex_dqs(mesh.vertices[mesh_face.a - 1], mesh.bone_a[idx_a], mesh.bone_b[idx_a], mesh.weight_a[idx_a], skeleton.dual_quaternions),
            display_settings & LBS
                ? skin_vertex_lbs(mesh.vertices[mesh_face.b - 1], mesh.bone_a[idx_b], mesh.bone_b[idx_b], mesh.weight_a[idx_b], skeleton.skin_matrices)
                : skin_vertex_dqs(mesh.vertices[mesh_face.b - 1], mesh.bone_a[idx_b], mesh.bone_b[idx_b], mesh.weight_a[idx_b], skeleton.dual_quaternions),
            display_settings & LBS
                ? skin_vertex_lbs(mesh.vertices[mesh_face.c - 1], mesh.bone_a[idx_c], mesh.bone_b[idx_c], mesh.weight_a[idx_c], skeleton.skin_matrices)
                : skin_vertex_dqs(mesh.vertices[mesh_face.c - 1], mesh.bone_a[idx_c], mesh.bone_b[idx_c], mesh.weight_a[idx_c], skeleton.dual_quaternions),
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
    is_running = init_window();
    if (!is_running)
    {
        return 1;
    }

    reload_scene(current_axis);
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
