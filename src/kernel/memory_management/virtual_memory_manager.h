#ifndef VIRTUAL_MEMORY_MANAGER_H
#define VIRTUAL_MEMORY_MANAGER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "memory_management/physical_memory_manager.h"
#include "utils/printf.h"
#include "stivale2.h"

#define KERNEL_DATA					0xffff800000000000
#define KERNEL_CODE					0xffffffff80000000

// Page flags.
#define PRESENT						(1)
#define READ_WRITABLE				(1 << 1)
#define USER_ACCESSIBLE				(1 << 2)	
#define WRITE_THROUGH				(1 << 3)	
#define CACHE_DISABLED				(1 << 4)	
#define ACCESSED					(1 << 5)	
#define DIRTY						(1 << 6)	
#define PAGE_ATTRIBUTE_TABLE		(1 << 7)	
#define GLOBAL						(1 << 8)	
#define EXECUTABLE					(~(1UL << 62))

#define KERNEL_PAGE					(PRESENT | READ_WRITABLE)
#define USER_PAGE					(PRESENT | READ_WRITABLE | USER_ACCESSIBLE)

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

/**
 * Initialize the page table with default stivale mappings: 4 GiB identity
 * mapped, 2GiB of kernel  data at 0xffff8..., 2 GiB of kernel code at
 * 0xffffffff8...
 * @input memmap A struct listing ranges of memory and their uses.
 * @input kern_base_addr A struct giving the physical base address of kernel
 * 						 code (since we are using fully virtual mappings, the
 * 						 physical addr of kernel code can be arbitrary).
 * @input pmrs A struct listing Protected Memory Ranges (PMRs), i.e. ranges
 * 			   of kernel code in physical memory.
 * @output True if page table is succesfully allocated/initialized, false 
 * 		   otherwise.
 */
bool InitPageTable(struct stivale2_struct_tag_memmap *memmap,
				   struct stivale2_struct_tag_kernel_base_address *kern_base_addr,
				   struct stivale2_struct_tag_pmrs *pmrs);

bool MapKernelPmrs(uint64_t *page_table_root);

/**
 * Given a PML4 table and a virtual address, return a pointer to the page table
 * entry corresponding to this address.
 * @input page_table_root Ptr to PML4 table.
 * @input vaddr The virtual address to lookup.
 * @input create Should the entry be created if it does not exist (t/f)?
 * @output A pointer to the virtual address' entry in the bottom-level page 
 * 		   table.
 */
uint64_t *GetPage(uint64_t *page_table_root, uint64_t vaddr);

/**
 * Given a virtual address, return a pointer to the corresponding entry in the
 * kernel's page table.
 * @input vaddr The virtual address to lookup.
 * @output A pointer to the virtual address' entry in the kernel's page table.
 */
uint64_t *GetKernelPage(uint64_t vaddr);

/**
 * Given a PML4 and a virtual address, create a page table entry for said vaddr
 * and retur a ptr to it.
 * @input page_table_root The PML4 to query/edit.
 * @output Ptr to created page table entry if PMM alloc succeeded, NULL 
 * 		   otherwise.
 */
uint64_t *CreatePage(uint64_t *page_table_root, uint64_t vaddr, 
					 uint16_t flags);

/**
 * Given a PML4 table, map a vaddr to a paddr within that table and set its 
 * flags.
 * @input page_table_root The PML4 table in which we will map the vaddr.
 * @input vaddr The virtual address to map.
 * @input paddr The physical address to which the virtual address will corresp-
 * 				ond.
 * @input flags The flags of the page table mapping, e.g. bit 1 is present,
 * 				 bit 2 is read/writable, etc.
 * @output True if allocation and creation of page table entries was successful,
 * 		   false otherwise.
 */
bool MapPage(uint64_t *page_table_root, uint64_t vaddr, uint64_t paddr, 
		 	 uint16_t flags);

