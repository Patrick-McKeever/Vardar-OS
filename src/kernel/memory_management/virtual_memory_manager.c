#include "virtual_memory_manager.h"
#include "utils/string.h"
#include "utils/printf.h"

static uint64_t *KERNEL_PAGE_TABLE_ROOT;

static bool AssertKernelMapping(uint64_t phys_addr, uint64_t virt_addr)
{
	
	if(KERNEL_PAGE_TABLE_ROOT == NULL) 
		return false;	
	
	// We're going to use this as opposed to our GetPage method, because this
	// code is taken from the OSDev wiki. We know it functions correctly, as
	// opposed to our code, which may be faulty. So, we should use this for
	// testing purposes.
	uint64_t pml4_entry = (virt_addr & (0x1ffUL << 39)) >> 39;
	uint64_t pml3_entry = (virt_addr & (0x1ffUL << 30)) >> 30;
	uint64_t pml2_entry = (virt_addr & (0x1ffUL << 21)) >> 21;
	uint64_t pml1_entry = (virt_addr & (0x1ffUL << 12)) >> 12;
	
	uint64_t *pml3, *pml2, *pml1;
	
	// If any entry in the desired path is not marked present, return false.
	if(! (pml3 = GetPageTable(KERNEL_PAGE_TABLE_ROOT, pml4_entry)))
		return false;
		
	if(! (pml2 = GetPageTable(pml3, pml3_entry)))
		return false;

	if(! (pml1 = GetPageTable(pml2, pml2_entry)))
		return false;

	if(! pml1[pml1_entry] & PRESENT)
		return false;
	
	uint64_t mapped_paddr = pml1[pml1_entry] & ~(511);
	return mapped_paddr == phys_addr;
}

static void TestKernelMapping(uint64_t phys_addr, uint64_t virt_addr)
{
	if(AssertKernelMapping(phys_addr, virt_addr))
		PrintK("Testing kernel mapping %h -> %h: Success.\n", 
				phys_addr, virt_addr);
	else
		PrintK("Testing kernel mapping %h -> %h: Failure.\n", 
				phys_addr, virt_addr);
}

void PrintPageAttrs(uint64_t *page_table_root, uint64_t virt_addr)
{
	uint64_t *page = GetPage(page_table_root, virt_addr);
	if(page == NULL) {
		PrintK("Addr %h has not been mapped.", virt_addr);
		return;
	}
	
	PrintK("%h:\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s",
		virt_addr,
		GetPageFlag(*page, PRESENT) 				? 
			"PRESENT" 			: 	"NOT-PRESENT",
		GetPageFlag(*page, READ_WRITABLE) 			? 
			"READ-WRITABLE" 	: 	"READ-ONLY",
		GetPageFlag(*page, USER_ACCESSIBLE) 		? 
			"USER-ACCESSIBLE" 	: 	"SUPERVISOR-ONLY",
		GetPageFlag(*page, WRITE_THROUGH) 			? 
			"WRITE-THROUGH" 	: 	"WRITE-BACK",
		GetPageFlag(*page, CACHE_DISABLED) 			? 
			"CACHE-DISABLED" 	: 	"CACHE-ENABLED",
		GetPageFlag(*page, ACCESSED) 				? 
			"ACCESSED" 			: 	"UNTOUCHED",
		GetPageFlag(*page, DIRTY) 					? 
			"DIRTY" 			: 	"CLEAN",
		GetPageFlag(*page, PAGE_ATTRIBUTE_TABLE) 	? 
			"PAT" 				: 	"NO-PAT"
	);
}

void PrintKernelPageAttrs(uint64_t virt_addr)
{
	PrintPageAttrs(KERNEL_PAGE_TABLE_ROOT, virt_addr);
}

bool InitPageTable(struct stivale2_struct_tag_memmap *memmap,
				   struct stivale2_struct_tag_kernel_base_address *kern_base_addr,
				   struct stivale2_struct_tag_pmrs *pmrs)
{
	KERNEL_PAGE_TABLE_ROOT = AllocFirstFrame();
	if(KERNEL_PAGE_TABLE_ROOT == NULL)
		return false;
	

