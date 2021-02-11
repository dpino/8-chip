#include <stdio.h>
#include <assert.h>

#include "chip8-vm.h"

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

void test(const char* name, test_fn_t test_fn)
{
    chip8_t vm;

    printf("Test %s:\t", name);
    chip8_initialize_vm(&vm);
    test_fn(&vm);
    printf("Ok\n");
}

void selftest()
{
    chip8_t vm;

    printf("chip8: selftest\n");

    test("RET", test_ret);
    test("JUMP", test_jmp);
    test("CALL", test_call);
    test("SKE", test_ske);
    test("SKNE", test_skne);
    test("SKRE", test_skre);
    test("LOAD", test_load);
    test("ADD", test_add);
    test("SETR", test_setr);
    test("OR", test_or);
    test("AND", test_and);
    test("XOR", test_xor);
    test("ADDR", test_addr);
    test("SUB", test_sub);
    test("SHR", test_shr);
    test("SUBB", test_subb);
    test("SHL", test_shl);
    test("JNEQ", test_jneq);
    test("SETI", test_seti);
    test("JMPV0", test_jmpv0);
    test("RRAND", test_rrand);
    test("JKEY", test_jkey);
    test("JNKEY", test_jnkey);
    test("GETDELAY", test_getdelay);
    test("SETDELAY", test_setdelay);
    test("SETSOUND", test_setsound);
    test("ADDI", test_addi);
    test("BCD", test_bcd);
    test("PUSH", test_push);
    test("POP", test_pop);
    // test("WAITKEY", test_waitkey);
    // test("SPRITEI", test_spritei);
}

int main(int argc, char* argv[])
{
    selftest();

    return 0;
}
