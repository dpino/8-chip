#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "chip8.h"
#include "util.h"

static void print_addr(uint16_t addr)
{
    fprintf(stdout, "0x%.4x ", addr);
}
static void print_instr(uint16_t value)
{
    opcode_t op;

    op.value = value;
    switch (op.hi & 0xF0) {

    }
    printf("0x%.4x\n", op.value);
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: chip8-disasm <filename>\n");
        exit(1);
    }

    const char* filename = argv[1];
    uint8_t *buffer = readbin(filename);

    uint16_t i = 0;
    while (buffer[i] != '\0') {
        print_addr(PC_START + i);
        print_instr(((uint16_t*) (buffer + i))[0]);
        i += 2;
    }

    return 0;
}
