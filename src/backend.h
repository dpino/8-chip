#pragma once

#include "SDL.h"

#define WIDTH   64
#define HEIGHT  32

#define PIXEL_SIZE 10

typedef struct display_t {
    SDL_Window *window;
    SDL_Renderer *renderer;
} display_t;

typedef struct color_t {
    uint8_t r, g, b;
} color_t;

enum color_index { black, blue, green, red, white };

extern color_t colors[];

typedef struct pixel_t {
    uint16_t x, y;
    color_t color;
} pixel_t;

display_t* create_display(size_t width, size_t height);
void delete_display(display_t *display);
void clean_display(display_t *display, color_t color);
void refresh(display_t* display);
void draw_pixel(display_t *display, uint16_t x, uint16_t y, color_t c);