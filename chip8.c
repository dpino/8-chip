#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <SDL2/SDL.h>

#include "chip8.h"

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
    vm->opcode.value = 0;
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

static inline uint16_t address(union opcode_t opcode)
{
    return opcode.value & 0xFFF;
}

// 00E0: Clear the screen.
static inline void cls(chip8_t *vm) {
    // NYI
}
// 00EE: Return from a subroutine.
static inline void ret(chip8_t *vm) {
    vm->PC = vm->stack[vm->SP];
    vm->SP--;
}

// 0NNN: Call program at address NNN.
static inline void ecall(chip8_t *vm) {
    // NYI.
}

// 1NNN: Goto NNN.
static inline void jmp(chip8_t* vm)
{
    vm->PC = address(vm->opcode);
}

// 2NNN: Call subroutine at NNN.
static inline void call(chip8_t* vm)
{
    vm->SP++;
    vm->stack[vm->SP] = vm->PC;
    vm->PC = address(vm->opcode);
}

// 3XNN: Skip the next instruction if V[X] == NN.
static inline void ske(chip8_t* vm)
{
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t value = vm->opcode.lo;

    if (vm->V[x] == value) {
        vm->PC += 2;
    }
    vm->PC += 2;
}

// 4XNN: Skip the next instruction if V[X] != NN.
static inline void skne(chip8_t* vm)
{
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t value = vm->opcode.lo;

    if (vm->V[x] != value) {
        vm->PC += 2;
    }
    vm->PC += 2;
}

// 5XY0: Skip the next instruction if V[X] == V[Y].
static inline void skre(chip8_t* vm)
{
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t y = vm->opcode.lo;

    if (vm->V[x] == vm->V[y]) {
        vm->PC += 2;
    }
    vm->PC += 2;
}

// 6XNN: Set V[X] to NN.
static inline void load(chip8_t* vm)
{
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t value = vm->opcode.lo;

    vm->V[x] = value;
    vm->PC += 2;
}

// 7XNN: Add NN to V[X].
static inline void add(chip8_t* vm)
{
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t value = vm->opcode.lo;

    vm->V[x] += value;
    vm->PC += 2;
}

// 8XY0 Sets VX to the value of VY.
static inline void setr(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t y = vm->opcode.lo & 0xF0;

    vm->V[x] = vm->V[y];
}

// 8XY1    Sets VX to VX or VY.
static inline void or(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t y = vm->opcode.lo & 0xF0;

    vm->V[x] = vm->V[x] | vm->V[y];
}

// 8XY2    Sets VX to VX and VY.
static inline void and(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t y = vm->opcode.lo & 0xF0;

    vm->V[x] = vm->V[x] & vm->V[y];
}

// 8XY3 Sets VX to VX xor VY.
static inline void xor(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t y = vm->opcode.lo & 0xF0;

    vm->V[x] = vm->V[x] ^ vm->V[y];
}

// 8XY4 Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
static inline void addr(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t y = vm->opcode.lo & 0xF0;
    uint8_t temp = vm->V[x];
    vm->V[x] += vm->V[y];
    vm->V[0xF] = vm->V[x] < temp ? 1 : 0;
}

// 8XY5 VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
static inline void sub(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t y = vm->opcode.lo & 0xF0;

    uint8_t temp = vm->V[x];
    vm->V[x] -= vm->V[y];
    vm->V[0xF] = vm->V[x] > temp ? 1 : 0;
}

// 8XY6 Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
static inline void shr(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xF;

    vm->V[0xF] = vm->V[x] & 0x1 ? 1 : 0;
    vm->V[x] >>= 1;
}

// 8XY7 Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
static inline void subb(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t y = vm->opcode.lo & 0xF0;

    uint8_t temp = vm->V[x];
    vm->V[x] = vm->V[y] - vm->V[x];
    vm->V[0xF] = vm->V[x] > temp ? 1 : 0;
}

