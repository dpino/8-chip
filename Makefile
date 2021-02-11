SDL2=`pkg-config --cflags --libs sdl2`

all:
	gcc src/util.c src/chip8-vm.c src/chip8-main.c -o chip8-main ${SDL2}
	gcc src/util.c src/chip8-vm.c src/assembler.c -o chip8-asm ${SDL2}
	gcc src/util.c src/chip8-vm.c src/disassembler.c -o chip8-disasm ${SDL2}
	gcc src/util.c src/chip8-vm.c src/chip8-test.c -o chip8-test ${SDL2}

clean:
	rm -Rf chip8-vm chip8-asm chip8-disasm chip8-test chip8-main