/**
 * Identical to MapPage, but mapping takes place within the kernel page table
 * by default.
 */
bool MapKernelPage(uint64_t vaddr, uint64_t paddr, uint16_t flags);

/**
 * For physical addrs in range [base, bound), map to virtual addrs
 * [base+offset, bound+offset).
 * @input page_table_root The page table in which the mapping will take place.
 * @input base The lowest physical addr to map.
 * @input bound The highest (non-inclusive) physical addr to map.
 * @input offset The difference between desired virtual addrs and the physical
 * 				 addrs. E.g. mapping paddrs [10,20) w/ offset of 10 will give
 * 				 virtual addrs [20, 30).
 * @input flags The flags of the page table mappings.
 * @output True if allocation/insertion of table entries was succesful, false
 * 		   otherwise.
 */
bool MapMultiple(uint64_t *page_table_root, uint64_t base, uint64_t bound,
				 uint64_t offset, uint16_t flags);

/**
 * Identical to MapMultiple, but uses kernel page table by default.
 */
bool MapMultipleKernel(uint64_t base, uint64_t bound, uint64_t offset, 
					   uint16_t flags);

/**
 * Remove a kernel page mapping for a given virtual address.
 * @input pagemap The pagemap on which the relevant mapping exists.
 * @input vaddr The virtual addr to unmap.
 * @output True if virtual addr existed in table and was unmapped, false if the
 * 		   virtual addr did not exist in the table.
 */
bool UnmapPage(uint64_t *pagemap, uint64_t vaddr);

/**
 * Identical to UnmapPage, but uses kernel page table by default.
 */
bool UnmapKernelPage(uint64_t vaddr);

/**
 * Remove a physical address' current mapping and replace it with a new one.
 * @input table The PML4 to be edited.
 * @input former_vaddr The vaddr to be removed.
 * @input new_vaddr The vaddr to which the physical addr corresponding to
 * 					former_vaddr ought to be mapped.
 * @input flags The flags of the new mapping.
 * @output True if UnmapPage and MapPage both return true, false otherwise.
 */
bool RemapPage(uint64_t *table, uint64_t former_vaddr, uint64_t new_vaddr, 
			   uint16_t flags);

/**
 * Identicla to RemapPage, but uses the kernel page table by default.
 */
bool RemapKernelPage(uint64_t former_vaddr, uint64_t new_vaddr, uint16_t flags);

/**
 * Retrieve the physical address corresponding to some virtual address from a
 * page table.
 * @input table The PML4 from which we will retrieve the physical address.
 * @input vaddr The virtual address whose corresponding physical addr will be
 * 				returned.
 * @output The physical address corresponding to vaddr.
 */
uint64_t VAddrToPAddr(uint64_t *table, uint64_t vaddr);

/**
 * Identical to VAddrToPaddr, but uses kernel page table by default.
 */
uint64_t KernelVAddrToPAddr(uint64_t vaddr);

/**
 * Given a page table entry, retrieve whether or not a given flag is set.
 * @input page An entry from a bottom-level page table.
 * @input flag The value of the flag when set. E.g. (1 << 1) for present
 * 			   (1 << 2) for read/writable. For simplicity's sake, just use
 * 			   flag macros from top of file. E.g. GetPageFlag(page, PRESENT)
 * @output True if flag is set, false otherwise.
 */
bool GetPageFlag(uint64_t page, uint64_t flag);

/**
 * Print the attributes of the entry corresponding to some virtual address given
 * the PML4 in which it resides.
 * @input page_table_root The PML4 in which the page table entry resides.
 * @input virt_addr The virtual address to lookup and retrieve the attributes of.
 */
void PrintPageAttrs(uint64_t *page_table_root, uint64_t virt_addr);

/**
 * Identical to PrintPageAttrs, but uses kernel page table by default.
 */
void PrintKernelPageAttrs(uint64_t virt_addr);

#endif

