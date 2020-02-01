SDL2=`pkg-config --cflags --libs sdl2`

all:
	gcc src/chip8.c -o chip8-vm ${SDL2}
	gcc src/assembler.c -o chip8-asm ${SDL2}

clean:
	rm -Rf chip8