	PrintK("PMM: %h frames free\n", NumFreeFrames());
	bool success = true;

	// Because we requested fully virtual mappings, we cannot immediately calc-
	// ulate the correspondence between a virtual higher-half address and a
	// physical address using offsets. Instead, we accept that the kernel will
	// be placed at some arbitrary physical address and mapped to some arbitrary
	// virtual address, and trust that the bootloader will report them correctly.
	uint64_t kern_phys_base = kern_base_addr->physical_base_address;
	uint64_t kern_virt_base = kern_base_addr->virtual_base_address;
	uint16_t kernel_mem = PRESENT | READ_WRITABLE;
	
	// Protected memory ranges (PMRs) describe ELF segments of kernel code,
	// giving their location in physical/virtual memory as determined by the
	// stivale2 bootloader. Here, we map all segments of kernel code.
	for(uint64_t i = 0; i < pmrs->entries; ++i) {
		struct stivale2_pmr pmr = pmrs->pmrs[i];
		uint64_t virt = pmr.base;
		uint64_t phys = kern_phys_base + (pmr.base - kern_virt_base);
		uint64_t len  = pmr.length;
	
		// Map all page frames in PMR to a virtual address determined by the
		// offset between the PMR's physical and virtual address. 
		uint64_t offset	= virt - phys;
		success &= MapMultipleKernel(phys, phys + len, offset, kernel_mem);
		TestKernelMapping(phys, phys + offset);
	}

	// Identity map 0x1000-4GiB.
	uint64_t four_gb = 0x100000000;
	success &= MapMultipleKernel(0x1000, four_gb, 0, kernel_mem);
	PrintK("PMM: %h frames free\n", NumFreeFrames());
	// Map 0x1000-4GiB to higher half for kernel data.
	success &= MapMultipleKernel(0x1000, four_gb, KERNEL_DATA, kernel_mem);
	PrintK("PMM: %h frames free\n", NumFreeFrames());
	//MapKernelPage(0xdd2000000, 0xffff8000dd20000, 0x3);
	// Just for debugging, remove later.
	//MapMultipleKernel(0xDD000000, 0x100000000, KERNEL_DATA, kernel_mem);
	
	for(uint32_t i = 0; i < memmap->entries; ++i) {
		uint64_t base = memmap->memmap[i].base;
		uint64_t bound = base + memmap->memmap[i].length;
		uint64_t type = memmap->memmap[i].type;

		if(type == STIVALE2_MMAP_FRAMEBUFFER)
			PrintK("Range %h-%h is framebuffer.\n", base, bound);

		// If we already mapped this, continue.
		if(bound <= 0x100000000)
			continue;	
		
		success &= MapMultipleKernel(base, bound, 0, kernel_mem);
		success &= MapMultipleKernel(base, bound, KERNEL_DATA, kernel_mem);
	}

	PrintKernelPageAttrs(0xffff8000fd000000);
	
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

uint64_t *GetPage(uint64_t *page_table_root, uint64_t vaddr)
{
	
	uint64_t *parent_table = page_table_root, *child_table;
	for(int i = 4; i > 1; --i) {
		uint64_t tab_index = V_ADDR_INDEX(vaddr, i);
		uint64_t *child_table = GetPageTable(parent_table, tab_index);
		if(child_table == NULL)
			return NULL;
		parent_table = child_table;
	}

	return &parent_table[V_ADDR_INDEX(vaddr, 1)];
}
uint64_t *GetKernelPage(uint64_t vaddr)
{
	return GetPage(KERNEL_PAGE_TABLE_ROOT, vaddr);
}

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
	PrintK("MapMultiple: Mapping %h-%h w/ to %h-%h\n\0", 
			base, bound, base + offset, bound + offset);
	for(uint64_t addr = base; addr < bound; addr += FRAME_SIZE) {
		//if((addr + offset) % 0x1000000 == 0)
			//PrintK("Mapping %h\n", (addr + offset));

		uint64_t vaddr = addr + offset;	
		bool success = MapPage(page_table_root, vaddr, addr, flags);
		if(! success) {
			PrintK("Failed mapping %h->%h\n", addr, vaddr);
			return false;
		}
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
