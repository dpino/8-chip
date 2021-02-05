#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "chip8.h"
#include "util.h"

#define DEBUG 0
#define NUM_INSTRUCTIONS 35

typedef struct {
    uint32_t numlines;
    char **lines;
} source_program_t;

typedef struct {
    char keyword[8];
    union {
        struct {
            char op1[6];
            char op2[6];
            char op3[6];
        };
        char op[6][3];
    };
    uint8_t numops;
} instr_t;

const static instr_t empty = {
    keyword: "",
    op1: "",
    op2: "",
    op3: "",
    numops: 0
};

source_program_t* source_program_read(char* buffer)
{
    source_program_t* ret = (source_program_t*) malloc(sizeof(source_program_t));

    // Read number of lines.
    int numlines = 0;
    for (int i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] == '\n') {
            numlines++;
        }
    }

    // Allocate number of lines.
    ret->numlines = numlines;
    ret->lines = (char**) calloc(sizeof(char*), numlines);

    // Copy content per line.
    int i = 0;
    for (char* ptr = buffer; *ptr != '\0';) {
        char *end = strchr(ptr, '\n');
        size_t len = end - ptr;
        ret->lines[i] = calloc(sizeof(char), len);
        strncpy(ret->lines[i++], ptr, len);
        ptr = end + 1;
    }

    return ret;
}

void source_program_free(source_program_t* p)
{
    free(p->lines);
    free(p);
}

void parsing_error(const char* errmsg, const char* line, int offset)
{
    fprintf(stderr, "%s: %s\n", errmsg, line);
    fprintf(stdout, "%*c\n", (int) strlen(errmsg) + 2 + offset, '^');
    exit(1);
}

static int eol(char c)
{
    return c == '\0' || c == ';';
}

static int next_token(char* dest, const char* src, int start, char* delim)
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
    *instr = empty;

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
    return bswap(opcode);
}

void assembler_dump_instruction(uint16_t opcode, const instr_t* instr)
{
    printf("0x%.4x: ", opcode);
    printf("{'%s'", instr->keyword);
    if (strlen(instr->op1) > 0) {
        printf(", '%s'", instr->op1);
    }
    if (strlen(instr->op2) > 0) {
        printf(", '%s'", instr->op2);
    }
    if (strlen(instr->op3) > 0) {
        printf(", '%s'", instr->op3);
    }
    printf("}\n");
}

size_t assembler_compile_program(uint16_t *output, source_program_t* program)
{
    instr_t instr = empty;
    int i = 0;
    for (; i < program->numlines; i++) {
        assembler_parse_line(&instr, program->lines[i]);
        output[i] = assembler_compile_instruction(&instr);
        if (DEBUG) {
            assembler_dump_instruction(output[i], &instr);
        }
    }
    source_program_free(program);

    return i;
}

void assembler_write_to_file(const char* fileout, uint16_t* output, size_t size)
{
    FILE* fp = fopen(fileout, "wb+");
    if (!fp) {
        fprintf(stderr, "Couldn't create file: %s", fileout);
        exit(1);
    }

    fwrite(output, sizeof(uint16_t), size, fp);
    fclose(fp);
}

char** chip8_parse_line(const char* line)
{
    const size_t token_size = 8;
    const size_t instr_size = 4;

    char token[token_size];
    char** ret = (char**) calloc(instr_size + 1, sizeof(char*));

    int pos = 0, i = 0;
    while (1) {
        pos += next_token(token, line, pos, " ,");
        if (eol(line[pos]))
            break;
        ret[i] = (char*) calloc(strlen(token), sizeof(char));
        strcpy(ret[i++], token);
        pos++;
    }
    ret[i] = NULL;
    return ret;
}

static void print_instr(char* instr[])
{
    printf("{");
    for (char** ptr = instr; *ptr; ptr++) {
        printf("%s", *ptr);
        if (*(ptr + 1)) {
            printf(", ");
        }
    }
    printf("}");
    printf("\n");
}

static void free_instr(char* instr[])
{
    for (char** ptr = instr; *ptr; ptr++) {
        free(*ptr);
    }
    free(instr);
}

static void selftest()
{
    printf("selftest: \n");

    char* line = "0x0200 LOAD #a, 0x02    ; 0x6a02";
    printf("%s\n", line);
    char** instr = chip8_parse_line(line);
    print_instr(instr);
    free_instr(instr);

    exit(0);
}

int main(int argc, char* argv[])
{
    if (argc > 1) {
        for (int i = 0; i < argc; i++) {
            if (!strcmp(argv[i], "-t")) {
                selftest();
            }
        }
    }
    if (!(argc == 2 | argc == 3)) {
        fprintf(stderr, "Usage: asm <filein> [<fileout>]\n");
        exit(1);
    }

    const char* filein = argv[1];
    char fileout[256];
    if (argc == 3) {
        strcpy(fileout, argv[2]);
    } else {
        char *pos = strrchr(filein, '.');
        if (pos != NULL) {
            strncpy(fileout, filein, pos - filein);
            strcpy(fileout + (pos - filein), ".rom");
            fileout[pos - filein + 4] = '\0';
        } else {
            strcpy(fileout, ".rom");
        }
    }

    char* buffer = readtext(filein);

    source_program_t* program = source_program_read(buffer);
    uint16_t output[program->numlines];
    size_t size = assembler_compile_program(output, program);
    assembler_write_to_file(fileout, output, size);
    if (argc != 3) {
        fprintf(stdout, "Generated: %s\n", fileout);
    }

    return 0;
}
