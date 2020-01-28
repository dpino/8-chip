SDL2=`pkg-config --cflags --libs sdl2`

all:
	gcc chip8.c -o chip8 ${SDL2}

clean:
	rm -Rf chip8
