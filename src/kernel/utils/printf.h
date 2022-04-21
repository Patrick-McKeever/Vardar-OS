#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

extern void (*term_write)(const char *string, size_t length);


void PrintK(char *str,...);

char *Convert(uint64_t num, int base);

#endif
