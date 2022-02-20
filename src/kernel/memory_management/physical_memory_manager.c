#include "physical_memory_manager.h"
#include "utils/string.h"
#include "utils/printf.h"
#include <stddef.h>

static MemMap PHYS_MEMORY_MAP;

static uint64_t UppermostUsableAddr(struct stivale2_struct_tag_memmap *memmap);
static int FindMemEntryBySize(struct stivale2_struct_tag_memmap *memmap,
							  uint32_t minimum_num_bytes);
static void SetPageUsed(int index);
static void SetPageFree(int index);
static bool PageIsUsed(int index);


bool InitPmm(struct stivale2_struct_tag_memmap *memmap)
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
	PHYS_MEMORY_MAP.num_entries = uppermost_usable_addr / FRAME_SIZE;
	PHYS_MEMORY_MAP.bitmap_size = RoundToNearestMultiple(bitmap_size, 
														 FRAME_SIZE);
	PHYS_MEMORY_MAP.uppermost_addr = uppermost_usable_addr;	

	int bmp_ind = FindMemEntryBySize(memmap, PHYS_MEMORY_MAP.bitmap_size);
	if(bmp_ind == -1) {
		return false;
	}

	// Set all pages as used in bitmap.
	PHYS_MEMORY_MAP.bitmap = (uint8_t*) memmap->memmap[bmp_ind].base;
	memset(PHYS_MEMORY_MAP.bitmap, -1, PHYS_MEMORY_MAP.bitmap_size);

	// Now set the usable ones as free (aside from the page containing the bmp).
	for(int i = 0; i < memmap->entries; ++i) {
		struct stivale2_mmap_entry page_frame = memmap->memmap[i];
		if(page_frame.type == USABLE_PAGE && i != bmp_ind) {
			size_t starting_page = memmap->memmap[i].base / FRAME_SIZE;
			size_t ending_page = starting_page + memmap->memmap[i].length / 
								 FRAME_SIZE;
			for(int i = starting_page; i < ending_page; ++i) {
				SetPageFree(i);
			}
		}
	}
	
	return true;
}

void *AllocFirstFrame() 
{
	for(int i = 0; i < PHYS_MEMORY_MAP.num_entries; ++i) {
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

void FreeFrame(void *frame)
{
	memset(frame, 0, FRAME_SIZE);
	SetPageFree(ADDR_TO_FRAME_IND((uint64_t) frame));
}

int NumFreeFrames()
{
	int free_frames = 0;
	for(int i = 0; i < PHYS_MEMORY_MAP.num_entries; ++i) {
		free_frames += ! PageIsUsed(i);
	}
	return free_frames;
}

/**
 * Find the uppermost address in a memory map which is marked as usable.
 * @input memmap A pointer to a memory map containing a list of pages.
 * @output The highest address in the highest page which is marked as usable.
 */
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
 * Find an entry in the memory map large enough to hold a specific amount of 
 * data.
 * @input memmap The memory map to search for a sufficiently large entry.
 * @input minimum_num_bytes The minimum number of bytes that the entry must
 *							be able to hold.
 * @output The index of an entry which satisfies these conditions, -1 if no
 *		   such entry exists. 
 */
static int FindMemEntryBySize(struct stivale2_struct_tag_memmap *memmap,
						   	  uint32_t minimum_num_bytes)
{
	for(int i = 0; i < memmap->entries; ++i) {
		struct stivale2_mmap_entry page_frame = memmap->memmap[i];
		bool large_enough = page_frame.length >= minimum_num_bytes;
		if(large_enough && page_frame.type == USABLE_PAGE) {
			return i;
		}
	}
	return -1;
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

