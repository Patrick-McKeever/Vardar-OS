#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

extern void (*term_write)(const char *string, size_t length);

static inline void PutChar(char c)
{
	term_write(&c, 1);
}

static inline void Puts(char *c)
{
	int len;
	for(len = 0; c[len] != '\0'; ++len) {}
	term_write(c, len);
}

void PrintK(char *str,...);

char *Convert(uint64_t num, int base);

#endif
