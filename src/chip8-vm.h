#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

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
#define PC_START 0x200
#define NUM_INSTRUCTIONS 35

#define MSB(val) ((val & 0xF0) >> 4)

typedef union {
    struct {
        uint8_t lo, hi;
    };
    uint16_t value;
} opcode_t;

typedef struct {
    uint8_t ram[RAM_MEMORY];
    uint8_t V[NUM_REGISTERS];
    opcode_t opcode;
    uint16_t I;
    uint16_t PC;
    uint8_t vRam[VIDEO_MEMORY];
    uint8_t vRamChanged;
    uint16_t stack[NUM_STACK_FRAMES];
    uint16_t SP;
    uint8_t keycode;
    uint8_t delay_timer;
    uint8_t sound_timer;

    SDL_Renderer *renderer;
    SDL_Window *window;
} chip8_t;

extern const uint16_t opcodes[];
extern const char* instructions[];
extern const size_t instructions_size;
extern const uint8_t num_operands_per_instruction[];

void chip8_emulateCycle(chip8_t *vm);
void chip8_evaluate_opcode(chip8_t *vm);
void chip8_initialize(chip8_t *vm);
void chip8_initialize_vm(chip8_t *vm);
void chip8_loadgame(chip8_t *vm, const char* filename);
void chip8_renderScreen(chip8_t *vm);
