#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "chip8-vm.h"
#include "util.h"

int lookup_operand(uint16_t opcode)
{
    int i = 0;
    for (; i < NUM_INSTRUCTIONS; i++) {
        if (opcodes[i] == opcode) {
            break;
        }
    }
    return (i < NUM_INSTRUCTIONS) ? i : -1;
}

const char* get_keyword(int pos)
{
    assert(pos >= 0 && pos < NUM_INSTRUCTIONS);

    return instructions[pos];
}

int get_num_operands_per_instruction(int pos)
{
    assert(pos >= 0 && pos < NUM_INSTRUCTIONS);

    return num_operands_per_instruction[pos];
}

static void print_addr(uint16_t addr)
{
    fprintf(stdout, "0x%.4x ", addr);
}

static void print_instr(uint16_t value)
{
    opcode_t opcode, ret;

    opcode.value = value;
    uint8_t msb = opcode.hi >> 4;
    switch (msb) {
        case 0x0:
            ret.value = (opcode.lo > 0) ? opcode.lo : 0;
            break;
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        case 0x9:
        case 0xA:
        case 0xB:
        case 0xC:
        case 0xD:
            ret.value = (opcode.hi & 0xF0) << 8;
            break;
        case 0x8:
            ret.value = (opcode.hi & 0xF0) << 8 | opcode.lo & 0x0F;
            break;
        case 0xE:
        case 0xF:
            ret.value = (opcode.hi & 0xF0) << 8 | opcode.lo;
            break;
        default:
            // Unreachable
            break;
    }

    uint8_t pos = lookup_operand(ret.value);
    if (pos == 255) {
        fprintf(stderr, "Unknown instruction: 0x%.4x\n", ret.value);
        return;
    }
    const char* keyword = get_keyword(pos);
    uint8_t num_operands = get_num_operands_per_instruction(pos);

    char line[80];
    if (num_operands == 0) {
        sprintf(line, "%s ", keyword);
    } else if (num_operands == 1) {
        uint16_t op1;
        if (msb == 0 | msb == 1 || msb == 2 || msb == 0xa || msb == 0xb) {
            op1 = opcode.value & 0xFFF;
            sprintf(line, "%s 0x%.3x", keyword, op1);
        } else {
            op1 = opcode.hi & 0x0F;
            sprintf(line, "%s #%x", keyword, op1);
        }
    } else if (num_operands == 2) {
        uint8_t op1, op2;
        if (msb == 3 || msb == 4 || msb == 6 || msb == 7) {
            op1 = opcode.hi & 0xf;
            op2 = opcode.lo;
            sprintf(line, "%s #%x, 0x%.2x",keyword, op1, op2);
        } else {
            op1 = opcode.hi & 0xf;
            op2 = (opcode.lo & 0xf0) >> 4;
            sprintf(line, "%s #%x, #%x", keyword, op1, op2);
        }
    } else if (num_operands == 3) {
            uint8_t op1 = opcode.hi & 0xf;
            uint8_t op2 = opcode.lo >> 4;
            uint8_t op3 = opcode.lo & 0xf;
            sprintf(line, "%s #%x, #%x, 0x%.2x", keyword, op1, op2, op3);
    } else {
        // Unreachable.
    }

    // Adjust tabs.
    char tabs[4];
    if (strlen(line) < 9) {
        strcpy(tabs, "\t\t\t");
    } else if (strlen(line) > 13) {
        strcpy(tabs, "\t");
    } else {
        strcpy(tabs, "\t\t");
    }

    // Print instruction plus comment with the opcode.
    printf("%s", line);
    printf("%s; 0x%x\n", tabs, opcode.value);
}

void selftest() {
    print_instr(0x0000);
    print_instr(0x00E0);
    print_instr(0x00EE);
    print_instr(0x1000);
    print_instr(0x2000);
    print_instr(0x3000);
    print_instr(0x4000);
    print_instr(0x5000);
    print_instr(0x6000);
    print_instr(0x7000);
    print_instr(0x8000);
    print_instr(0x8001);
    print_instr(0x8002);
    print_instr(0x8003);
    print_instr(0x8004);
    print_instr(0x8005);
    print_instr(0x8006);
    print_instr(0x8007);
    print_instr(0x800E);
    print_instr(0x9000);
    print_instr(0xA000);
    print_instr(0xB000);
    print_instr(0xC000);
    print_instr(0xD000);
    print_instr(0xE09E);
    print_instr(0xE0A1);
    print_instr(0xF007);
    print_instr(0xF00A);
    print_instr(0xF015);
    print_instr(0xF018);
    print_instr(0xF01E);
    print_instr(0xF029);
    print_instr(0xF033);
    print_instr(0xF055);
    print_instr(0xF065);
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: chip8-disasm <filename>\n");
        exit(1);
    }

    const char* filename = argv[1];
    uint8_t buffer[4096];
    size_t size = readbin(buffer, filename);


    uint8_t *ptr = buffer, *end = buffer + size;
    while (ptr != end) {
        print_addr(PC_START + (ptr - buffer));
        print_instr(bswap(((uint16_t*) ptr)[0]));
        ptr += 2;
    }

    return 0;
}
