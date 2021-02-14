#pragma once

#define DEBUG 0

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

extern const instr_t empty_instr;

void assembler_parse_line(instr_t *instr, const char *line);
int eol(char c);
int next_token(char* dest, const char* src, int start, char* delim);
