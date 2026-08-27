//
// Created by freda on 2026-08-26.
//

#ifndef BONEBEND_RENDERER_H
#define BONEBEND_RENDERER_H

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define BUFFER_SIZE WINDOW_WIDTH * WINDOW_HEIGHT

#include <stdint.h>
#include "triangle.h"
#include "SDL3/SDL_render.h"

extern SDL_Texture* color_buffer_texture;
extern uint32_t* color_buffer;
extern float* z_buffer;

void put_pixel(int32_t x, int32_t y, uint32_t color);
void draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);
void draw_filled_circle(int32_t cx, int32_t cy, int32_t radius, uint32_t color);
void clear_color_buffer(uint32_t color);
void clear_z_buffer(void);
void draw_triangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color);
void fill_flat_bottom_triangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t Mx, int32_t My, uint32_t color);
void fill_flat_top_triangle(int32_t x1, int32_t y1, int32_t Mx, int32_t My, int32_t x2, int32_t y2, uint32_t color);
void draw_filled_triangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color);
void draw_filled_triangle_z_buffer(vec4_t v0, vec4_t v1, vec4_t v2, uint32_t color);
void draw_type_triangle(const triangle_t* triangle, uint32_t color);
uint32_t apply_light_intensity(uint32_t color, float intensity);

#endif //BONEBEND_RENDERER_H
