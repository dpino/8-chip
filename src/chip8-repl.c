#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "chip8-vm.h"
#include "parser.h"
#include "util.h"

#define PROMPT "> "

enum Commands {
    SOURCE, DUMP, HELP, SAVE
};

static void help(chip8_t* vm, const char *args);

static void load(chip8_t* vm, const char *args)
{
    char* filename = first_word((char*) args);
    fprintf(stdout, "load %s\n", filename);
}

static void save(chip8_t* vm, const char *args)
{
    fprintf(stdout, "save %s\n", args);
}

static void dump(chip8_t* vm, const char *args)
{
    printf("PC: 0x%x, SP: 0x%x, I: 0x%x\n", vm->PC, vm->SP, vm->I);
    // Count how many registers are set.
    uint8_t count = 0;
    for (int i = 0; i < NUM_REGISTERS; i++) {
        if (vm->V[i] > 0)
            count++;
    }

    // Print registers that have a value set.
    for (int i = 0, j = 0; i < NUM_REGISTERS; i++) {
        if (count > 0 && j == count) {
            printf("\n");
            break;
        }
        if (vm->V[i] > 0) {
            printf("#%x: 0x%x", i, vm->V[i]);
            if (++j < count)
                printf(", ");
        }
        if (count / 2 > 0 && j == count / 2)
            printf("\n");
    }
}

static void eval(chip8_t* vm, const char* line)
{
    instr_t instr = empty_instr;

    assembler_parse_line(&instr, line);
    vm->opcode.value = assembler_compile_instruction(&instr);
    if (DEBUG)
        fprintf(stderr, "opcode: 0x%.4x\n", vm->opcode.value);
    chip8_evaluate_opcode(vm);
}

typedef void(*method_t)(chip8_t* vm, const char*);

typedef struct {
    char* label;
    method_t fn;
    const char* description;
} command_t;

command_t commands[] = {
    { ".dump", dump, "Print out state of VM." },
    { ".help", help, "Print out this help." },
    { ".load", load, "Load a program from a file." },
    { ".save", save, "Save current program to a file." }
};

static void help(chip8_t* vm, const char *args)
{
    size_t size = sizeof(commands) / sizeof(command_t);
    for (int i = 0; i < size; i++) {
        command_t each = commands[i];
        printf("%s\t%s\n", each.label, each.description);
    }
}

command_t* lookup_command(const char *line)
{
    uint8_t n_commands = sizeof(commands) / sizeof(command_t);

    for (int i = 0; i < n_commands; i++) {
        command_t cmd = commands[i];
        if (!strncasecmp(line, cmd.label, strlen(cmd.label))) {
            return &commands[i];
        }
    }
    return NULL;
}

void execute_command(chip8_t *vm, const command_t* cmd, char* line)
{
    char *args = line + strlen(cmd->label);
    cmd->fn(vm, args);
}

void parse_line(chip8_t* vm, char *line, ssize_t len)
{
    command_t* cmd = lookup_command(line);
    if (cmd) {
        execute_command(vm, cmd, line);
    } else {
        eval(vm, line);
    }
}

int main(int argc, char* argv[])
{
    size_t len = 256;
    char filename[len];
    chip8_t vm;

    chip8_initialize_vm(&vm);

    char* line = (char*) calloc(len, sizeof(char));
    ssize_t linesize;

    for (;;) {
        fprintf(stdout, "%s", PROMPT);
        linesize = getline(&line, &len, stdin);
        parse_line(&vm, line, linesize);
    }
    return 0;
}
