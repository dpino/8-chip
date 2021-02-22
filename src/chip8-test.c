#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>

#include "chip8-vm.h"
#include "parser.h"

typedef void (*test_fn_t)(chip8_t*);

size_t lookup_instruction(const char* name)
{
    size_t i = 0;
    for (; i < NUM_INSTRUCTIONS; i++) {
        if (!strcmp(name, instructions[i]))
            break;
    }
    if (i == NUM_INSTRUCTIONS) {
        fprintf(stderr, "Couldn't find instruction: %s\n", name);
        exit(1);
    }
    return i;
}

static void print_hex(uint16_t value)
{
    printf("hex: %04x\n", value);
}

void chip8_evaluate_opcode_name(const char* name, chip8_t* vm)
{
    opcode_t opcode;

    size_t pos = lookup_instruction(name);
    opcode.value = opcodes[pos];
    if ((vm->opcode.value & 0x0FFF) != 0) {
        opcode.value = opcode.hi << 8 | vm->opcode.value & 0x0FFF;
    }
    vm->opcode = opcode;
    chip8_evaluate_opcode(vm);
}

void test_ret(chip8_t* vm)
{
    vm->SP = 0;
    vm->stack[vm->SP] = 0x0AAA;
    chip8_evaluate_opcode_name("RET", vm);
    assert(vm->PC == 0xAAA);
}

void test_jmp(chip8_t* vm)
{
    vm->opcode.value = 0x0AAA;
    chip8_evaluate_opcode_name("JUMP", vm);
    assert(vm->PC == 0xAAA);
}

void test_call(chip8_t* vm)
{
    vm->opcode.value = 0x0AAA;
    chip8_evaluate_opcode_name("CALL", vm);
    assert(vm->SP == 1 && vm->stack[vm->SP] == 0x200 && vm->PC == 0xAAA);
}

void test_ske(chip8_t* vm)
{
    vm->opcode.value = 0x30AA;
    vm->V[0] = 0xAA;
    chip8_evaluate_opcode_name("SKE", vm);
    assert(vm->PC = 0x204);
}

void test_skne(chip8_t* vm)
{
    vm->opcode.value = 0x4DAA;
    vm->V[0xD] = 0xBB;
    chip8_evaluate_opcode_name("SKNE", vm);
    assert(vm->PC = 0x204);
}

void test_skre(chip8_t* vm)
{
    vm->opcode.value = 0x5DC0;
    vm->V[0xD] = 0x80;
    vm->V[0xC] = 0x80;
    chip8_evaluate_opcode_name("SKRE", vm);
    assert(vm->PC == 0x204);
}

void test_load(chip8_t* vm)
{
    vm->opcode.value = 0x60AA;
    chip8_evaluate_opcode_name("LOAD", vm);
    assert(vm->PC == 0x202 && vm->V[0] == 0xAA);
}

void test_add(chip8_t* vm)
{
    vm->opcode.value = 0x70FE;
    vm->V[0] = 1;
    chip8_evaluate_opcode_name("ADD", vm);
    assert(vm->PC == 0x202 && vm->V[0] == 0xFF);
}

void test_setr(chip8_t* vm)
{
    vm->opcode.value = 0x8120;
    vm->V[1] = 0x00;
    vm->V[2] = 0xFF;
    chip8_evaluate_opcode_name("MOVE", vm);
    assert(vm->PC == 0x202 && vm->V[1] == 0xFF);
}

void test_or(chip8_t* vm)
{
    vm->opcode.value = 0x8121;
    vm->V[1] = 0x01;
    vm->V[2] = 0xFE;
    chip8_evaluate_opcode_name("OR", vm);
    assert(vm->PC == 0x202 && vm->V[1] == 0xFF);
}

void test_and(chip8_t* vm)
{
    vm->opcode.value = 0x8122;
    vm->V[1] = 0x01;
    vm->V[2] = 0xFF;
    chip8_evaluate_opcode_name("AND", vm);
    assert(vm->PC == 0x202 && vm->V[1] == 0x01);
}

