#include "memory_management/kheap.h"
#include "utils/string.h"
#include "memory_management/physical_memory_manager.h"

static const size_t HEADER_SIZE = sizeof(uint32_t);
static const size_t FOOTER_SIZE = sizeof(uint32_t);
static const size_t MDATA_SIZE	= sizeof(uint32_t) * 2;
static void *HEAP_START;
static size_t HEAP_SIZE;

static inline uint32_t*
footer_from_header(uint32_t *header);

static inline uint32_t*
header_from_footer(uint32_t *footer);

static inline uint32_t*
next_header(uint32_t *header);

static inline uint32_t*
prev_footer(uint32_t *footer);

static inline size_t
merge_size(void *block1, void *block2);

static inline size_t
threeway_merge_size(void *front, void *middle, void *back);

static void*
split_block(void *block, size_t alloc_size);

static void*
merge_block(void *first_block, void *second_block);

static inline void*
merge_forward(void *first_block, void *second_block, size_t size);

static inline void*
merge_backward(void *first_block, void *second_block, size_t size);

static inline void*
threeway_merge(void *front, void *middle, void *back, size_t size);

void init_heap(size_t heap_size)
{
	HEAP_START = AllocContiguous(heap_size);
	HEAP_SIZE = heap_size;
}

void *kalloc(size_t size)
{
	const size_t HEADER_SIZE = sizeof(uint32_t);
	const size_t FOOTER_SIZE = sizeof(uint32_t);

	void *heap_ptr = HEAP_START;
	uint32_t *heap_header = (uint32_t*) heap_ptr;
	while(((uintptr_t) heap_ptr < (uintptr_t) HEAP_START + HEAP_SIZE) &&
		  ((*heap_header & 1) || (*heap_header <= size)))
	{
		heap_ptr += (*heap_header & SIZE_MASK) + HEADER_SIZE + FOOTER_SIZE;
		heap_header = (uint32_t*) heap_ptr;
	}

	// If we've found such a block, then allocate it.
	if((uintptr_t) heap_ptr < (uintptr_t) HEAP_START + HEAP_SIZE) {
		*heap_header &= 1;
		// Round size up to even number for alignment reasons.
		size_t new_size = ((size + 1) >> 1) << 1;
		size_t old_size = *heap_header & SIZE_MASK;

		// Set block as used, document size in header and footer.
		uint32_t *heap_footer = (uint32_t *) heap_ptr + HEADER_SIZE + new_size;
		*heap_header = new_size | 1;
		*heap_footer = *heap_header;
		
		// If we're splitting the block in two, then create a heap
		// block header at the start of the unallocated half specifying
		// size (low bit will always be 0, since size is aligned to 8).
		if(old_size < new_size + MDATA_SIZE) {
			heap_ptr = split_block(heap_ptr, new_size);
		}
		
		memset((heap_ptr + HEADER_SIZE), 0, size);
		// Make sure caller doesn't see header byte.
		return (heap_ptr + HEADER_SIZE);
	}

	return NULL;
}

void *krealloc(void *allocation, size_t size)
{
	// Clear allocation bits of this block's header/footer.
	uint32_t *header = (uint32_t*) (allocation - HEADER_SIZE);
	uint32_t *footer = (uint32_t*) (allocation + HEADER_SIZE + (*header & SIZE_MASK));
	uint32_t *next_block_header = next_header(header);
	uint32_t *prev_block_footer = prev_footer(footer);
	void *curr_block = (void*) header;
	void *next_block = (void*) next_block_header;
	void *prev_block = (void*) header_from_footer(prev_block_footer);
	
	// There is a possibility for an Ω(1) reallocation if the next
	// or previous block is large enough to contain the requested
	// allocation.
	size_t block_size		=	*header & - 2;

	// Note that we add mdata (i.e. h+f) size, because, in the case 
	// of a merge, this block's footer and the next block's header 
	// will just become part of the chunk body.
	if(next_block && !(*next_block_header & 1) && 
		merge_size(curr_block, next_block) >= size)
	{
		return merge_forward(curr_block, next_block, size);
	}

	if(prev_block && !(*prev_block_footer & 1) &&
		merge_size(prev_block, curr_block) >= size)
	{
		return merge_backward(prev_block, curr_block, size);
	}

	if(prev_block && next_block &&
		!(*prev_block_footer & 1) && !(*next_block_header & 1) &&
		threeway_merge_size(prev_block, curr_block, next_block) >= size)
	{
		return threeway_merge(prev_block, curr_block, next_block, size);
	}

	// Otherwise, we just need to find a new block and copy the memory.
	void *new_allocation = kalloc(size);
	if(!new_allocation) {
		return NULL;
	}

	memmove((new_allocation + HEADER_SIZE), (allocation + HEADER_SIZE),
			block_size);
	kfree(allocation);
	return new_allocation;
}

