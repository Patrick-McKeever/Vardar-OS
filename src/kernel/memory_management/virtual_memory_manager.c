#include "virtual_memory_manager.h"
#include "utils/string.h"
#include "utils/printf.h"

static uint64_t *KERNEL_PAGE_TABLE_ROOT;

static void PrintKernelRoot() {
	uint64_t *root =GetPageTable(KERNEL_PAGE_TABLE_ROOT, 0);
	for(int i = 0; i < 128; ++i) {
		PrintK("Entry %d: 0x%h\t\0", i, root[i]);
		if(i % 8 == 0)
			PrintK("\n\0");
	}
}


bool InitPageTable(struct stivale2_struct_tag_memmap *memmap,
				   struct stivale2_struct_tag_kernel_base_address *kern_base_addr,
				   struct stivale2_struct_tag_pmrs *pmrs)
{
	KERNEL_PAGE_TABLE_ROOT = AllocFirstFrame();
	if(KERNEL_PAGE_TABLE_ROOT == NULL)
		return false;
	
	bool success = true;

	// Because we requested fully virtual mappings, we cannot immediately calc-
	// ulate the correspondence between a virtual higher-half address and a
	// physical address using offsets. Instead, we accept that the kernel will
	// be placed at some arbitrary physical address and mapped to some arbitrary
	// virtual address, and trust that the bootloader will report them correctly.
	uint64_t kern_phys_base = kern_base_addr->physical_base_address;
	uint64_t kern_virt_base = kern_base_addr->virtual_base_address;
	uint16_t kernel_mem = PRESENT | READ_WRITABLE;
	
	// Protected memory ranges (PMRs) describe ELF segments of kernel/code data,
	// giving their location in physical/virtual memory as determined by the
	// stivale2 bootloader.
	for(uint64_t i = 0; i < pmrs->entries; ++i) {
		struct stivale2_pmr pmr = pmrs->pmrs[i];
		uint64_t virt = pmr.base;
		uint64_t phys = kern_phys_base + (pmr.base - kern_virt_base);
		uint64_t len  = pmr.length;
	
		// Map all page frames in PMR to a virtual address determined by the
		// offset between the PMR's physical and virtual address. 
		uint64_t offset	= virt - phys;
		success &= MapMultipleKernel(phys, phys + len, offset, kernel_mem);
	}

	// Identity map 0x1000-4GiB.
	success &= MapMultipleKernel(0x1000, 0x100000000, 0, kernel_mem);
	// Map 0x1000-4GiB to higher half for kernel data.
	success &= MapMultipleKernel(0x1000, 0x100000000, KERNEL_DATA, kernel_mem);
	
	for(uint32_t i = 0; i < memmap->entries; ++i) {
		uint64_t base = memmap->memmap[i].base;
		uint64_t bound = base + memmap->memmap[i].length;
		//uint64_t type = memmap->memmap[i].type;

		// If we already mapped this, continue.
		if(bound <= 0x100000000)
			continue;	
		
		success &= MapMultipleKernel(base, bound, 0, kernel_mem);
		success &= MapMultipleKernel(base, bound, KERNEL_DATA, kernel_mem);
	}

	
	__asm__ volatile("mov %0, %%cr3" :: 
					 "r" ((uint64_t) KERNEL_PAGE_TABLE_ROOT));
	return success;
}

//bool InitPageTable(struct stivale2_struct_tag_memmap *memmap)
//{
//	KERNEL_PAGE_TABLE_ROOT = AllocFirstFrame();
//	__asm__ volatile("mov %%cr3, %%rax;"
//					 "mov %%rax, %0;" 
//					 : "=r" (KERNEL_PAGE_TABLE_ROOT)
//					 :);
//	//bool succ = true;
//	//// Probably want to do sth with PAT here.
//	//for(uint32_t i = 0; i < memmap->entries; ++i) {
//	//	uint64_t base = memmap->memmap[i].base;
//	//	uint64_t bound = base + memmap->memmap[i].length;
//	//	uint64_t type = memmap->memmap[i].type;
//	//	
//	//	switch(type) {
//	//	case STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE:
//	//		succ &= MapMultipleKernel(base, bound, 0, PRESENT);
//	//		break;
//	//	case STIVALE2_MMAP_FRAMEBUFFER:
//	//		succ &= MapMultipleKernel(base, bound, 0, PRESENT);
//	//	}
//	//}
//	//
//	//PrintK("0x5F73000 MAPPED TO 0x%h\n\0", 
//	//		KernelVAddrToPAddr(0x5F73000));
//
//	//uint16_t kernel_mem = PRESENT | READ_WRITABLE;
//	//// Identity map 4GB.
//	//succ &= MapMultipleKernel(0, 0x100000000, 0, kernel_mem);
//	//// Map 2GB of kernel data, 2GB of kernel code.
//	//succ &= MapMultipleKernel(0, 0x80000000, KERNEL_DATA, kernel_mem);
//	//succ &= MapMultipleKernel(0, 0x80000000, KERNEL_CODE, PRESENT);
//	//PrintK("Writing back page table\n\0");
//	//__asm__ volatile("mov %0, %%cr3" :: 
//	//				 "r" ((uint64_t) KERNEL_PAGE_TABLE_ROOT));
//	//PrintK("Wrote back page table\n\0");
//	return true;
//}

bool MapPage(uint64_t *page_table_root, uint64_t vaddr, uint64_t paddr, 
		 	 uint16_t flags)
{
	uint64_t *parent_table = page_table_root, *child_table;
	for(int i = 4; i > 1; --i) {
		uint64_t tab_index = V_ADDR_INDEX(vaddr, i);
		uint64_t *child_table = GetOrCreatePageTable(parent_table, tab_index, 
													 flags);
		if(child_table == NULL)
			return false;
		parent_table = child_table;
	}
	
	parent_table[V_ADDR_INDEX(vaddr, 1)] = paddr | flags;
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
	for(int i = 4; i > 1; --i) {
		uint64_t tab_index = V_ADDR_INDEX(vaddr, i);
		child_table = GetPageTable(parent_table, tab_index);
		if(child_table == NULL)
			return false;
		parent_table = child_table;
	}

	parent_table[V_ADDR_INDEX(vaddr, 1)] = 0;
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
	for(int i = 4; i > 1; --i) {
		uint64_t tab_index = V_ADDR_INDEX(vaddr, i);
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
