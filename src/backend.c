#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "backend.h"

color_t colors[] = {
    {0, 0, 0},         // black
    {0, 0, 255},       // blue
    {0, 255, 0},       // green
    {255, 0, 0},       // red
    {255, 255, 255},   // white
};

display_t* create_display(size_t width, size_t height)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Could not init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Window *window = SDL_CreateWindow("My application",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width * PIXEL_SIZE,
            height * PIXEL_SIZE,
            0);
    if (!window) {
        fprintf(stderr, "Could not create window\n");
        exit(1);
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if(!renderer) {
        fprintf(stderr, "Could not create renderer\n");
        exit(1);
    }

    display_t *display = (display_t*) malloc(sizeof(display_t));
    display->window = window;
    display->renderer = renderer;

    return display;
}

void delete_display(display_t *display)
{
    SDL_DestroyWindow(display->window);
    SDL_Quit();
}

void clean_display(display_t *display, color_t color)
{
    SDL_SetRenderDrawColor(display->renderer, color.r, color.g, color.b, 255);
    SDL_RenderClear(display->renderer);
}

void refresh(display_t* display)
{
    SDL_RenderPresent(display->renderer);
}

void draw_pixel(display_t *display, uint16_t x, uint16_t y, color_t c)
{
    SDL_SetRenderDrawColor(display->renderer, c.r, c.g, c.b, 255);
    for (uint16_t i = x * PIXEL_SIZE; i < (x * PIXEL_SIZE) + PIXEL_SIZE; i++) {
        for (uint16_t j = y * PIXEL_SIZE; j < (y * PIXEL_SIZE) + PIXEL_SIZE; j++) {
            SDL_RenderDrawPoint(display->renderer, i, j);
        }
    }
}
