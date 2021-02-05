SDL2=`pkg-config --cflags --libs sdl2`

all:
	gcc src/chip8.c src/util.c -o chip8-vm ${SDL2}
	gcc src/assembler.c src/util.c -o chip8-asm ${SDL2}
	gcc src/disassembler.c src/util.c -o chip8-disasm ${SDL2}

clean:
	rm -Rf chip8-vm chip8-asm chip8-disasm
