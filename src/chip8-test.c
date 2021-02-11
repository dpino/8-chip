#include <stdio.h>
#include <assert.h>

#include "chip8-vm.h"

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

void selftest()
{
    chip8_t vm;

    printf("chip8: selftest\n");

    printf("Test ret:\t");
    {
        chip8_initialize_vm(&vm);
        vm.SP = 0;
        vm.stack[vm.SP] = 0x0AAA;
        chip8_evaluate_opcode_name("RET", &vm);
        assert(vm.PC == 0xAAA);
        printf("Ok\n");
    }

    printf("Test jmp:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x0AAA;
        chip8_evaluate_opcode_name("JUMP", &vm);
        assert(vm.PC == 0xAAA);
        printf("Ok\n");
    }

    printf("Test call:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x0AAA;
        chip8_evaluate_opcode_name("CALL", &vm);
        assert(vm.SP == 1 && vm.stack[vm.SP] == 0x200 && vm.PC == 0xAAA);
        printf("Ok\n");
    }

    printf("Test ske:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x30AA;
        vm.V[0] = 0xAA;
        chip8_evaluate_opcode_name("SKE", &vm);
        assert(vm.PC = 0x204);
        printf("Ok\n");
    }

    printf("Test skne:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x4DAA;
        vm.V[0xD] = 0xBB;
        chip8_evaluate_opcode_name("SKNE", &vm);
        assert(vm.PC = 0x204);
        printf("Ok\n");
    }

    printf("Test skre:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x5DC0;
        vm.V[0xD] = 0x80;
        vm.V[0xC] = 0x80;
        chip8_evaluate_opcode_name("SKRE", &vm);
        assert(vm.PC == 0x204);
        printf("Ok\n");
    }

    printf("Test load:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x60AA;
        chip8_evaluate_opcode_name("LOAD", &vm);
        assert(vm.PC == 0x202 && vm.V[0] == 0xAA);
        printf("Ok\n");
    }

    printf("Test add:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x70FE;
        vm.V[0] = 1;
        chip8_evaluate_opcode_name("ADD", &vm);
        assert(vm.PC == 0x202 && vm.V[0] == 0xFF);
        printf("Ok\n");
    }

    printf("Test setr:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x8120;
        vm.V[1] = 0x00;
        vm.V[2] = 0xFF;
        chip8_evaluate_opcode_name("MOVE", &vm);
        assert(vm.PC == 0x202 && vm.V[1] == 0xFF);
        printf("Ok\n");
    }

    printf("Test or:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x8121;
        vm.V[1] = 0x01;
        vm.V[2] = 0xFE;
        chip8_evaluate_opcode_name("OR", &vm);
        assert(vm.PC == 0x202 && vm.V[1] == 0xFF);
        printf("Ok\n");
    }

    printf("Test and:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x8122;
        vm.V[1] = 0x01;
        vm.V[2] = 0xFF;
        chip8_evaluate_opcode_name("AND", &vm);
        assert(vm.PC == 0x202 && vm.V[1] == 0x01);
        printf("Ok\n");
    }

    printf("Test xor:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x8123;
        vm.V[1] = 0x0F;
        vm.V[2] = 0xF0;
        chip8_evaluate_opcode_name("XOR", &vm);
        assert(vm.PC == 0x202 && vm.V[1] == 0xFF);
        printf("Ok\n");
    }

    printf("Test addr:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x8014;
        vm.V[0] = 0xFF;
        vm.V[1] = 0xFF;
        chip8_evaluate_opcode_name("ADDR", &vm);
        assert(vm.PC == 0x202 && vm.V[0] == 0xFE && vm.V[0xF] == 1);
        printf("Ok\n");
    }

    printf("Test sub:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x8015;
        vm.V[0] = 0x01;
        vm.V[1] = 0x02;
        chip8_evaluate_opcode_name("SUB", &vm);
        assert(vm.PC == 0x202 && vm.V[0] == 0xFF && vm.V[0xF] == 0x1);
        printf("Ok\n");
    }

    printf("Test shr:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x8006;
        vm.V[0] = 0xFF;
        chip8_evaluate_opcode_name("SHR", &vm);
        assert(vm.PC == 0x202 && vm.V[0] == 0x7F);
        printf("Ok\n");
    }

    printf("Test subb:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x8127;
        vm.V[1] = 0xFF;
        vm.V[2] = 0xFF;
        chip8_evaluate_opcode_name("SUBB", &vm);
        assert(vm.PC == 0x202 && vm.V[1] == 0x0);
        printf("Ok\n");
    }

    printf("Test shl:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x800E;
        vm.V[0] = 0xFF;
        chip8_evaluate_opcode_name("SHL", &vm);
        assert(vm.PC == 0x202 && vm.V[0] == 0xFE);
        printf("Ok\n");
    }

    printf("Test jneq:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0x9010;
        vm.V[0] = 0xAA;
        vm.V[1] = 0xAB;
        chip8_evaluate_opcode_name("JNEQ", &vm);
        assert(vm.PC == 0x204);
        printf("Ok\n");
    }

    printf("Test seti:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xABBB;
        chip8_evaluate_opcode_name("LOADI", &vm);
        assert(vm.PC == 0x202 && vm.I == 0xBBB);
        printf("Ok\n");
    }

    printf("Test jmpv0:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xBC00;
        vm.V[0] = 0xCC;
        chip8_evaluate_opcode_name("JUMPI", &vm);
        assert(vm.PC == 0xCCC);
        printf("Ok\n");
    }

    printf("Test rrand:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xC0FF;
        chip8_evaluate_opcode_name("RAND", &vm);
        assert(vm.PC == 0x202 && vm.V[0] > 0 && vm.V[0] <= 0xFF);
        printf("Ok\n");
    }

    printf("Test jkey:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xE09E;
        vm.keycode = 'A';
        vm.V[0] = 'A';
        chip8_evaluate_opcode_name("SKPR", &vm);
        assert(vm.PC == 0x204 && vm.keycode == vm.V[0]);
        printf("Ok\n");
    }

    printf("Test jnkey:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xE0A1;
        vm.keycode = 'A';
        vm.V[0] = 'B';
        chip8_evaluate_opcode_name("SKUP", &vm);
        assert(vm.PC == 0x204 && vm.keycode != vm.V[0]);
        printf("Ok\n");
    }

    printf("Test getdelay:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xF007;
        vm.delay_timer = 255;
        chip8_evaluate_opcode_name("MOVED", &vm);
        assert(vm.PC == 0x202 && vm.V[0] == 255);
        printf("Ok\n");
    }

    /*
    printf("Test waitkey:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xF00A;
        waitkey(&vm);
        assert(vm.PC == 0x202);
        printf("Ok\n");
    }
    */

    printf("Test setdelay:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xF015;
        vm.V[0] = 255;
        chip8_evaluate_opcode_name("LOADD", &vm);
        assert(vm.PC == 0x202 && vm.delay_timer == 255);
        printf("Ok\n");
    }

    printf("Test setsound:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xF018;
        vm.V[0] = 255;
        chip8_evaluate_opcode_name("LOADS", &vm);
        assert(vm.PC == 0x202 && vm.sound_timer == 255);
        printf("Ok\n");
    }

    printf("Test addi:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xF01E;
        vm.V[0] = 0x1;
        vm.I = 0x0FFF;
        chip8_evaluate_opcode_name("ADDI", &vm);
        assert(vm.PC == 0x202 && vm.I == 0x0 && vm.V[0xF] == 1);
        printf("Ok\n");
    }

    /*
    printf("Test spritei:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xF029;
        spritei(&vm);
        assert(vm.PC == 0x202);
        printf("Ok\n");
    }
    */

    printf("Test bcd:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xF033;
        vm.V[0] = 255;
        chip8_evaluate_opcode_name("BCD", &vm);
        assert(vm.PC == 0x202 &&
            vm.ram[vm.I] == 2 &&
            vm.ram[vm.I + 1] == 5 &&
            vm.ram[vm.I + 2] == 5);
        printf("Ok\n");
    }

    printf("Test push:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xFF55;
        vm.I = 0;
        for (int i = 0; i < 16; i++) {
            vm.V[i] = i;
        }
        chip8_evaluate_opcode_name("PUSH", &vm);
        assert(vm.PC == 0x202);
        for (int i = 0; i < 16; i++) {
            assert(vm.ram[vm.I + i] == i);
        }
        printf("Ok\n");
    }

    printf("Test pop:\t");
    {
        chip8_initialize_vm(&vm);
        vm.opcode.value = 0xFF65;
        vm.I = 0;
        for (int i = 0; i < 16; i++) {
            vm.ram[vm.I + i] = i;
        }
        chip8_evaluate_opcode_name("POP", &vm);
        assert(vm.PC == 0x202);
        for (int i = 0; i < 16; i++) {
            assert(vm.V[i] == i);
        }
        printf("Ok\n");
    }
}

int main(int argc, char* argv[])
{
    selftest();

    return 0;
}
