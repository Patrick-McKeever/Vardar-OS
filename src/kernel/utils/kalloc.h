#ifndef KALLOC_H
#define KALLOC_H

typedef struct __heap_block_header {
	size_t size;
	bool free;
	__heap_block_header *next;
} heap_block_header_t;

static void *HEAP_HEAD, *HEAP_TAIL;

bool
init_heap(uint64_t heap_size)
{
	HEAP_HEAD = AllocPageFrame();
	for(size_t i = 0; i < heap_size; i += 0x1000) {
		heap_tail = AllocPageFrame();
		if(! heap_tail)
			return false;
	}

	return true;
}

void *
kalloc(size_t num_bytes)
{
	void *p = HEAP_HEAD;
	
}

void
free(void *allocation);

static void
merge(heap_block_header_t *block1, heap_block_header_t *block2)
{

}

#endif
