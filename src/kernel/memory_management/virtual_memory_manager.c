#include "virtual_memory_manager.h"
#include "utils/string.h"
#include "utils/printf.h"

static uint64_t *KERNEL_PAGE_TABLE_ROOT;

static void PrintKernelRoot() {
	for(int i = 0; i < 128; ++i) {
		PrintK("Entry %d: 0x%h\t\0", i, KERNEL_PAGE_TABLE_ROOT[i]);
		if(i % 8 == 0)
			PrintK("\n\0");
	}
}

bool InitPageTable(struct stivale2_struct_tag_memmap *memmap)
{
	memset(KERNEL_PAGE_TABLE_ROOT, 0, 512 * 8);
	bool succ = true;
	PrintKernelRoot();
	// Probably want to do sth with PAT here.
	for(uint32_t i = 0; i < memmap->entries; ++i) {
		uint64_t base = memmap->memmap[i].base;
		uint64_t bound = base + memmap->memmap[i].length;
		uint64_t type = memmap->memmap[i].type;
		
		switch(type) {
		case STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE:
			succ &= MapMultipleKernel(base, bound, 0, PRESENT);
			break;
		case STIVALE2_MMAP_FRAMEBUFFER:
			succ &= MapMultipleKernel(base, bound, 0, PRESENT);
		}
	}

	uint16_t kernel_mem = PRESENT | READ_WRITABLE;
	uint64_t two_gb = 2*1024*1024;
	uint64_t four_gb = 2*two_gb;
	succ &= MapMultipleKernel(0, four_gb, 0, kernel_mem);
	succ &= MapMultipleKernel(0, two_gb, KERNEL_DATA, kernel_mem);
	PrintKernelRoot();
	succ &= MapMultipleKernel(0, two_gb, KERNEL_CODE, PRESENT);
	PrintKernelRoot();
	PrintK("Writing back page table\n\0");
	__asm__ volatile("mov %0, %%cr3" :: 
					 "r" ((uint64_t) KERNEL_PAGE_TABLE_ROOT));
	PrintK("Wrote back page table\n\0");
	return succ;
}

bool MapPage(uint64_t *page_table_root, uint64_t vaddr, uint64_t paddr, 
		 	 uint16_t flags)
{
	uint64_t *parent_table = page_table_root, *child_table;
	for(int i = 3; i >= 1; --i) {
		uint64_t tab_index = V_ADDR_INDEX(vaddr, i + 1);
		uint64_t *child_table = GetOrCreatePageTable(parent_table, tab_index, 
													 flags);
		if(child_table == NULL)
			return false;
		parent_table = child_table;
	}
	
	child_table[V_ADDR_INDEX(vaddr, 1)] = paddr | flags;
	return true;
}

bool MapMultiple(uint64_t *page_table_root, uint64_t base, uint64_t bound,
				 uint64_t offset, uint16_t flags)
{
	PrintK("MapMultiple: Mapping %h-%h w/ offset %h\n\0", base, bound, offset);
	bool success = true;
	for(int addr = base; addr < bound; addr += FRAME_SIZE) {
		success &= MapPage(page_table_root, addr + offset, addr, flags);
		if(! success)
			return false;
	}
	return true;
}

bool MapMultipleKernel(uint64_t base, uint64_t bound, uint64_t offset, uint16_t flags)
{
	return MapMultiple(KERNEL_PAGE_TABLE_ROOT, base, bound, offset, flags);
}

bool MapKernelPage(uint64_t vaddr, uint64_t paddr, uint16_t flags)
{
	return MapPage(KERNEL_PAGE_TABLE_ROOT, vaddr, paddr, flags);
}

bool UnmapPage(uint64_t *page_table_root, uint64_t vaddr)
{
	uint64_t *parent_table = page_table_root, *child_table;
	for(int i = 3; i >= 1; --i) {
		uint64_t tab_index = V_ADDR_INDEX(vaddr, i + 1);
		child_table = GetPageTable(parent_table, tab_index);
		if(child_table == NULL)
			return false;
		parent_table = child_table;
	}

	child_table[V_ADDR_INDEX(vaddr, 1)] = 0;
	__asm__ volatile("invlpg (%[pg_addr])" ::[pg_addr] "r" (vaddr));
	return true;
}

bool UnmapKernelPage(uint64_t vaddr)
{
	return UnmapPage(KERNEL_PAGE_TABLE_ROOT, vaddr);
}

bool RemapPage(uint64_t *page_table_root, uint64_t former_vaddr, uint64_t new_vaddr, 
			   uint16_t flags)
{
	uint64_t paddr 	= VAddrToPAddr(page_table_root, former_vaddr);
	int unmap_code 	= UnmapPage(page_table_root, former_vaddr);
	int map_code 	= MapPage(page_table_root, former_vaddr, paddr, flags);
	return unmap_code && map_code; 	
}

bool RemapKernelPage(uint64_t former_vaddr, uint64_t new_vaddr, uint16_t flags)
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
		uint64_t tab_index = V_ADDR_INDEX(vaddr, i + 1);
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
