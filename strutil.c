#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

char* ltrim(const char* str);
char* rtrim(const char* str);
char* trim(const char *str);

char** split(const char* str, char delim, size_t* nmemb);

static char isSpace(const char c)
{
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

static char* copyString(const char* src, size_t start, size_t end)
{
    size_t len = end - start + 1;
    char* dest = (char*) calloc(len, sizeof(char));
    strncpy(dest, src + start, end - start + 1);
    dest[len - 1] = '\0';
    return dest;
}

char* ltrim(const char* str)
{
    size_t i = 0, len = strlen(str);
    for (i = 0; i < len; i++) {
        if (!isSpace(str[i]))
            break;
    }
    return copyString(str, i, len - i + 1);
}

char* rtrim(const char* str)
{
    size_t i = 0;
    for (i = strlen(str) - 1; i >= 0; i--) {
        if (!isSpace(str[i]))
            break;
    }
    return copyString(str, 0, i + 1);
}

char* trim(const char *str)
{
    char* tmp = ltrim(str);
    char* ret = rtrim(tmp);
    free(tmp);
    return ret;
}

static size_t countWords(const char* str, char delim)
{
	size_t ret = 0;
	size_t len = strlen(str);
	for (size_t i = 0; i < len;) {
	    if (str[i] == delim) {
            while (str[i++] == delim && i < len);
            ret++;
        } else {
            i++;
        }
	}
	return ret + 1;
}

char** split(const char* str, char delim, size_t* nmemb)
{
    *nmemb = countWords(str, delim);
    char** ret = (char**) calloc(*nmemb, sizeof(char*));

	size_t len = strlen(str);
	size_t i = 0, start = 0, j = 0;
	while (i < len) {
	    if (str[i] == delim) {
	        ret[j++] = copyString(str, start, i);
            while (str[i++] == delim && i < len);
            start = i - 1;
        } else {
            i++;
        }
	}
	ret[j++] = copyString(str, start, i);

    return ret;
}


static void test_trim(const char* str, const char* expected)
{
    printf("test_trim: '%s' == '%s'\n", str, expected);
	char* actual = trim(str); 
    assert(strcmp(actual, expected) == 0);
    free(actual);
}

static void freeArray(void* arr[], size_t nelem)
{
    for (size_t i = 0; i < nelem; i++) {
        if (arr[i])
            free(arr[i]);
    }
    free(arr);
}

static void test_split(const char* str, char* expected[], size_t expected_nelem)
{
    printf("test_split: '%s' == { ", str);
    for (int i = 0; i < expected_nelem - 1; i++)
        printf("'%s', ", expected[i]);
    printf("'%s'", expected[expected_nelem - 1]);
    printf(" }\n");

    size_t nelem;
    char** actual = split(trim(str), ' ', &nelem);
    assert(nelem == expected_nelem);
    for (size_t i = 0; i < nelem; i++) {
        assert(strcmp(actual[i], expected[i]) == 0);
    }
    freeArray((void**) actual, nelem);
}

void selftest()
{
    fprintf(stdout, "selftest:\n");

    test_trim("  foo  bar  quaz  ", "foo  bar  quaz");

    char* parts[] = { "foo", "bar", "quaz" };
    test_split("  foo  bar  quaz  ", parts, 3);

    fprintf(stdout, "Ok\n");
}

int main(int argc, char* argv[])
{
    selftest();

    return 0;
}
