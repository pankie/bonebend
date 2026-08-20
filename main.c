#include <stdlib.h>
#include <SDL3/SDL.h>

#include "vec.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BUFFER_SIZE WINDOW_WIDTH * WINDOW_HEIGHT

#define RED 0xFFFF0000
#define GREEN 0xFF00FF00
#define BLUE 0xFF0000FF

static SDL_Window *window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* color_buffer_texture = NULL;
static uint32_t* color_buffer = NULL;
static bool is_running = false;

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

static void draw_filled_circle(const int32_t cx, const int32_t cy, const int32_t radius, uint32_t color)
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
static vec3_t project(const vec3_t point, const float fov_factor, const float camera_z)
{
    float z = point.z + camera_z;
    z = fmaxf(camera_z, 0.1f);

    vec3_t projected_point;
    projected_point.x = (fov_factor * point.x) / z + (float) WINDOW_WIDTH / 2;
    projected_point.y = (fov_factor * -point.y) / z + (float) WINDOW_HEIGHT / 2; // flips y so that screen y grows downwards
    projected_point.z = z;
    return projected_point;
}

static void process_events(void)
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
    }
}

static void render(void)
{
    clear_color_buffer(0xFF000000);

    // draw_line(0, 0, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1, BLUE);
    // draw_filled_circle(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 25, GREEN);

    float fov_factor = 500.0f;
    float camera_z = 3.0f;

    const vec3_t cube_vertices[] = {
        {-1.0f,  1.0f,  0.0f},  // 0: front-top-left
        { 1.0f,  1.0f,  0.0f},  // 1: front-top-right
        { 1.0f, -1.0f,  0.0f},  // 2: front-bottom-right
        {-1.0f, -1.0f,  0.0f},  // 3: front-bottom-left
        {-1.0f,  1.0f, -1.0f},  // 4: back-top-left
        { 1.0f,  1.0f, -1.0f},  // 5: back-top-right
        { 1.0f, -1.0f, -1.0f},  // 6: back-bottom-right
        {-1.0f, -1.0f, -1.0f}   // 7: back-bottom-left
    };

    const int cube_edges[12][2] = {
        // front face
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        // back face
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        // connecting edges (front to back)
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    for (size_t i = 0; i < 12; i++)
    {
        const vec3_t v0 = cube_vertices[cube_edges[i][0]];
        const vec3_t v1 = cube_vertices[cube_edges[i][1]];

        const vec3_t p0 = project(v0, fov_factor, camera_z);
        const vec3_t p1 = project(v1, fov_factor, camera_z);

        draw_line((int) p0.x, (int) p0.y, (int) p1.x, (int) p1.y, RED);
    }


    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, WINDOW_WIDTH * sizeof(uint32_t));
    SDL_RenderTexture(renderer, color_buffer_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}


int main(void)
{
    is_running = init_window();
    if (!is_running)
    {
        return 1;
    }

    uint64_t start_ticks = SDL_GetTicks();
    while (is_running)
    {
        process_events();

        float time = (float)(SDL_GetTicks() - start_ticks) / 1000.0f;


        render();
    }

    destroy_window();
    return 0;
}
