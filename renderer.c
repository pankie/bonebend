//
// Created by freda on 2026-08-26.
//

#include "renderer.h"
#include <stdlib.h>

#include "SDL3/SDL_log.h"

SDL_Texture* color_buffer_texture = NULL;
uint32_t* color_buffer = NULL;
float* z_buffer = NULL;

void swap(int32_t* a, int32_t* b)
{
    const int32_t tmp = *a;
    *a = *b;
    *b = tmp;
}

void put_pixel(const int32_t x, const int32_t y, const uint32_t color)
{
    if (x >= 0 && x < WINDOW_WIDTH && y >= 0 && y < WINDOW_HEIGHT)
    {
        color_buffer[y * WINDOW_WIDTH + x] = color;
    }
}

// Bresenham's line algorithm - draws directly into the pixel buffer.
void draw_line(int32_t x0, int32_t y0, const int32_t x1, const int32_t y1, const uint32_t color) {
    const int32_t dx = abs(x1 - x0);
    const int32_t dy = -abs(y1 - y0);
    const int32_t sx = x0 < x1 ? 1 : -1;
    const int32_t sy = y0 < y1 ? 1 : -1;
    int32_t err = dx + dy;

    while (1) {
        put_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        const int32_t e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_filled_circle(const int32_t cx, const int32_t cy, const int32_t radius, const uint32_t color)
{
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                put_pixel(cx + x, cy + y, color);
            }
        }
    }
}

void clear_color_buffer(const uint32_t color)
{
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
        color_buffer[i] = color;
    }
}

void clear_z_buffer(void)
{
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
        z_buffer[i] = 0.0f;
    }
}

void draw_triangle(const int32_t x0, const int32_t y0, const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2, const uint32_t color)
{
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

void fill_flat_bottom_triangle(const int32_t x0, const int32_t y0, const int32_t x1, const int32_t y1, const int32_t Mx, const int32_t My, const uint32_t color)
{
    const float inv_slope_1 = (float) (x1 - x0) / (float) (y1 - y0);
    const float inv_slope_2 = (float) (Mx - x0) / (float) (My - y0);

    float x_start = (float) x0;
    float x_end = (float) x0;
    for (int32_t y = y0; y <= My; y++)
    {
        draw_line( (int32_t) x_start, y,  (int32_t) x_end, y, color);
        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }
}

void fill_flat_top_triangle(const int32_t x1, const int32_t y1, const int32_t Mx, const int32_t My, const int32_t x2, const int32_t y2, const uint32_t color)
{
    const float inv_slope_1 = (float) (x2 - x1) / (float) (y2 - y1);
    const float inv_slope_2 = (float) (x2 - Mx) / (float) (y2 - My);

    float x_start = (float) x2;
    float x_end = (float) x2;
    for (int32_t y = y2; y >= y1; y--)
    {
        draw_line( (int32_t) x_start, y,  (int32_t) x_end, y, color);
        x_start -= inv_slope_1;
        x_end -= inv_slope_2;
    }
}


void draw_filled_triangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t color)
{
    if (y0 > y1)
    {
        swap(&y0, &y1);
        swap(&x0, &x1);
    }

    if (y1 > y2)
    {
        swap(&y1, &y2);
        swap(&x1, &x2);
    }

    if (y0 > y1)
    {
        swap(&y0, &y1);
        swap(&x0, &x1);
    }

    if (y1 == y2)
    {
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
    }
    else if (y0 == y1)
    {
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
    }
    else
    {
        const int32_t My = y1;
        float Mx_float = (float) (x2 - x0);
        Mx_float *= (float) (y1 - y0);
        Mx_float /= (float) (y2 - y0);
        Mx_float += (float) x0;
        const int32_t Mx = (int32_t) Mx_float;

        fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);
        fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
    }
}

void draw_filled_triangle_z_buffer(const vec4_t v0, const vec4_t v1, const vec4_t v2, const uint32_t color)
{
    int32_t x_min = (int32_t) fminf(v0.x, fminf(v1.x, v2.x));
    int32_t x_max = (int32_t) fmaxf(v0.x, fmaxf(v1.x, v2.x));
    int32_t y_min = (int32_t) fminf(v0.y, fminf(v1.y, v2.y));
    int32_t y_max = (int32_t) fmaxf(v0.y, fmaxf(v1.y, v2.y));

    if (x_min < 0) x_min = 0;
    if (y_min < 0) y_min = 0;
    if (x_max > WINDOW_WIDTH - 1)  x_max = WINDOW_WIDTH - 1;
    if (y_max > WINDOW_HEIGHT - 1) y_max = WINDOW_HEIGHT - 1;

    const vec2_t a = {v0.x, v0.y};
    const vec2_t b = {v1.x, v1.y};
    const vec2_t c = {v2.x, v2.y};

    for (int32_t y = y_min; y <= y_max; y++)
    {
        for (int32_t x = x_min; x <= x_max; x++)
        {
            const vec2_t p = { (float) x, (float) y };
            const vec3_t w = barycentric_weights(a, b, c, p);

            if (w.x <0 || w.y < 0 || w.z < 0)
            {
                continue;
            }

            const float interpolated_reciprocal_w = 1.0f / v0.w * w.x + 1.0f / v1.w * w.y + 1.0f / v2.w * w.z;

            const size_t idx = y * WINDOW_WIDTH + x;
            if (interpolated_reciprocal_w > z_buffer[idx])
            {
                z_buffer[idx] = interpolated_reciprocal_w;
                put_pixel(x, y, color);
            }
        }
    }
}

void draw_type_triangle(const triangle_t* triangle, const uint32_t color)
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

uint32_t apply_light_intensity(const uint32_t color, float intensity)
{
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 1.0f) intensity = 1.0f;

    const uint32_t a = (color >> 24) & 0xFF;
    const uint32_t r = (uint32_t)(((color >> 16) & 0xFF) * intensity);
    const uint32_t g = (uint32_t)(((color >> 8) & 0xFF) * intensity);
    const uint32_t b = (uint32_t)((color & 0xFF) * intensity);

    return (a << 24) | (r << 16) | (g << 8) | b;
}