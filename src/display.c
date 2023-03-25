#include "backend.h"
#include <stdarg.h>

uint8_t fonts[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

#define LOG 1

#define MAX_COLUMNS 5
#define FONT_HEIGHT 5
#define FONT_WIDTH 4
#define FONT_SIZE 16

void _log(const char* str,...)
{
#ifdef LOG
    fprintf(stderr, "### LOG(info): %s\n", str);
#endif
}

void draw_font(display_t *display, uint16_t x, uint16_t y, const uint8_t *sprite)
{
    for (size_t col = 0; col < MAX_COLUMNS; col++) {
        uint8_t nibble = sprite[col] >> FONT_WIDTH;
        uint8_t x_offset = x + (FONT_WIDTH - 1);
        for (uint8_t mask = 0x01, i = 0; i < FONT_WIDTH; mask <<= 1, i++) {
            if (nibble & mask) {
                draw_pixel(display, x_offset - i, y, colors[black]);
            }
        }
        y++;
    }

}

void draw_fonts(display_t *display)
{
    uint8_t *sprite;
    uint16_t x = 0, y = 0;

    for (size_t i = 0; i < FONT_SIZE; i++) {
        sprite = (uint8_t*) &fonts[i * MAX_COLUMNS];
        if (i > 0 && i % (FONT_SIZE / 2) == 0) {
            x = 0;
            y = y + (FONT_HEIGHT + 1);
        }
        draw_font(display, x, y, sprite);
        x += (FONT_WIDTH + 1);
    }
}

int main(void)
{
    uint8_t byte = 0xF0;
    uint16_t x = 0, y = 0;
    display_t *display = create_display(WIDTH, HEIGHT);

    // Clean display.
    clean_display(display, colors[white]);

    draw_fonts(display);

    // Paint horizontal line on top.
    for (uint16_t i = 0; i < WIDTH; i++) {
        draw_pixel(display, i, HEIGHT - 1, colors[red]);
    }

    // Do paint.
    refresh(display);

    SDL_Delay(3000);

    // Close.
    delete_display(display);

    return 0;
}
