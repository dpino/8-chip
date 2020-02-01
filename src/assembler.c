#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "chip8.h"
#include "util.h"

#define NUM_INSTRUCTIONS 35

typedef struct {
    uint32_t numlines;
    char **lines;
} source_program_t;


typedef struct {
    char keyword[8];
    char op1[4];
    char op2[2];
    char op3[2];
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

static void parsing_error(const char* errmsg, const char* line, int offset)
{
    fprintf(stderr, "%s: %s", errmsg, line);
    fprintf(stdout, "%*c\n", (int) strlen(errmsg) + 2 + offset, '^');
    exit(1);
}

void assemble_parse_line(instr_t *instr, char *l)
{
    *instr = empty;

    // Find end of line.
    char* end = strchr(l, ';') ? strchr(l, ';') : strchr(l, '\0');
    char* pos = l;
    int numops = 0;

    // Parse keyword.
    char* space = strchr(pos, ' ') ? strchr(l, ' ') : end;
    if (space) {
        const size_t len = space - pos;
        if (len > 5) {
            parsing_error("Error: Unrecognized keyword", l, 0);
        }
        strncpy(instr->keyword, pos, len);
    }

    // Parse operands.
    if (space < end) {
        pos = space;
        while (*pos == ' ') pos++;
        char* comma = strchr(pos, ',') ? strchr(pos, ',') : end;
        const size_t len = comma - pos;
        if (len > 4) {
            parsing_error("Error: Operand too long", l, pos - l);
        }
        strncpy(instr->op1, pos, len);
        numops++;

        if (comma < end) {
            pos = comma + 1;
            while (*pos == ' ') pos++;
            char* comma = strchr(pos, ',') ? strchr(pos, ',') : end;
            const size_t len = comma - pos;
            if (len > 2) {
                parsing_error("Error: Operand too long", l, pos - l);
            }
            strncpy(instr->op2, pos, len);
            numops++;

            if (comma < end) {
                pos = comma + 1;
                while (*pos == ' ') pos++;
                const size_t len = end - pos;
                if (len > 2) {
                    parsing_error("Error: Operand too long", l, pos - l);
                }
                strncpy(instr->op3, pos, len);
                numops++;
            }
        }
    }
    instr->numops = numops;

    // Check is valid keyword.
    int i = 0;
    for (; i < NUM_INSTRUCTIONS; i++) {
        if (!strcmp(instr->keyword, instructions[i])) {
            break;
        }
    }
    if (i == NUM_INSTRUCTIONS) {
        parsing_error("Error: Unrecognized keyword", l, 0);
    }

    // Check number of operands.
    if (num_operands_per_instruction[i] < instr->numops) {
        parsing_error("Error: Too few operands", l, 0);
    } else if (num_operands_per_instruction[i] < instr->numops) {
        parsing_error("Error: Too many operands", l, 0);
    }
}

uint16_t assemble_instr(instr_t* instr)
{
    uint16_t opcode;

    // Find opcode.
    int i = 0;
    while (i < NUM_INSTRUCTIONS) {
        // printf("%s: %s\n", instructions[i], instr->keyword);
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

    // printf("assemble_instr\n");
    switch (opcode) {
        case 0x00E0: // CLS.
        case 0x00EE: // RET.

        break;

        case 0x1000: // JUMP.
        case 0x2000: // CALL.
        case 0xA000: // LOADI.
        case 0xB000: // JUMPI
            opcode |= strtoul(instr->op1, NULL, 16);
        break;

        case 0x3000: // SKE.
        case 0x4000: // SKNE.
        case 0x6000: // LOAD.
        case 0x7000: // ADD.
        case 0xC000: // RAND.
            opcode |= strtoul(instr->op1, NULL, 16) && 0xFF << 8 | strtoul(instr->op2, NULL, 16);
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
              opcode |= strtoul(instr->op1, NULL, 16) << 8 | strtoul(instr->op2, NULL, 16) << 4;
              // printf("%.4x, %s, %s (%ld)\n", opcode, instr->op1, instr->op2, strtoul(instr->op2, NULL, 16) << 4);
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
            opcode |= strtoul(instr->op1, NULL, 16) && 0x0F << 8;
        break;

        case 0xD000: // DRAW.
            fprintf(stderr, "NYI: DRAW\n");
        break;
    }
    return opcode;
}

void dump_instr(uint16_t opcode, const instr_t* instr)
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

static void dump_bytecode(const char* fileout, uint16_t* output, size_t size)
{
    FILE* fp = fopen(fileout, "wb+");
    if (!fp) {
        fprintf(stderr, "Couldn't create file: %s", fileout);
        exit(1);
    }

    fwrite(output, sizeof(uint16_t), size, fp);
    fclose(fp);
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: asm <filename>\n");
        exit(1);
    }

    const char* filein = argv[1];
    char* buffer = readtext(filein);

    source_program_t* program = source_program_read(buffer);
    uint16_t output[program->numlines];

    instr_t instr = empty;
    int i = 0;
    for (; i < program->numlines; i++) {
        assemble_parse_line(&instr, program->lines[i]);
        output[i] = assemble_instr(&instr);
        // printf("0x%.4x\n", output[i]);
        dump_instr(output[i], &instr);
    }
    size_t size = i;
    source_program_free(program);

    // Save compiled code to bin file.
    char fileout[256];
    char *pos = strstr(filein, ".chip8");
    if (pos != NULL) {
        strncpy(fileout, filein, pos - filein);
        strcpy(fileout + (pos - filein), ".bin");
        fileout[pos - filein + 4] = '\0';
    } else {
        strcpy(fileout, ".bin");
    }
    dump_bytecode(fileout, output, size);

    return 0;
}
