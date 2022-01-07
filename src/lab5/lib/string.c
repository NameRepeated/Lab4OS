#include "string.h"

void *memset(void *dst, int c, unsigned long n) {
    char *cdst = (char *)dst;
    for (unsigned long i = 0; i < n; ++i)
        cdst[i] = c;

    return dst;
}

void *memcpy(void *dst, void *src, unsigned long n) {
    char *cdst = (char *)dst;
    char *csrc = (char *)src;
    for (unsigned long i = 0; i < n; ++i) 
        cdst[i] = csrc[i];

    return dst;
}