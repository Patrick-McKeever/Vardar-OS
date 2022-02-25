#include "virtual_memory_manager.h"
#include "utils/string.h"
#include "utils/printf.h"

// Static declarations.
static uint64_t *KERNEL_PAGE_TABLE_ROOT;

static inline uint64_t *GetOrCreatePageTable(uint64_t *parent, uint64_t index, 
											 uint16_t flags);
static inline uint64_t *GetPageTable(uint64_t *parent, uint64_t index);
static bool AssertKernelMapping(uint64_t phys_addr, uint64_t virt_addr);
static void TestKernelMapping(uint64_t phys_addr, uint64_t virt_addr);

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
	// Map 0x1000-4GiB to higher half for kernel data.
	success &= MapMultipleKernel(0x1000, four_gb, KERNEL_DATA, kernel_mem);
	
	for(uint32_t i = 0; i < memmap->entries; ++i) {
		uint64_t base = memmap->memmap[i].base;
		uint64_t bound = base + memmap->memmap[i].length;
		uint64_t type = memmap->memmap[i].type;

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

uint64_t *GetPage(uint64_t *page_table_root, uint64_t vaddr)
{
	
	uint64_t *parent_table = page_table_root;
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

uint64_t *CreatePage(uint64_t *page_table_root, uint64_t vaddr, 
					 uint16_t flags)
{
	uint64_t *parent_table = page_table_root;
	for(int i = 4; i > 1; --i) {
		uint64_t tab_index = V_ADDR_INDEX(vaddr, i);
		uint64_t *child_table = GetOrCreatePageTable(parent_table, tab_index,
													 flags);
		if(child_table == NULL)
			return NULL;
		parent_table = child_table;
	}

	return &parent_table[V_ADDR_INDEX(vaddr, 1)];
}

bool MapPage(uint64_t *page_table_root, uint64_t vaddr, uint64_t paddr, 
		 	 uint16_t flags)
{
	uint64_t *page_table_entry = CreatePage(page_table_root, vaddr, flags);
	if(page_table_entry == NULL)
		return false;
	*page_table_entry = paddr | flags;
	return true;
}

bool MapMultiple(uint64_t *page_table_root, uint64_t base, uint64_t bound,
				 uint64_t offset, uint16_t flags)
{
	PrintK("MapMultiple: Mapping %h-%h w/ to %h-%h\n\0", 
			base, bound, base + offset, bound + offset);
	for(uint64_t addr = base; addr < bound; addr += FRAME_SIZE) {
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
	uint64_t *page_table_entry = GetPage(page_table_root, vaddr);
	if(page_table_entry == NULL)
		return false;

	*page_table_entry = 0;
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

/**
 * Look up the index-th entry of nth-level page table parent. If this entry has 
 * been set, return the pointer to the corresponding (n+1)th level page table.
 * If not, allocate the (n+1)th level page table, create an entry for it, and
 * return a ptr to it.
 * @input parent The parent of the page table to create/retrieve.
 * @input index The index of the page table to lookup.
 * @input flags The flags to be set for this page table if it must be created.
 * @output A pointer to the page table if PMM allocation succeeded, NULL 
 * 		   otherwise.
 */
static inline uint64_t *GetOrCreatePageTable(uint64_t *parent, uint64_t index, 
											 uint16_t flags)
{
	if(GetPageFlag(parent[index], PRESENT))
		return (uint64_t*) (parent[index] & ~(511));
	
	void *free_frame = AllocFirstFrame();	
	if(free_frame == NULL) {
		return NULL;
	}

	parent[index] = ((uint64_t) free_frame) | flags;
	return free_frame;
}

/**
 * Given a page table and an index, return the page table pointed to by the
 * index-th entry if it exists, otherwise return NULL.
 * @input parent The parent of the page table to retrieve.
 * @input index The index of the page table to retrieve.
 * @output A pointer to the page table if it exists, otherwise NULL.
 */
static inline uint64_t *GetPageTable(uint64_t *parent, uint64_t index)
{
	if(GetPageFlag(parent[index], PRESENT))
		return (uint64_t*) (parent[index] & ~(511));
	return NULL;
}


/**
 * A way to test that a physical address has been correctly mapped to a virtual
 * address in the kernel page table.
 * @input phys_addr The physical address to which a virtual address should be
 * 					mapped.
 * @input virt_addr The virtual address whose physical address will be compared
 * 					with the given physical address.
 * @output True if the virtual address maps to the physical address, false
 * 		   otherwise.
 */
static bool AssertKernelMapping(uint64_t phys_addr, uint64_t virt_addr)
{
	
	if(KERNEL_PAGE_TABLE_ROOT == NULL) 
		return false;	
	
	// We're going to use this as opposed to our GetPage method, because this
	// lookup code is taken from the OSDev wiki. We know it functions correctly, 
	// as opposed to our code, which may be faulty. So, we should use this for
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

/**
 * (Dumb debugging function, remove later).
 * Determine whether virtual address maps to physical address, print out an
 * appropriate message to terminal depending on whether this is the case.
 * @input phys_addr The physical addr to which a virtual addr should map.
 * @input virt_addr The virtual address to lookup.
 */
static void TestKernelMapping(uint64_t phys_addr, uint64_t virt_addr)
{
	if(AssertKernelMapping(phys_addr, virt_addr))
		PrintK("Testing kernel mapping %h -> %h: Success.\n", 
				phys_addr, virt_addr);
	else
		PrintK("Testing kernel mapping %h -> %h: Failure.\n", 
				phys_addr, virt_addr);
}

