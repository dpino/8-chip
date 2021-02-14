#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

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

