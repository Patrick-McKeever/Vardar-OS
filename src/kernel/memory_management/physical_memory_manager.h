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
	// Number of blocks that have been allocated but not freed.
	uint32_t used_blocks;
} MemMap;

static MemMap PHYS_MEMORY_MAP;

/**
 * Mark a page frame as used.
 * @input index The index of the page to mark as used.
 */
static void SetFrameUsed(int index);

/**
 * Mark a page as freed.
 * @input index The index of the page to mark as freed.
 */
static void SetFrameFree(int index);

/**
 * @return Whether or not the page at the identified index is being used.
 */
static bool FrameIsUsed(int index);


void InitPmm(struct stivale2_struct_tag_memmap *memmap)
{
	// The stivale boot structure does not guarantee any particular order for
	// the page frames it gives us. To begin, we should find the address of the
	// highest usable page to determine the necessary bitmap length.
	PHYS_MEMORY_MAP.max_blocks = memmap->entries;
	uint64_t uppermost_usable_addr = 0;

	for(int i = 0; i < memmap->entries; ++i) {
		struct stivale2_mmap_entry page_frame = memmap->memmap[i];
		if(page_frame.type == USABLE_PAGE) {
			size_t upper_frame_addr = page_frame.base + page_frame.length - 1;
			uppermost_usable_addr = (upper_frame_bit > uppermost_usable_addr) ?
										upper_frame_bit : uppermost_usable_addr;
		}
	}

	// Get num bytes in bitmap.
	// To bootstrap our PMM, we need to find a block of memory large enough
	// to hold the bitmap - hence, we round up to the nearest multiple of the
	// page size (4KiB).
	uint32_t bitmap_size = uppermost_usable_addr / FRAME_SIZE / 8;
	PHYS_MEMORY_MAP.bitmap_size = RoundToNearestMultiple(bitmap_size, 
														 FRAME_SIZE);
		
	bool found_page = false;
	for(int i = 0; i < memmap->entries; ++i) {
		struct stivale2_mmap_entry page_frame = memmap->memmap[i];
		bool large_enough = page_frame.size >= PHYS_MEMORY_MAP.bitmap_size;
		if(large_enough && page_frame.type == USABLE_PAGE) {
			// Set all bits in frame (bitmap) to 1 (-1 is 0b11111111).
			PHYS_MEMORY_MAP.bitmap = (uint8_t *) (page_frame[i].base);
			memset(PHYS_MEMORY_MAP.bitmap, -1, PHYS_MEMORY_MAP.bitmap_size);

			// Now that we're using this page for the bitmap, remove the used
			// segment from the memory map.
			memmap->memmap[i].base += PHYS_MEMORY_MAP.bitmap_size;
			memmap->memmap[i].length = PHYS_MEMORY_MAP.bitmap_size;
			found_page = true;
			break;
		}
	}

	if(! found_page) {
		// Later on, make this into a panic.
		for (;;) {
			asm ("hlt");
    	}
	}

	for(int i = 0; i < memmap->entries; ++i) {
		struct stivale2_mmap_entry page_frame = memmap->memmap[i];
		if(page_frame.type == USABLE_PAGE) {
			size_t starting_page = mmap[i].base / FRAME_SIZE;
			size_t ending_page = starting_page + mmap[i].length / FRAME_SIZE;
			for(int i = starting_page; i < ending_page; ++i) {
				SetPageFree(i);
			}
		}
	}
}

static void SetPageUsed(int index)
{
	uint8_t *bmp_entry_byte = &PHYS_MEMORY_MAP.bitmap[index / 8];
	SetNthBit(bmp_entry_byte, index % 8);
}

static void SetPageFree(int index)
{
	uint8_t *bmp_entry_byte = &PHYS_MEMORY_MAP.bitmap[index / 8];
	ClearNthBit(bmp_entry_byte, index % 8);
}

static bool PageIsUsed(int index)
{
	uint8_t bmp_entry_byte = PHYS_MEMORY_MAP.bitmap[index / 8];
	return GetNthBit(bmp_entry_byte, index % 8);
}


#endif
