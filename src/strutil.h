#pragma once

char* ltrim(const char* str);
char* rtrim(const char* str);
char* trim(const char *str);

char** split(const char* str, char delim, size_t* nmemb);
