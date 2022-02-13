#include "physical_memory_manager.h"
#include "utils/string.h"
#include <stdbool.h>
#include <stddef.h>

static MemMap PHYS_MEMORY_MAP;

static uint64_t UppermostUsableAddr(struct stivale2_struct_tag_memmap *memmap);
static void *FindPageByMinSize(struct stivale2_struct_tag_memmap *memmap,
							   uint32_t minimum_num_bytes);
static void SetPageUsed(int index);
static void SetPageFree(int index);
static bool PageIsUsed(int index);


void InitPmm(struct stivale2_struct_tag_memmap *memmap)
{
	// The stivale boot structure does not guarantee any particular order for
	// the page frames it gives us. To begin, we should find the address of the
	// highest usable page to determine the necessary bitmap length.
	uint64_t uppermost_usable_addr = UppermostUsableAddr(memmap);
	
	// Get num bytes in bitmap.
	// To bootstrap our PMM, we need to find a block of memory large enough
	// to hold the bitmap - hence, we round up to the nearest multiple of the
	// page size (4KiB).
	uint32_t bitmap_size = uppermost_usable_addr / FRAME_SIZE / 8;
	PHYS_MEMORY_MAP.bitmap_size = RoundToNearestMultiple(bitmap_size, 
														 FRAME_SIZE);
	PHYS_MEMORY_MAP.uppermost_addr = uppermost_usable_addr;	
	bool found_page = false;
	for(int i = 0; i < memmap->entries; ++i) {
		struct stivale2_mmap_entry page_frame = memmap->memmap[i];
		bool large_enough = page_frame.length >= PHYS_MEMORY_MAP.bitmap_size;
		if(large_enough && page_frame.type == USABLE_PAGE) {
			// Set all bits in frame (bitmap) to 1 (-1 is 0b11111111).
			PHYS_MEMORY_MAP.bitmap = (uint8_t *) (page_frame.base);
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
			size_t starting_page = memmap->memmap[i].base / FRAME_SIZE;
			size_t ending_page = starting_page + memmap->memmap[i].length / 
								 FRAME_SIZE;
			for(int i = starting_page; i < ending_page; ++i) {
				SetPageFree(i);
			}
		}
	}
}

void *AllocFirstFrame() 
{
	for(int i = 0; i < PHYS_MEMORY_MAP.bitmap_size; ++i) {
		if(! PageIsUsed(i)) {
			SetPageUsed(i);
			void *frame = (void*)((size_t) i * FRAME_SIZE);
			memset(frame, 0, FRAME_SIZE);

			// Stivale2 spec mandates that 4GiB of memory (entries identified)
			// as "usable" in memmap) be identity-mapped. As such, this function
			// will return an identity-mapped address, since PMM only tracks
			// usable entries.
			return frame;
		}
	}
	return NULL;
}


static uint64_t UppermostUsableAddr(struct stivale2_struct_tag_memmap *memmap)
{
	uint64_t uppermost_usable_addr = 0;
	// Entries are guaranteed to be sorted from highest to lowest, so this will
	// top top addr.
	for(int i = memmap->entries - 1; i >= 0; --i) {
		struct stivale2_mmap_entry page_frame = memmap->memmap[i];
		if(page_frame.type == USABLE_PAGE) {
			uppermost_usable_addr = page_frame.base + page_frame.length - 1;
			break;
		}
	}
	return uppermost_usable_addr;
}

/**
 * Mark a page frame as used.
 * @input index The index of the page to mark as used.
 */
static void SetPageUsed(int index)
{
	uint8_t *bmp_entry_byte = &PHYS_MEMORY_MAP.bitmap[index / 8];
	SetNthBit(bmp_entry_byte, index % 8);
}

/**
 * Mark a page as freed.
 * @input index The index of the page to mark as freed.
 */
static void SetPageFree(int index)
{
	uint8_t *bmp_entry_byte = &PHYS_MEMORY_MAP.bitmap[index / 8];
	ClearNthBit(bmp_entry_byte, index % 8);
}

/**
 * @return Whether or not the page at the identified index is being used.
 */
static bool PageIsUsed(int index)
{
	uint8_t bmp_entry_byte = PHYS_MEMORY_MAP.bitmap[index / 8];
	return GetNthBit(bmp_entry_byte, index % 8);
}

