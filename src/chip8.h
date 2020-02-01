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

const uint16_t opcodes[] = {
    0x0000,  0x00E0,  0x00EE, 0x1000,  0x2000, 0x3000,  0x4000, 0x5000,
    0x6000,  0x7000,  0x8000, 0x8001,  0x8002, 0x8003,  0x8004, 0x8005,
    0x8006,  0x8007,  0x800E, 0x9000,  0xA000, 0xB000,  0xC000, 0xD000,
    0xE09E,  0xE0A1,  0xF007, 0xF00A,  0xF015, 0xF018,  0xF01E, 0xF029,
    0xF033,  0xF055,  0xF065
};

const char* instructions[] = {
    "SYS" , "CLS"  , "RET"  , "JUMP", "CALL" , "SKE"  , "SKNE", "SKRE",
    "LOAD", "ADD"  , "MOVE" , "OR"  , "AND"  , "XOR"  , "ADDR", "SUB",
    "SHR" , "SUBB" , "SHL"  , "JNEQ", "LOADI", "JUMPI", "RAND", "DRAW",
    "SKPR", "SKUP" , "MOVED", "KEYD", "LOADD", "LOADS", "ADDI", "LDSPR",
    "BCD" , "PUSH" , "POP"
};

const uint8_t num_operands_per_instruction[] = {
    1, 0, 0, 1, 1, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 1, 1, 1, 2, 3,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1
};