void test_xor(chip8_t* vm)
{
    vm->opcode.value = 0x8123;
    vm->V[1] = 0x0F;
    vm->V[2] = 0xF0;
    chip8_evaluate_opcode_name("XOR", vm);
    assert(vm->PC == 0x202 && vm->V[1] == 0xFF);
}

void test_addr(chip8_t* vm)
{
    vm->opcode.value = 0x8014;
    vm->V[0] = 0xFF;
    vm->V[1] = 0xFF;
    chip8_evaluate_opcode_name("ADDR", vm);
    assert(vm->PC == 0x202 && vm->V[0] == 0xFE && vm->V[0xF] == 1);
}

void test_sub(chip8_t* vm)
{
    vm->opcode.value = 0x8015;
    vm->V[0] = 0x01;
    vm->V[1] = 0x02;
    chip8_evaluate_opcode_name("SUB", vm);
    assert(vm->PC == 0x202 && vm->V[0] == 0xFF && vm->V[0xF] == 0x1);
}

void test_shr(chip8_t* vm)
{
    vm->opcode.value = 0x8006;
    vm->V[0] = 0xFF;
    chip8_evaluate_opcode_name("SHR", vm);
    assert(vm->PC == 0x202 && vm->V[0] == 0x7F);
}

void test_subb(chip8_t* vm)
{
    vm->opcode.value = 0x8127;
    vm->V[1] = 0xFF;
    vm->V[2] = 0xFF;
    chip8_evaluate_opcode_name("SUBB", vm);
    assert(vm->PC == 0x202 && vm->V[1] == 0x0);
}

void test_shl(chip8_t* vm)
{
    vm->opcode.value = 0x800E;
    vm->V[0] = 0xFF;
    chip8_evaluate_opcode_name("SHL", vm);
    assert(vm->PC == 0x202 && vm->V[0] == 0xFE);
}

void test_jneq(chip8_t* vm)
{
    vm->opcode.value = 0x9010;
    vm->V[0] = 0xAA;
    vm->V[1] = 0xAB;
    chip8_evaluate_opcode_name("JNEQ", vm);
    assert(vm->PC == 0x204);
}

void test_seti(chip8_t* vm)
{
    vm->opcode.value = 0xABBB;
    chip8_evaluate_opcode_name("LOADI", vm);
    assert(vm->PC == 0x202 && vm->I == 0xBBB);
}

void test_jmpv0(chip8_t* vm)
{
    vm->opcode.value = 0xBC00;
    vm->V[0] = 0xCC;
    chip8_evaluate_opcode_name("JUMPI", vm);
    assert(vm->PC == 0xCCC);
}

void test_rrand(chip8_t* vm)
{
    vm->opcode.value = 0xC0FF;
    chip8_evaluate_opcode_name("RAND", vm);
    assert(vm->PC == 0x202 && vm->V[0] > 0 && vm->V[0] <= 0xFF);
}

void test_jkey(chip8_t* vm)
{
    vm->opcode.value = 0xE09E;
    vm->keycode = 'A';
    vm->V[0] = 'A';
    chip8_evaluate_opcode_name("SKPR", vm);
    assert(vm->PC == 0x204 && vm->keycode == vm->V[0]);
}

void test_jnkey(chip8_t* vm)
{
    vm->opcode.value = 0xE0A1;
    vm->keycode = 'A';
    vm->V[0] = 'B';
    chip8_evaluate_opcode_name("SKUP", vm);
    assert(vm->PC == 0x204 && vm->keycode != vm->V[0]);
}

void test_getdelay(chip8_t* vm)
{
    vm->opcode.value = 0xF007;
    vm->delay_timer = 255;
    chip8_evaluate_opcode_name("MOVED", vm);
    assert(vm->PC == 0x202 && vm->V[0] == 255);
}

void test_setdelay(chip8_t* vm)
{
    vm->opcode.value = 0xF015;
    vm->V[0] = 255;
    chip8_evaluate_opcode_name("LOADD", vm);
    assert(vm->PC == 0x202 && vm->delay_timer == 255);
}

