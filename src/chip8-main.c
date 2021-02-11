#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "chip8-vm.h"

int main(int argc, char* argv[])
{
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-t")) {
            // selftest();
            exit(0);
        }
    }

    chip8_t vm;

    chip8_initialize(&vm);
    chip8_loadgame(&vm, "roms/pong.rom");

    chip8_renderScreen(&vm);

    for (;;) {
        chip8_emulateCycle(&vm);

        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
