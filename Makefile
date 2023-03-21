SDL2=`pkg-config --cflags --libs sdl2`
CC=gcc
CFLAGS=-std=c99
LIBS=src/util.c src/parser.c src/chip8-vm.c

all: chip8-main chip8-asm chip8-disasm chip8-test chip8-repl display

chip8-main: src/chip8-main.c
	${CC} ${CFLAGS} ${LIBS} src/chip8-main.c -o chip8-main ${SDL2}

chip8-asm: src/assembler.c
	${CC} ${CFLAGS} ${LIBS} src/assembler.c -o chip8-asm ${SDL2}

chip8-disasm: src/disassembler.c
	${CC} ${CFLAGS} ${LIBS} src/disassembler.c -o chip8-disasm ${SDL2}

chip8-test: src/chip8-test.c
	${CC} ${CFLAGS} ${LIBS} src/chip8-test.c -o chip8-test ${SDL2}

chip8-repl: src/chip8-repl.c src/parser.c
	${CC} ${CFLAGS} ${LIBS} src/chip8-repl.c -o chip8-repl ${SDL2}

display:
	${CC} ${CFLAGS} ${LIBS} src/backend.c src/display.c -o display ${SDL2}

clean:
	rm -Rf chip8-vm chip8-asm chip8-disasm chip8-test chip8-main chip8-repl display
