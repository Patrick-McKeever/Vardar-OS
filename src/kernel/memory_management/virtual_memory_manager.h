#ifndef VIRTUAL_MEMORY_MANAGER_H
#define VIRTUAL_MEMORY_MANAGER_H

#include "physical_memory_manager.h"

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


static uint64_t *KERNEL_PAGE_TABLE_ROOT;

static uint64_t *GetOrCreatePageTable(uint64_t *parent, size_t index, int flags);
static uint64_t *GetPageTable(uint64_t *parent, size_t index);
void InitPageTable(uint64_t *page_table);

bool MapPage(uint64_t *page_table_root, size_t vaddr, size_t paddr, 
		 	 uint16_t flags)
{
	uint64_t *parent_table = page_table_root, child_table;
	for(int i = 3; i >= 1; --i) {
		size_t tab_index = V_ADDR_INDEX(vaddr, i + 1);
		uint64_t *child_table = GetOrCreatePageTable(parent_table, tab_index, 
													 flags);
		if(child_table == NULL)
			return false;
		parent_table = child_table;
	}
	
	uint64_t entry = paddr | flags;
	child_table[V_ADDR_INDEX(vaddr, 1)] = paddr | flags;
	return true;
}

bool MapKernelPage(size_t vaddr, size_t paddr, uint16_t flags)
{
	return MapPage(KERNEL_PAGE_TABLE_ROOT, vaddr, paddr, flags);
}

bool UnmapPage(uint64_t *pagemap, size_t vaddr)
{
	uint64_t *parent_table = page_table_root, child_table;
	for(int i = 3; i >= 1; --i) {
		size_t tab_index = V_ADDR_INDEX(vaddr, i + 1);
		uint64_t *child_table = GetPageTable(parent_table, tab_index);
		if(child_table == NULL)
			return false;
		parent_table = child_table;
	}
	child_table[V_ADDR_INDEX(vaddr, 1)] = 0;
	__asm__ volatile("invlpg (%[pg_addr])" ::[pg_addr] "r" (vaddr));
	return true;
}

bool UnmapKernelPage(size_t vaddr)
{
	return UnmapPage(KERNEL_PAGE_TABLE_ROOT, vaddr);
}

bool RemapKernelPage(size_t former_vaddr, size_t new_vaddr, uint16_t flags)
{
	uint64_t paddr = KernelVAddrToPAddr(former_vaddr);
	UnmapKernelPage(vaddr);
	MapKernelPage(former_vaddr, paddr, flags);
}

uint64_t VAddrToPAddr(uint64_t *table, uint64_t vaddr)
{
	uint64_t *parent_table = page_table_root, child_table;
	for(int i = 3; i >= 1; --i) {
		size_t tab_index = V_ADDR_INDEX(vaddr, i + 1);
		uint64_t *child_table = GetPageTable(parent_table, tab_index);
		parent_table = child_table;
	}
	// Get rid of flags.
	return (child_table[V_ADDR_INDEX(vaddr, 1)] & ~(0x1FF));
}

uint64_t KernelVAddrToPAddr(uint64_t vaddr)
{
	return VAddrToPadder(KERNEL_PAGE_TABLE_ROOT, vaddr);
}

void GetPageFlag(uint64_t page, uint64_t flag)
{
	return (page & flag) != 0;
}

static uint64_t *GetOrCreatePageTable(uint64_t *parent, size_t index, int flags)
{
	if(GetPageFlag(parent[index], PRESENT)) {
		void *free_frame = AllocFirstFrame();
		if(free_frame == NULL) {
			return NULL;
		}
		entry[index] = free_frame | flags;
		return free_frame;
	}

	return &entry[index];
}


static uint64_t *GetPageTable(uint64_t *parent, size_t index)
{
	if(GetPageFlag(parent[index]), PRESENT)
		return entry[index];
	return NULL;
}
#endif
