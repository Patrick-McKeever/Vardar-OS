#ifndef KHEAP_H
#define KHEAP_H

#define SIZE_MASK	0xFFFE

#include <stddef.h>
#include <stdint.h>

void init_heap(size_t heap_size);
void *krealloc(void *allocation, size_t size);
void *kalloc(size_t size);
void kfree(void *allocation);
void map_heap(uint64_t *page_table_root);

#endif
