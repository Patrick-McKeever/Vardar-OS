#include "virtual_memory_manager.h"

static uint64_t *KERNEL_PAGE_TABLE_ROOT;

void InitPageTable(uint64_t *page_table, 
				   struct stivale2_struct_tag_memmap *memmap)
{
	// Probably want to do sth with PAT here.
	//for(uint)
}

bool MapPage(uint64_t *page_table_root, size_t vaddr, size_t paddr, 
		 	 uint16_t flags)
{
	uint64_t *parent_table = page_table_root, *child_table;
	for(int i = 3; i >= 1; --i) {
		size_t tab_index = V_ADDR_INDEX(vaddr, i + 1);
		uint64_t *child_table = GetOrCreatePageTable(parent_table, tab_index, 
													 flags);
		if(child_table == NULL)
			return false;
		parent_table = child_table;
	}
	
	child_table[V_ADDR_INDEX(vaddr, 1)] = paddr | flags;
	return true;
}

bool MapKernelPage(size_t vaddr, size_t paddr, uint16_t flags)
{
	return MapPage(KERNEL_PAGE_TABLE_ROOT, vaddr, paddr, flags);
}

bool UnmapPage(uint64_t *page_table_root, size_t vaddr)
{
	uint64_t *parent_table = page_table_root, *child_table;
	for(int i = 3; i >= 1; --i) {
		size_t tab_index = V_ADDR_INDEX(vaddr, i + 1);
		child_table = GetPageTable(parent_table, tab_index);
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

bool RemapPage(uint64_t *page_table_root, size_t former_vaddr, size_t new_vaddr, 
			   uint16_t flags)
{
	uint64_t paddr 	= VAddrToPAddr(page_table_root, former_vaddr);
	int unmap_code 	= UnmapPage(page_table_root, former_vaddr);
	int map_code 	= MapPage(page_table_root, former_vaddr, paddr, flags);
	return unmap_code && map_code; 	
}

bool RemapKernelPage(size_t former_vaddr, size_t new_vaddr, uint16_t flags)
{
	uint64_t paddr 	= KernelVAddrToPAddr(former_vaddr);
	int unmap_code 	= UnmapKernelPage(former_vaddr);
	int map_code 	= MapKernelPage(former_vaddr, paddr, flags);	
	return unmap_code && map_code; 
}

uint64_t VAddrToPAddr(uint64_t *table, uint64_t vaddr)
{
	uint64_t *parent_table = table, *child_table;
	for(int i = 3; i >= 1; --i) {
		size_t tab_index = V_ADDR_INDEX(vaddr, i + 1);
		child_table = GetPageTable(parent_table, tab_index);
		parent_table = child_table;
	}
	// Get rid of flags.
	return (child_table[V_ADDR_INDEX(vaddr, 1)] & ~(0x1FF));
}

uint64_t KernelVAddrToPAddr(uint64_t vaddr)
{
	return VAddrToPAddr(KERNEL_PAGE_TABLE_ROOT, vaddr);
}

bool GetPageFlag(uint64_t page, uint64_t flag)
{
	return (page & flag) != 0;
}