void test_setsound(chip8_t* vm)
{
    vm->opcode.value = 0xF018;
    vm->V[0] = 255;
    chip8_evaluate_opcode_name("LOADS", vm);
    assert(vm->PC == 0x202 && vm->sound_timer == 255);
}

void test_addi(chip8_t* vm)
{
    vm->opcode.value = 0xF01E;
    vm->V[0] = 0x1;
    vm->I = 0x0FFF;
    chip8_evaluate_opcode_name("ADDI", vm);
    assert(vm->PC == 0x202 && vm->I == 0x0 && vm->V[0xF] == 1);
}

void test_bcd(chip8_t* vm)
{
    vm->opcode.value = 0xF033;
    vm->V[0] = 255;
    chip8_evaluate_opcode_name("BCD", vm);
    assert(vm->PC == 0x202 &&
        vm->ram[vm->I] == 2 &&
        vm->ram[vm->I + 1] == 5 &&
        vm->ram[vm->I + 2] == 5);
}

void test_push(chip8_t* vm)
{
    vm->opcode.value = 0xFF55;
    vm->I = 0;
    for (int i = 0; i < 16; i++) {
        vm->V[i] = i;
    }
    chip8_evaluate_opcode_name("PUSH", vm);
    assert(vm->PC == 0x202);
    for (int i = 0; i < 16; i++) {
        assert(vm->ram[vm->I + i] == i);
    }
}

void test_pop(chip8_t* vm)
{
    vm->opcode.value = 0xFF65;
    vm->I = 0;
    for (int i = 0; i < 16; i++) {
        vm->ram[vm->I + i] = i;
    }
    chip8_evaluate_opcode_name("POP", vm);
    assert(vm->PC == 0x202);
    for (int i = 0; i < 16; i++) {
        assert(vm->V[i] == i);
    }
}

void test_waitkey(chip8_t* vm)
{
    chip8_evaluate_opcode_name("KEYD", vm);
    assert(vm->PC == 0x202);
}

void test_spritei(chip8_t* vm)
{
    chip8_evaluate_opcode_name("LDSPR", vm);
    assert(vm->PC == 0x202);
}

void test_opcode(const char* name, test_fn_t test_fn)
{
    chip8_t vm;

    printf("Test %s:\t", name);
    chip8_initialize_vm(&vm);
    test_fn(&vm);
    printf("Ok\n");
}

bool instr_equals(const instr_t* actual, const instr_t* expected)
{
    return strcmp(actual->keyword, expected->keyword) == 0 &&
           strcmp(actual->op1, expected->op1) == 0 &&
           strcmp(actual->op2, expected->op2) == 0 &&
           strcmp(actual->op3, expected->op3) == 0 &&
           actual->numops == expected->numops;
}

void print_instr(const instr_t *instr)
{
    printf("{ keyword: '%s', ", instr->keyword);
    for (int i = 0; i < 3; i++) {
        size_t len = strlen(instr->op[i]);
        if (len > 0)
            printf("op%d: '%s', ", i, instr->op[i]);
    }
    if (instr->numops > 0)
        printf("numops: %d }", instr->numops);
    printf("\n");
}

void test_parse_line(const char* line, const instr_t* expected)
{
    instr_t actual;

    printf("Parse '%s': ", line);
    assembler_parse_line(&actual, line);
    if (instr_equals(&actual, expected)) {
        print_instr(&actual);
    } else {
        printf("Error\n");
        print_instr(&actual);
        print_instr(expected);
        exit(1);
    }
}

void fatal(const char* fmt, ...)
{
    va_list argv;

    va_start(argv, fmt);
    vfprintf(stderr, fmt, argv);
    va_end(argv);
    exit(1);
}