void kfree(void *allocation)
{
	const size_t HEADER_SIZE = sizeof(uint32_t);
	const size_t FOOTER_SIZE = sizeof(uint32_t);

	// Clear allocation bits of this block's header/footer.
	uint32_t *header = (uint32_t*) (allocation - HEADER_SIZE);
	*header &= SIZE_MASK;
	uint32_t *footer = (uint32_t*) (allocation + HEADER_SIZE + *header);
	*footer &= SIZE_MASK;

	// Remember case where allocation is first block, and there's no prev footer,
	// and where allocation is last block, and there's no next header.
	uint32_t *next_block_header = (uint32_t*) ((uintptr_t) footer + FOOTER_SIZE);
	uint32_t *next_block_footer = (uint32_t*) ((uintptr_t) next_block_header + 
												HEADER_SIZE + 
												(*next_block_header & SIZE_MASK));
	uint32_t *prev_block_footer = (uint32_t*) (allocation - FOOTER_SIZE);
	uint32_t *prev_block_header = (uint32_t*) ((uintptr_t) prev_block_footer -
												(*prev_block_footer & SIZE_MASK) - 
												HEADER_SIZE);

	// If next block is free, merge with next block.
	if(!(*next_block_header & 1)) {
		footer = next_block_footer;
		*header += *next_block_header;
		*footer = *header;
	}

	// If previous block is free, merge with previous block.
	if(!(*prev_block_footer & 1)) {
		header = prev_block_header;
		*header += *prev_block_footer;
		*footer = *header;
	}
}

static inline uint32_t*
footer_from_header(uint32_t *header)
{
	size_t block_size = *header & SIZE_MASK;
	uintptr_t footer_addr = (uintptr_t) header + HEADER_SIZE + block_size;
	return (uint32_t*) footer_addr;
}

static inline uint32_t*
header_from_footer(uint32_t *footer)
{	
	size_t block_size = *footer & SIZE_MASK;
	uintptr_t header_addr = (uintptr_t) footer - block_size - HEADER_SIZE;
	return (uint32_t*) header_addr;
}

static inline uint32_t*
next_header(uint32_t *header)
{
	size_t block_size = *header & SIZE_MASK;
	uint32_t *next_header  = (uint32_t*) ((uintptr_t) header + HEADER_SIZE + 
								block_size + FOOTER_SIZE);

	if((uintptr_t) next_header == (uintptr_t) HEAP_START + HEAP_SIZE) {
		return NULL;
	}
	return next_header;
}

static inline uint32_t*
prev_footer(uint32_t *footer)
{	
	size_t block_size = *footer & SIZE_MASK;
	uint32_t *next_footer = (uint32_t*) ((uintptr_t) footer - block_size -
								HEADER_SIZE - FOOTER_SIZE);
	uintptr_t start_of_block = (uintptr_t) header_from_footer(next_footer);

	if(start_of_block == (uintptr_t) HEAP_START) {
		return NULL;
	}
	return next_footer;
}

static inline size_t
merge_size(void *block1, void *block2)
{
	uint32_t *block1_header = (uint32_t*) block1;
	uint32_t *block2_header = (uint32_t*) block2;
	size_t block1_size = *block1_header & SIZE_MASK;
	size_t block2_size = *block2_header & SIZE_MASK;
	
	// Two adjacent blocks have a header and footer with a size
	// of MDATA_SIZE that will be eliminated in a merge.
	return block1_size + block2_size + MDATA_SIZE;
}