// 8XYE Stores the most significant bit of VX in VF and then shifts VX to the left by 1.[b]
static inline void shl(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t y = vm->opcode.lo & 0xF0;

    vm->V[0xF] = vm->V[x] & 0x80 ? 1 : 0;
    vm->V[x] <<= 1;
}

// 9XY0: Skip the next instruction if V[X] != V[Y].
static inline void jneq(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xF;
    const uint8_t y = vm->opcode.lo & 0xF0;

    if (vm->V[x] != vm->V[y]) {
        vm->PC += 2;
    }
    vm->PC += 2;
}

// ANNN: Set I to the address of NNN.
static inline void seti(chip8_t *vm) {
    vm->I = address(vm->opcode);
}

// BNNN: Jump to the address NNN + V[0].
static inline void jmpv0(chip8_t *vm) {
    vm->PC = address(vm->opcode) + vm->V[0];
}

// CXNN: Set V[X] to the result of a bitwise 'and' operation on a random number (0-255) and NN.
static inline void rrand(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xf;
    const uint8_t value = vm->opcode.lo;

    vm->V[x] = random() % 256 & value;
}

// DXYN: Draw a sprite at coordinate (V[X], V[Y]) that has a width of 8 pixels and a height of N pixels.
static inline void draw(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xf;
    const uint8_t y = vm->opcode.lo & 0xf0;
    const uint8_t n = vm->opcode.lo & 0xf;

    vm->V[0xF] = 0;
    for (int row = x; row < x + n; row++) {
        uint8_t *ptr = (uint8_t*) vm->ram + vm->I;
        for (int col = y; col < y + 8; col++) {
            uint8_t oldpixel = vm->vRam[row * 8 + col];
            uint8_t newpixel = oldpixel ^ *(ptr + col);
            vm->vRam[row * 8 + col] = newpixel & 0x1;
            if (oldpixel > 0) {
                vm->V[0xF] = 1;
            }
        }
    }
    vm->vRamChanged = 1;
    vm->PC += 2;
}

static inline void jkey(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xf;

    if (vm->keycode == vm->V[x]) {
        vm->PC += 2;
    }
    vm->PC += 2;
}

static inline void jnkey(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xf;

    if (vm->keycode != vm->V[x]) {
        vm->PC += 2;
    }
    vm->PC += 2;
}

// Sets VX to the value of the delay timer.
static inline void getdelay(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xf;

    vm->V[x] = vm->delay_timer;
}

// A key press is awaited, and then stored in VX. (Blocking Operation. All instruction
// halted until next key event).
static inline void waitkey(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xf;

    unsigned char c = fgetc(stdin);
    vm->V[x] = c;
}

// Sets the delay timer to VX.
static inline void setdelay(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xf;

    vm->delay_timer = vm->V[x];
}

// Sets the sound timer to VX.
static inline void setsound(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xf;

    vm->sound_timer = vm->V[x];
}

// Adds VX to I. VF is set to 1 when there is a range overflow (I+VX>0xFFF), and to
// 0 when there isn't.
static inline void addi(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xf;

    uint8_t old = vm->I;
    vm->I += vm->V[x];
    vm->V[0xF] = (old > vm->I) ? 1 : 0;
}

// Sets I to the location of the sprite for the character in VX. Characters 0x0-0xF
// are represented by a 4x5 font.
static inline void spritei(chip8_t *vm) {
    // NYI:
}

// Stores the binary-coded decimal representation of VX, with the most significant
// of three digits at the address in I, the middle digit at I plus 1, and the least
// significant digit at I plus 2. (In other words, take the decimal representation
// of VX, place the hundreds digit in memory at location in I, the tens digit at
// location I+1, and the ones digit at location I+2.)
static inline void bcd(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xf;

    uint8_t value = vm->V[x];
    vm->ram[vm->I + 0] = value / 100;
    vm->ram[vm->I + 1] = (value / 10) % 10;
    vm->ram[vm->I + 2] = (value % 100) % 10;
}

