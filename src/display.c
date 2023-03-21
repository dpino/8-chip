#include "backend.h"

int main(void)
{
    display_t *display = create_display(WIDTH, HEIGHT);

    // Clean display.
    clean_display(display, colors[white]);

    // Paint horizontal line on top.
    for (uint16_t i = 0; i < WIDTH; i++) {
        draw_pixel(display, i, 0, colors[red]);
    }

    // Do paint.
    refresh(display);

    SDL_Delay(3000);

    // Close.
    delete_display(display);

    return 0;
}