static inline size_t
threeway_merge_size(void *front, void *middle, void *back)
{
	uint32_t *front_header	= 	(uint32_t*) front;
	uint32_t *middle_header	= 	(uint32_t*) middle;
	uint32_t *back_header	= 	(uint32_t*) back;
	size_t front_size 		= 	*front_header & SIZE_MASK;
	size_t middle_size 		= 	*middle_header & SIZE_MASK;
	size_t back_size		= 	*back_header & SIZE_MASK;
	
	// Likewise, we will here eliminate 2 metadata segments,
	// reclaiming them for a new chunk's body.
	return front_size + middle_size + back_size + MDATA_SIZE * 2;
}

static void*
split_block(void *block, size_t alloc_size)
{
	// From: || [Outer-Header] || [Body]  || [Outer-Footer]
	// To:   || [Outer-Header] || [Body1] || [Inner-Footer] || 
	// 			[Inner-Header] || [Body2] || [Outer-Footer]
	uint32_t *outer_header = (uint32_t*) block;
	uint32_t *outer_footer = footer_from_header(outer_header);
	size_t block_size = *outer_header & SIZE_MASK;

	if(!(block_size >= alloc_size + MDATA_SIZE)) {
		return NULL;
	}

	uint32_t *inner_footer = footer_from_header(outer_header);
	uint32_t *inner_header = next_header(outer_header);

	*outer_header = alloc_size | 1;
	*inner_footer = alloc_size | 1;
	*inner_header = block_size - alloc_size - MDATA_SIZE;
	*outer_footer = *inner_header;

	kfree((void*) inner_header + HEADER_SIZE);
	return (void*) outer_header;
}

static void*
merge_block(void *first_block, void *second_block)
{
	uint32_t *first_block_header  = (uint32_t*) first_block;
	uint32_t *second_block_header = (uint32_t*) second_block;
	uint32_t *second_block_footer = footer_from_header(second_block_header);
	
	if(next_header(first_block_header) != second_block_header) {
		return NULL;
	}

	// We are eliminating a chunk of metadata (i.e. h/f) that
	// used to separate these two chunks.
	*first_block_header  = merge_size(first_block, second_block) |
						   (*first_block_header & 1);
	*second_block_footer = *first_block_header;
	return (void*) first_block_header;	
}

static inline void*
merge_forward(void *first_block, void *second_block, size_t size)
{
	uint32_t *second_block_header = (uint32_t*) second_block;
	size_t second_block_size = *second_block_header & SIZE_MASK;
	void *new_block = merge_block(first_block, second_block);
	// All of second block, except its footer, can be discarded.
	memset(second_block, 0, second_block_size + HEADER_SIZE);
	
	// There will be space left over at the end of the new block.
	return split_block(new_block, size);
}

static inline void*
merge_backward(void *first_block, void *second_block, size_t size)
{
	uint32_t *second_block_header = (uint32_t*) second_block;
	size_t second_block_size = *second_block_header & SIZE_MASK;
	memmove(first_block + HEADER_SIZE, second_block + HEADER_SIZE,
			second_block_size);
	void *new_block = merge_block(first_block, second_block);
	memset(second_block, 0, second_block_size + HEADER_SIZE);

	return split_block(new_block, size);
}

static inline void*
threeway_merge(void *front, void *middle, void *back, size_t size)
{
	uint32_t *front_header	= 	(uint32_t*) front;
	uint32_t *middle_header	= 	(uint32_t*) middle;
	uint32_t *back_header	= 	(uint32_t*) back;
	uint32_t *back_footer	=	footer_from_header(back_header);
	size_t middle_size 		= 	*middle_header & SIZE_MASK;

	memmove(middle, front, middle_size);
	size_t size_to_zero 	= 	(size_t) (uintptr_t) back_footer -
									((uintptr_t) front + middle_size);
	memset(front + middle_size, 0, size_to_zero);
	*front_header 			= 	threeway_merge_size(front, middle, back) | 1;
	*back_footer			=	*front_header;

	return split_block(front, size);
}
