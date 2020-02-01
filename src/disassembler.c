#include <stdio.h>

#include "util.h"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: chip8-disasm <filename>\n");
        exit(1);
    }

    const char* filename = argv[1];
    uint8_t buffer = readfile(filename, "rb");

        
    return 0;
}
