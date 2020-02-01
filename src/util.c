#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define BUFFER_STRIDE 4096

static long filesize(FILE *fp)
{
    int pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, pos, SEEK_SET);

    return size;
}

static unsigned char* readfile(FILE *fp, size_t size)
{
    char* buffer = (char*) calloc(sizeof(char), size + 1);

    long i = 0;
    while (i < size) {
        fread((buffer + i), 1, BUFFER_STRIDE, fp);
        i += size - BUFFER_STRIDE < BUFFER_STRIDE ? size : BUFFER_STRIDE;
    }
    fclose(fp);

    return buffer;
}

unsigned char* readtext(const char* filename)
{
    FILE* fp = fopen(filename, "rt");
    if (!fp) {
        fprintf(stderr, "Couldn't open file: %s\n", filename);
        exit(1);
    }

    size_t size = filesize(fp);
    unsigned char* ret = readfile(fp, size + 1);
    ret[size] = '\0';

    return ret;
}

size_t readbin(uint8_t *buffer, const char* filename)
{
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("Couldn't open file\n");
        return 0;
    }

    long size = filesize(fp);
    fread(buffer, sizeof(uint8_t), size, fp);
    fclose(fp);

    return size;
}