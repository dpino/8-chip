#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "chip8-vm.h"
#include "parser.h"
#include "util.h"

#define DEBUG 0

typedef struct {
    uint32_t numlines;
    char **lines;
} source_program_t;

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
    instr_t instr = empty_instr;
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
