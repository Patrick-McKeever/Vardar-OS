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
