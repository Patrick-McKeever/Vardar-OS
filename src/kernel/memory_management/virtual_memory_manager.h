#ifndef VIRTUAL_MEMORY_MANAGER_H
#define VIRTUAL_MEMORY_MANAGER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "memory_management/physical_memory_manager.h"
#include "stivale2.h"

// Page flags.
#define PRESENT						0x0000000000000001
#define READ_WRITABLE				0x0000000000000002
#define USER_ACCESSIBLE				0x0000000000000004
#define WRITE_THROUGH				0x0000000000000008
#define CACHE_DISABLED				0x0000000000000010
#define ACCESSED					0x0000000000000020
#define DIRTY						0x0000000000000040
#define PAGE_ATTRIBUTE_TABLE		0x0000000000000080
#define GLOBAL						0x0000000000000f00

// 512 entries per table, of 4KiB pages each.
#define LOG2_ENTRIES_PER_TABLE		9
#define LOG2_FRAME_SIZE				12
#define MAX_PAGE_IND				0x1FF

// If we want to map virtual address n, we need to figure out where in the table
// tree n resides. If each table holds 512 entries and leaves hold 4KiB pages,
// then the entry pointing to n will be (n / (4KiB * 512^(m - 1))) % 512 at each
// level m. This is because each leaf entry holds 4KiB of physical memory, and
// each table contains 512 pointers to lower levels. Thus, a level 2 table will
// contain 512^(2-1)= 512 pointers to 4KiB pages; a level 3 table will contain
// 512 pointers to 512 level 2 tables, each of which contains 512 4KiB pages,
// equivalent to 512^(3-1) = 512*512 4KiB pages in total; and so on.
// This macro expression is equivalent; we simply replace division with a right
// shift (since we'll inevitably be dividing by some power of 2) and modulo with
// bitwise and (since x % y = x % (y-1)).
#define V_ADDR_INDEX(vaddr, level)	\
	(vaddr >> (LOG2_FRAME_SIZE + LOG2_ENTRIES_PER_TABLE * (level - 1))) \
	& MAX_PAGE_IND


void InitPageTable(uint64_t *page_table, 
				   struct stivale2_struct_tag_memmap *memmap);

bool MapPage(uint64_t *page_table_root, size_t vaddr, size_t paddr, 
		 	 uint16_t flags);

bool MapKernelPage(size_t vaddr, size_t paddr, uint16_t flags);

bool UnmapPage(uint64_t *pagemap, size_t vaddr);

bool UnmapKernelPage(size_t vaddr);

bool RemapPage(uint64_t *table, size_t former_vaddr, size_t new_vaddr, 
			   uint16_t flags);

bool RemapKernelPage(size_t former_vaddr, size_t new_vaddr, uint16_t flags);

uint64_t VAddrToPAddr(uint64_t *table, uint64_t vaddr);

uint64_t KernelVAddrToPAddr(uint64_t vaddr);

bool GetPageFlag(uint64_t page, uint64_t flag);

static inline uint64_t *GetOrCreatePageTable(uint64_t *parent, size_t index, 
											 int flags)
{
	if(GetPageFlag(parent[index], PRESENT))
		return &parent[index];
	
	void *free_frame = AllocFirstFrame();	
	if(free_frame == NULL) {
		return NULL;
	}
	parent[index] = ((uint64_t) free_frame) | flags;
	return free_frame;
}

static inline uint64_t *GetPageTable(uint64_t *parent, size_t index)
{
	if(GetPageFlag(parent[index], PRESENT))
		return &parent[index];
	return NULL;
}

#endif
