#ifndef KHEAP_H
#define KHEAP_H

#define SIZE_MASK	0xFFFE

#include <stddef.h>

void init_heap(size_t heap_size);
void *krealloc(void *allocation, size_t size);
void *kalloc(size_t size);
void kfree(void *allocation);

#endif
