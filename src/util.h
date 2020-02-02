#pragma once

#include <stdint.h>

unsigned char* readtext(const char* filename);
size_t readbin(uint8_t *buffer, const char* filename);
uint16_t bswap(uint16_t value);