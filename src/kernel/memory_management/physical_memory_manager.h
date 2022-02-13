#ifndef PHYSICAL_MEMORY_MANAGER_H
#define PHYSICAL_MEMORY_MANAGER_H

#include "stivale2.h"
#include "utils/misc.h"

#define FRAME_SIZE 		4096
#define USABLE_PAGE		1

typedef struct {
	uint64_t num_entries;
	// Number of bytes in bitmap.
	uint32_t bitmap_size;
	// Pointer to bytes forming a bitmap, giving the status of pages in memory.
	uint8_t *bitmap;
	// Size of memory in KB.
	uint32_t mem_size;
	// Size of memory over size of block (4KiB).
	uint32_t max_blocks;
	uint64_t uppermost_addr;
	// Number of blocks that have been allocated but not freed.
	uint32_t used_blocks;
} MemMap;

/**
 * Initialize the static MemMap instance given a stivale2 memmap.
 * @input memmap A pointer to a stivale2 memmap containing a list of PFs, their
 *				 types, and the number of entries.
 */
void InitPmm(struct stivale2_struct_tag_memmap *memmap);

/**
 * @output A pointer to the first unused page frame, NULL if none are available.
 */
void *AllocFirstFrame();



#endif
