#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "chip8-vm.h"
#include "util.h"

const instr_t empty_instr = {
    keyword: "",
    op1: "",
    op2: "",
    op3: "",
    numops: 0
};

static void parsing_error(const char* errmsg, const char* line, int offset)
{
    fprintf(stderr, "%s: %s\n", errmsg, line);
    fprintf(stdout, "%*c\n", (int) strlen(errmsg) + 2 + offset, '^');
    exit(1);
}

int eol(char c)
{
    return c == '\0' || c == ';';
}

int next_token(char* dest, const char* src, int start, char* delim)
{
    char* ptr = (char*) (src + start);
    int len;

    // Skip whitespace.
    while (*ptr == ' ') ptr++;
    // Mark beginning of token here.
    char* begin = ptr;
    // Move forward while not finding delim character or end of string.
    while (!eol(*ptr)) {
        for (int i = 0; i < strlen(delim); i++) {
            // printf("'%c' == '%c'\n", ptr[0], delim[i]);
            if (ptr[0] == delim[i])
                goto exit;
        }
        ptr++;
    }
    // Copy token.
    exit:
    len = ptr - begin;
    strncpy(dest, begin, len);
    dest[len] = '\0';
    // Return distance between last position and beginning.
    return ptr - (src + start);
}

void assembler_parse_line(instr_t *instr, const char *line)
{
    *instr = empty_instr;

    if (DEBUG) {
        printf("line: %s\n", line);
    }

    char token[8];
    int pos = 0, start = 0;

    pos += next_token(token, line, pos, " \t");
    // Maybe address?
    if (token[0] == '0' && token[1] == 'x') {
        start = pos;
        pos += next_token(token, line, pos, " \t");
    }
    // Must be a keyword.
    if (strlen(token) > 5) {
        parsing_error("Error: Unrecognized keyword", line, pos);
    }
    strcpy(instr->keyword, token);

    // TODO: Verify keyword exists, otherwise return error.
    /* Check is valid keyword.
    int i = 0;
    for (; i < NUM_INSTRUCTIONS; i++) {
        if (!strcmp(instr->keyword, instructions[i])) {
            break;
        }
    }
    if (i == NUM_INSTRUCTIONS) {
        parsing_error("Error: Unrecognized keyword", l, 0);
    }
    */

    // Parse operands.
    int i = 0;
    while (1) {
        pos += next_token(token, line, pos, " \t,");
        if (strlen(token) > 0) {
            strcpy(instr->op[i++], token);
        }
        if (eol(line[pos]))
            break;
        pos++;
    }
    instr->numops = i;

    /*
    // TODO: Check number of operands.
    if (num_operands_per_instruction[i] < instr->numops) {
        parsing_error("Error: Too few operands", line, start);
    } else if (num_operands_per_instruction[i] < instr->numops) {
        parsing_error("Error: Too many operands", line, start);
    }
    */
}

uint16_t tohex(const char* str)
{
    if (*str == '#' || *str == 'v') {
        str++;
    }
    return strtoul(str, NULL, 16);
}

uint16_t assembler_compile_instruction(instr_t* instr)
{
    uint16_t opcode;

    // Find opcode.
    int i = 0;
    while (i < NUM_INSTRUCTIONS) {
        if (!strcmp(instr->keyword, instructions[i])) {
            opcode = opcodes[i];
            break;
        }
        i++;
    }
    if (i == NUM_INSTRUCTIONS) {
        fprintf(stderr, "Illegal instruction: '%s'", instr->keyword);
        exit(1);
    }

    switch (opcode) {
        case 0x00E0: // CLS.
        case 0x00EE: // RET.

        break;

        case 0x1000: // JUMP.
        case 0x2000: // CALL.
        case 0xA000: // LOADI.
        case 0xB000: // JUMPI
            opcode |= tohex(instr->op1);
        break;

        case 0x3000: // SKE.
        case 0x4000: // SKNE.
        case 0x6000: // LOAD.
        case 0x7000: // ADD.
        case 0xC000: // RAND.
            opcode |= tohex(instr->op1) << 8 | tohex(instr->op2);
        break;

        case 0x5000: // SKRE.
        case 0x8000: // MOVE.
        case 0x8001: // OR.
        case 0x8002: // AND.
        case 0x8003: // XOR.
        case 0x8004: // ADDR.
        case 0x8005: // SUB.
        case 0x8006: // SHR.
        case 0x800E: // SHL.
        case 0x9000: { // SKRNE.
            opcode |= tohex(instr->op1) << 8 | tohex(instr->op2) << 4;
        }
        break;

        case 0xE09E: // SKPR.
        case 0xE0A1: // SKUP.
        case 0xF007: // MOVED.
        case 0xF00A: // KEYD.
        case 0xF015: // LOADD.
        case 0xF018: // LOADS.
        case 0xF01E: // ADDI.
        case 0xF029: // LDSPR.
        case 0xF033: // BCD.
        case 0xF055: // STOR.
        case 0xF065: // READ.
            opcode |= tohex(instr->op1) & 0x0F << 8;
        break;

        case 0xD000: {// DRAW.
            opcode = 0xD000 | tohex(instr->op1) << 8 | tohex(instr->op2) << 4 | tohex(instr->op3);
        }
        break;
    }
    return opcode;
}

void dump_instr(const instr_t* instr)
{
    fprintf(stderr, "{ keyword: '%s', op1: '%s', op2: '%s', op3: '%s', numops: %d }\n",
            instr->keyword, instr->op1, instr->op2, instr->op3, instr->numops);
}
