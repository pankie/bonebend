#include <SDL3/SDL.h>

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

    color_buffer = malloc(sizeof(uint32_t) * WINDOW_WIDTH * WINDOW_HEIGHT);

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

static void clear_color_buffer(const uint32_t color)
{
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
        color_buffer[i] = color;
    }
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

    put_pixel(200, 200, RED);

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