instr_t* instruction(char* arg1, ...)
{
    static instr_t ret;
    memset(&ret, 0, sizeof(instr_t));

    va_list argv;
    va_start(argv, arg1);

    int numops = 0;
    char* arg = arg1;
    while (arg != NULL) {
        if (!strcmp(arg, "keyword")) {
            arg = va_arg(argv, char*);
            strcpy(ret.keyword, arg);
        } else if (!strcmp(arg, "op1")) {
            arg = va_arg(argv, char*);
            strcpy(ret.op1, arg);
            numops++;
        } else if (!strcmp(arg, "op2")) {
            arg = va_arg(argv, char*);
            strcpy(ret.op2, arg);
            numops++;
        } else if (!strcmp(arg, "op3")) {
            arg = va_arg(argv, char*);
            strcpy(ret.op3, arg);
            numops++;
        } else {
            fatal("Unrecognized parameter: %s", arg);
        }
        arg = va_arg(argv, char*);
    }
    va_end(argv);

    ret.numops = numops;
    return &ret;
}

void opcode_tests()
{
    printf("Opcode tests\n");

    test_opcode("RET", test_ret);
    test_opcode("JUMP", test_jmp);
    test_opcode("CALL", test_call);
    test_opcode("SKE", test_ske);
    test_opcode("SKNE", test_skne);
    test_opcode("SKRE", test_skre);
    test_opcode("LOAD", test_load);
    test_opcode("ADD", test_add);
    test_opcode("SETR", test_setr);
    test_opcode("OR", test_or);
    test_opcode("AND", test_and);
    test_opcode("XOR", test_xor);
    test_opcode("ADDR", test_addr);
    test_opcode("SUB", test_sub);
    test_opcode("SHR", test_shr);
    test_opcode("SUBB", test_subb);
    test_opcode("SHL", test_shl);
    test_opcode("JNEQ", test_jneq);
    test_opcode("SETI", test_seti);
    test_opcode("JMPV0", test_jmpv0);
    test_opcode("RRAND", test_rrand);
    test_opcode("JKEY", test_jkey);
    test_opcode("JNKEY", test_jnkey);
    test_opcode("GETDELAY", test_getdelay);
    test_opcode("SETDELAY", test_setdelay);
    test_opcode("SETSOUND", test_setsound);
    test_opcode("ADDI", test_addi);
    test_opcode("BCD", test_bcd);
    test_opcode("PUSH", test_push);
    test_opcode("POP", test_pop);
    // test_opcode("WAITKEY", test_waitkey);
    // test_opcode("SPRITEI", test_spritei);
}


void parsing_tests()
{
    printf("\nParsing tests\n");

    test_parse_line("LOAD #a, 0x02", instruction(
        "keyword", "LOAD",
        "op1", "#a",
        "op2", "0x02", NULL
    ));
    test_parse_line("LOADI 0x2ea", instruction(
        "keyword", "LOADI",
        "op1", "0x2ea", NULL
    ));
    test_parse_line("DRAW #a, #b, 0x06", instruction(
        "keyword", "DRAW",
        "op1", "#a",
        "op2", "#b",
        "op3", "0x06", NULL
    ));
    test_parse_line("CALL 0x2d4", instruction(
        "keyword", "CALL",
        "op1", "0x2d4", NULL
    ));
    test_parse_line("LOADD #0", instruction(
        "keyword", "LOADD",
        "op1", "#0", NULL
    ));
    test_parse_line("MOVED #0", instruction(
        "keyword", "MOVED",
        "op1", "#0", NULL
    ));
    test_parse_line("SKE #0, 0x00", instruction(
        "keyword", "SKE",
        "op1", "#0",
        "op2", "0x00", NULL
    ));
    test_parse_line("JUMP 0x21a", instruction(
        "keyword", "JUMP",
        "op1", "0x21a", NULL
    ));
    test_parse_line("RAND #7, #1", instruction(
        "keyword", "RAND",
        "op1", "#7",
        "op2", "#1", NULL
    ));
    test_parse_line("ADD #7, 0x08", instruction(
        "keyword", "ADD",
        "op1", "#7",
        "op2", "0x08", NULL
    ));
}

int main(int argc, char* argv[])
{
    printf("chip8: selftest\n");

    opcode_tests();
    parsing_tests();

    printf("chip8: Ok\n");

    return 0;
}
