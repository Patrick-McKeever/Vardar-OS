#include "string.h"
#include <stdint.h>

void *memset (void *dest, int val, size_t len)
{
  uint8_t *ptr = (uint8_t *) dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}

void *memmove(void* dstptr, const void* srcptr, size_t size)
{
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	if (dst < src) {
		for (size_t i = 0; i < size; i++)
			dst[i] = src[i];
	} else {
		for (size_t i = size; i != 0; i--)
			dst[i-1] = src[i-1];
	}
	return dstptr;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    while(n && *s1 && ( *s1 == *s2 )) {
		++s1;
		++s2;
		--n;
    }

    if ( n == 0 ) {
        return 0;
    }

    else {
        return (*(unsigned char *)s1 - *(unsigned char *)s2);
    }
}
