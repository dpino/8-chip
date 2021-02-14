#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "chip8-vm.h"
#include "parser.h"

#define PROMPT "> "

enum Commands {
    SOURCE, DUMP, SAVE
};

static void load(chip8_t* vm, const char *args)
{
    fprintf(stdout, "load %s\n", args);
}

static void save(chip8_t* vm, const char *args)
{
    fprintf(stdout, "save %s\n", args);
}

static void dump(chip8_t* vm, const char *args)
{
    printf("PC: 0x%x, SP: 0x%x, I: 0x%x\n", vm->PC, vm->SP, vm->I);
    for (int i = 0; i < NUM_REGISTERS; i++) {
        printf("V%d: 0x%x", i, vm->V[i]);
        printf(i < NUM_REGISTERS - 1 ? ", " : "\n");
        if (i == 7)
            printf("\n");
    }
}

static void eval(chip8_t* vm, const char* line)
{
    fprintf(stdout, "eval\n");
    instr_t instr = empty_instr;

    assembler_parse_line(&instr, line);

    // TODO: Use assembler to compile instruction.
    // opcode_t opcode = assembler_compile_instruction(&instr);

    opcode_t opcode;
    vm->opcode = opcode;
    chip8_evaluate_opcode(vm);
}

typedef void(*method_t)(chip8_t* vm, const char*);

typedef struct {
    char* label;
    method_t fn;
} command_t;

command_t commands[] = {
    { "dump", dump },
    { "load", load },
    { "save", save }
};

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
