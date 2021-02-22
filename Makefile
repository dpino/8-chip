SDL2=`pkg-config --cflags --libs sdl2`

all: chip8-main chip8-asm chip8-disasm chip8-test chip8-repl

chip8-main: src/chip8-main.c
	gcc -std=c99 src/util.c src/chip8-vm.c src/chip8-main.c -o chip8-main ${SDL2}

chip8-asm: src/assembler.c
	gcc -std=c99 src/util.c src/parser.c src/chip8-vm.c src/assembler.c -o chip8-asm ${SDL2}

chip8-disasm: src/disassembler.c
	gcc -std=c99 src/util.c src/chip8-vm.c src/disassembler.c -o chip8-disasm ${SDL2}

chip8-test: src/chip8-test.c
	gcc -std=c99 src/util.c src/parser.c src/chip8-vm.c src/chip8-test.c -o chip8-test ${SDL2}

chip8-repl: src/chip8-repl.c src/parser.c
	gcc -std=c99 src/util.c src/parser.c src/chip8-vm.c src/chip8-repl.c -o chip8-repl ${SDL2}

clean:
	rm -Rf chip8-vm chip8-asm chip8-disasm chip8-test chip8-main chip8-repl
