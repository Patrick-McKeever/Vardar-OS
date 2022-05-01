#include "proc/elf.h"
#include "proc/proc.h"
#include "memory_management/physical_memory_manager.h"
#include "memory_management/virtual_memory_manager.h"
#include "utils/string.h"
#include "stivale2.h"

static const char ELF_MAGIC[4] = { 0x7F, 'E', 'L', 'F' };
static const uint16_t USER_PROC_PAGE = PRESENT | READ_WRITABLE | USER_ACCESSIBLE;

static inline uint8_t*
find_shdr_strtab(uint8_t *raw_elf, uint16_t num_shdrs, elf_shdr_t *shdrs)
{
	for(int i = 0; i < num_shdrs; ++i) {
		if(shdrs[i].type == ELF_SHDR_STRTAB) {
			return (raw_elf + shdrs[i].offset);
		}
	}
	return NULL;
}

int
parse_elf(uint8_t *raw_elf, pcb_t *pcb)
{
	pcb->pagemap = AllocFirstFrame();

	elf_hdr_t *header = (elf_hdr_t*) raw_elf;
	// Verify that header contains magic number.
	if(strncmp(header->magic, ELF_MAGIC, 4)) {
		PrintK("ELF magic not detected.\n");
		return -1;
	}

	// The process level pagemap should still contain all relevant kernel data,
	// which is necessary to restore kernel state after interrupts. (Also, you
	// can't just throw out things like the GDT, IDT, etc.) Here, we create those
	// mappings in the process-level pagemap. So, map 0x1000-4GiB to higher half,
	// and map PMRs as specified.
	uint64_t four_gb = 0x100000000;
	MapKernelPmrs(pcb->pagemap);

	uint64_t paddr = VAddrToPAddr(pcb->pagemap, 0xffffffff80000ba9);
	PrintK("Inst paddr listed as 0x%h, vaddr 0x%h\n", paddr, 0xffffffff80000ba9);

	// Find entry point, set RIP equal to entry point.
	uint64_t entry_pt = header->entry_pt;

	// For each of the program segments, create relevant user level pages in the
	// process page table.
	elf_phdr_t *phdrs = (elf_phdr_t*) (raw_elf + header->phdr_offset);
	for(size_t i = 0; i < header->phdr_num_entries; ++i) {
		if(phdrs[i].type == ELF_PHDR_LOAD) {
			uint8_t *segment = (raw_elf + phdrs[i].offset);
			size_t seg_base = phdrs[i].vaddr;
			size_t seg_bound = seg_base + phdrs[i].mem_size;

			// Calculate the virtual address of the entry point. This will just be
			// the virtual address of this segment plus the offset of the entry point
			// from the start of this segment. Set proc's rip equal to this value.
			if(entry_pt >= seg_base && entry_pt <= seg_bound) {
				pcb->registers.rip = phdrs[i].vaddr + (entry_pt - phdrs[i].offset);
			}

			// Round up bound to a power of 0x1000.
			seg_bound = (((seg_bound + (1 << LOG2_FRAME_SIZE) - 1) >> LOG2_FRAME_SIZE) 
							<< LOG2_FRAME_SIZE);

			// For each page of memory in segment, copy it into a newly-allocated page frame,
			// and map the page frame to the desired location in virtual memory.
			for(size_t off = 0; off < seg_bound; off += 0x1000) {
				void *frame = AllocFirstFrame();
				memmove(frame, segment + off, 0x1000);
				MapPage(pcb->pagemap, seg_base + off, (uintptr_t) frame, USER_PROC_PAGE);
			}
		}
	}

	// Create stack, map it, and set processor RSP/RBP equal to top of stack.
	void *stack = AllocFirstFrame();
	MapPage(pcb->pagemap, DEFAULT_STACK_BASE, (uintptr_t) stack, USER_PROC_PAGE);
	pcb->registers.rbp = DEFAULT_STACK_BASE;
	pcb->registers.rsp = DEFAULT_STACK_BASE;

	paddr = VAddrToPAddr(pcb->pagemap, DEFAULT_STACK_BASE);
	//asm volatile("mov %0, %%cr3" ::"r"((uintptr_t) &pcb->pagemap):);
	PrintK("Stack paddr listed as 0x%h, should be 0x%h\n", paddr, (uintptr_t) stack);


	// As of now, we are not attempting to parse section headers. However, this will
	// become relevant when we attempt to make a dynamic linker.
	//elf_shdr_t *shdrs = (elf_shdr_t*) (raw_elf + header->shdr_offset);
	//uint8_t *shstrtab = find_shdr_strtab(raw_elf, header->shdr_num_entries, shdrs);

	return 0;
}

