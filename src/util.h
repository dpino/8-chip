#pragma once

#include <stdint.h>

uint16_t bswap(uint16_t value);
char* first_word(char* str);
size_t readbin(uint8_t *buffer, const char* filename);
unsigned char* readtext(const char* filename);
char* strtrim(char* str);
