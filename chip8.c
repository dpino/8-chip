#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#define RAM_MEMORY 4096
#define NUM_REGISTERS 16
#define VIDEO_WIDTH 64
#define VIDEO_HEIGHT 32
#define VIDEO_MEMORY (VIDEO_WIDTH * VIDEO_HEIGHT)
#define PIXEL_SIZE 16
#define WINDOW_WIDTH (VIDEO_WIDTH * PIXEL_SIZE)
#define WINDOW_HEIGHT (VIDEO_HEIGHT * PIXEL_SIZE)
#define NUM_STACK_FRAMES 16

typedef struct {
    uint8_t ram[RAM_MEMORY];
    uint8_t V[NUM_REGISTERS];
    uint16_t opcode;
    uint16_t I;
    uint16_t PC;
    uint8_t vRam[VIDEO_MEMORY];
    uint16_t stack[NUM_STACK_FRAMES];
    uint16_t SP;
    uint8_t key;

    SDL_Renderer *renderer;
    SDL_Window *window;
} chip8_t;

unsigned char chip8_fontset[80] =
{ 
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

const size_t filesize(FILE* fp)
{
	fseek(fp, 0, SEEK_END);
	size_t pos = ftell(fp);
	rewind(fp);
	return pos;
}

void chip8_loadgame(chip8_t *vm, const char* filename)
{
	FILE *fp;

	fp = fopen(filename, "rb");
	if (!fp) {
		fprintf(stderr, "Could not load game '%s'\n", filename);
		exit(1);
	}

	size_t size = filesize(fp);
	fprintf(stderr, "size: %zd\n", size);
	uint8_t *buffer = (uint8_t*) malloc(size);
	fread(buffer, 1, size, fp);
	for (int i = 0; i < size; i++) {
	    vm->ram[i + 0x80] = buffer[i];
    }
    free(buffer);
    fprintf(stderr, "game loaded: %s\n", filename);
}

void chip8_initialize(chip8_t *vm)
{
    vm->PC = 0x200;
    vm->opcode = 0;
    vm->I = 0;
    vm->SP = 0;

    memset(&vm->vRam, 0, sizeof(vm->vRam));
    memset(&vm->stack, 0, sizeof(vm->stack));
    memset(&vm->V, 0, sizeof(vm->V));
    memset(&vm->ram, 0, sizeof(vm->ram));

    for (int i = 0; i < 80; i++) {
    	vm->ram[i] = chip8_fontset[i];
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &vm->window, &vm->renderer);
    SDL_SetRenderDrawColor(vm->renderer, 0, 0, 0, 255);
    SDL_RenderClear(vm->renderer);
}

static void chip8_setRandomColor(chip8_t *vm)
{
	uint32_t color = random() % (1 << 32 - 1);
	uint8_t r = color >> 16 & 0xff, g = color >> 8 & 0xff, b = color & 0xff;
	SDL_SetRenderDrawColor(vm->renderer, r, g, b, 255);
}

static void chip8_setWhiteColor(chip8_t *vm)
{
	SDL_SetRenderDrawColor(vm->renderer, 255, 255, 255, 255);
}

void chip8_renderPixel(chip8_t *vm, uint8_t x, uint8_t y)
{
	chip8_setWhiteColor(vm);
	for (uint16_t i = y * PIXEL_SIZE; i < (y + 1) * PIXEL_SIZE; i++) {
		for (uint16_t j = x * PIXEL_SIZE; j < (x + 1) * PIXEL_SIZE; j++) {
			SDL_RenderDrawPoint(vm->renderer, j, i);	
		}
	}
}

void chip8_renderScreen(chip8_t *vm)
{
	for (int i = 0; i < VIDEO_HEIGHT; i++) {
		for (int j = 0; j < VIDEO_WIDTH; j++) {
			chip8_renderPixel(vm, j, i);
		}
	}
	SDL_RenderPresent(vm->renderer);
}

void chip8_emulateCycle(chip8_t *vm)
{

}

int main(int argc, char* argv[])
{
	SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    
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
