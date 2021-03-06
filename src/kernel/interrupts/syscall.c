#include "interrupts/syscall.h"
#include "graphics/terminal.h"
#include "proc/sched.h"
#include "utils/printf.h"

static syscall_handler_t SYSCALLS[256];

__attribute__((sysv_abi))
void Isr80Handler(const registers_t *const regs, const control_registers_t *const cregs)
{
	syscall_handler_t handler = SYSCALLS[regs->rax];
	(*handler)(regs);
	return;
}


void register_syscall(uint64_t rax, syscall_handler_t handler)
{
	SYSCALLS[rax] = handler;
}

void syscall_1(const registers_t *const regs)
{
	PrintK((char*)regs->r8);
}

void syscall_3c(const registers_t *const regs)
{
	for(;;);
	return;
}