// Stores V0 to VX (including VX) in memory starting at address I. The offset from I
// is increased by 1 for each value written, but I itself is left unmodified.
static inline void push(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xf;

    for (uint8_t i = 0; i <= x; i++) {
        vm->ram[vm->I + i] = vm->V[i];
    }
}

// Fills V0 to VX (including VX) with values from memory starting at address I. The
// offset from I is increased by 1 for each value written, but I itself is left
// unmodified.
static inline void pop(chip8_t *vm) {
    const uint8_t x = vm->opcode.hi & 0xf;

    for (uint8_t i = 0; i <= x; i++) {
        vm->V[i] = vm->ram[vm->I + i];
    }
}

void chip8_emulateCycle(chip8_t *vm)
{
    vm->opcode.hi = vm->ram[vm->PC] << 8;
    vm->opcode.lo = vm->ram[vm->PC];

    switch (vm->opcode.hi & 0xF0) {
        case 0x0: {
                      const uint8_t lo = vm->opcode.lo;

                      if (lo == 0xE0) {
                          cls(vm);
                      } else if (lo == 0xEE) {
                          ret(vm);
                      } else {
                          ecall(vm);
                      }
                  }
                  break;
        case 0x1: jmp(vm);
                  break;
        case 0x2: call(vm);
                  break;
        case 0x3: ske(vm);
                  break;
        case 0x4: skne(vm);
                  break;
        case 0x5: skre(vm);
                  break;
        case 0x6: load(vm);
                  break;
        case 0x7: add(vm);
                  break;
        case 0x8: {
                      const uint8_t lo = vm->opcode.lo & 0xF;

                      // Bitwise operations.
                      if (lo == 0x0) {
                          setr(vm);
                      } else if (lo == 0x1) {
                          or(vm);
                      } else if (lo == 0x2) {
                          and(vm);
                      } else if (lo == 0x3) {
                          xor(vm);
                      } else if (lo == 0x4) {
                          addr(vm);
                      } else if (lo == 0x5) {
                          sub(vm);
                      } else if (lo == 0x6) {
                          shr(vm);
                      } else if (lo == 0x7) {
                          subb(vm);
                      } else if (lo == 0xE) {
                          shl(vm);
                      } else {
                          // Unreachable.
                      }
                  }
                  break;
        case 0x9: jneq(vm);
                  break;
        case 0xA: seti(vm);
                  break;
        case 0xB: jmpv0(vm);
                  break;
        case 0xC: rrand(vm);
                  break;
        case 0xD: draw(vm);
                  break;
        case 0xE: {
                      const uint8_t lo = vm->opcode.lo;

                      if (lo == 0x9E) {
                          jkey(vm);
                      } else if (lo == 0xA1) {
                          jnkey(vm);
                      } else {
                          // Unreachable.
                      }
                  }
                  break;
        case 0xF: {
                      const uint8_t lo = vm->opcode.lo;

                      if (lo == 0x07) {
                          getdelay(vm);
                      } else if (lo == 0x0A) {
                          waitkey(vm);
                      } else if (lo == 0x15) {
                          setdelay(vm);
                      } else if (lo == 0x18) {
                          setsound(vm);
                      } else if (lo == 0x1E) {
                          addi(vm);
                      } else if (lo == 0x29) {
                          spritei(vm);
                      } else if (lo == 0x33) {
                          bcd(vm);
                      } else if (lo == 0x55) {
                          push(vm);
                      } else if (lo == 0x65) {
                          pop(vm);
                      } else {
                          // Unrecheable.
                      }
                  }
                  break;
        default: {
                     // Unreachable.
                 }
    }
}

void selftest()
{
    chip8_t vm;

    printf("chip8: selftest\n");

    {
        printf("Test jmp: ");

        chip8_initialize(&vm);    
        vm.opcode.value = 0x1AAA;
        jmp(&vm);
        assert(vm.PC == 0xAAA);
        printf("Ok\n");
    }
}

int main(int argc, char* argv[])
{
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-t")) {
            selftest();
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
