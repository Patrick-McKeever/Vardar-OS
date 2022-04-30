#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void *memset (void *dest, int val, size_t len);

void *memmove(void* dstptr, const void* srcptr, size_t size);

int strncmp(const char *s1, const char *s2, size_t n);

int strcmp(const char *X, const char *Y);

char *strcpy(char *dest, const char *src);

unsigned int strlen(const char *s);


#endif
