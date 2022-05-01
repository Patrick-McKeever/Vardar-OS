#include "proc.h"
#include "gdt/gdt.h"
#include "memory_management/virtual_memory_manager.h"

void
run_proc(pcb_t *pcb)
{
	uint64_t ds_segsel = USER_DS_SEGSEL;
	uint64_t cs_segsel = USER_CS_SEGSEL;
	uint64_t pt_paddr = KernelVAddrToPAddr((uintptr_t) pcb->pagemap);
	uint64_t inst_paddr = VAddrToPAddr((uint64_t*) pcb->pagemap, pcb->registers.rip);
	PrintK("Entry pt maps to: 0x%h\n", inst_paddr);
	asm volatile("mov %0, %%cr3" :: "r"(pt_paddr) :);

	asm volatile(
			"push %0\n\t"
			"push %1\n\t"
			"push %2\n\t"
			"push %3\n\t"
			"push %4\n\t"
			"push %5\n\t"
			"push %6\n\t"
			"push %7\n\t"
			"push %8\n\t"
			"push %9\n\t"
			"push %10\n\t"
			"push %11\n\t"
			"push %12\n\t"
			"push %13\n\t"
			"pop %%rax\n\t"
			"pop %%rbx\n\t"
			"pop %%rcx\n\t"
			"pop %%rdx\n\t"
			"pop %%rdi\n\t"
			"pop %%rsi\n\t"
			"pop %%r8\n\t"
			"pop %%r9\n\t"
			"pop %%r10\n\t"
			"pop %%r11\n\t"
			"pop %%r12\n\t"
			"pop %%r13\n\t"
			"pop %%r14\n\t"
			"pop %%r15\n\t"
			/* DS */
			"push %14\n\t"
			/* RSP */
			"push %15\n\t"
			/* RFLAGS */
			"pushfq\n\t"
			/* CS */
			"push %16\n\t"
			/* RIP */
			"push %17\n\t"
			"mov %18, %%rbp\n\t"
			"iretq\n\t"
		:
		:	"g"(pcb->registers.rax),
			"g"(pcb->registers.rbx),
			"g"(pcb->registers.rcx),
			"g"(pcb->registers.rdx),
			"g"(pcb->registers.rdi),
			"g"(pcb->registers.rsi),
			"g"(pcb->registers.r8),
			"g"(pcb->registers.r9),
			"g"(pcb->registers.r10),
			"g"(pcb->registers.r11),
			"g"(pcb->registers.r12),
			"g"(pcb->registers.r13),
			"g"(pcb->registers.r14),
			"g"(pcb->registers.r15),
			"g"(ds_segsel),
			"g"(pcb->registers.rsp),
			"g"(cs_segsel),
			"g"(pcb->registers.rip),
			"g"(pcb->registers.rbp)
		:
	);
}

