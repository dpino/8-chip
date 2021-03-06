#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

uint16_t bswap(uint16_t value)
{
    uint8_t hi = value >> 8;
    uint8_t lo = value & 0xff;

    return lo << 8 | hi;
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

char* strtrim(char* str)
{
    size_t start, end;
    size_t i, len = strlen(str);

    // If there are not spaces at the beginning and end, return whole string.
    if (str[0] != ' ' && str[len - 1] != ' ')
        return str;

    // Skip leading spaces.
    for (i = 0; str[i] == ' ' && i < len - 1; i++);
    start = i;

    // Skip trailing spaces.
    for (i = len - 1; i >= 0 && str[i] == ' '; i--);
    end = i;

    // Copy substring.
    memmove(str, str + start, end - start + 1);
    str[(end - start) + 1] = '\0';
    return str;
}

char* first_word(char* str)
{
    // Remove leading and trailing spaces.
    strtrim(str);

    // Locate first space.
    char* end = strchr(str, ' ');
    if (!end)
        end = str + strlen(str) - 1;
    size_t len = end - str + 1;

    // Copy substring.
    char* ret = (char*) calloc(len, sizeof(char));
    strncpy(ret, str, len);
    ret[len - 1] = '\0';
    return ret;
}
