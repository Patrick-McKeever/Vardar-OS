#ifndef CPU_H
#define CPU_H

#include "memory_manager/physical_memory_manager.h"
#include "stivale2.h"

int init_cpus(struct stivale2_struct_tag_smp *cpu_list)
{
	for(int i = 0; i < cpu_list->cpu_count; ++i) {
		struct stivale2_smp_info cpu = cpu_list->smp_info[i];
		
		// If this CPU is the bootstrap processor (BSP), then it has already 
		// been initialized (and is running this code), so don't bother
		// initializing it.
		if(cpu.lapic_id == cpu_list.bsp_lapic_id)
			continue;
		init_cpu(&cpu);
	}
}

int init_cpu(struct stivale2_smp_info *cpu)
{
	// Provide a stack for the new CPU.
	cpu.target_stack = AllocFirstFrame();
	// The bootloader greatly simplifies the process of CPU startup. It will
	// continually poll this field of the struct until an atomic write occurs.
	// At this point, the specified CPU will start up and jump to the specified
	// address.
	// What should I put as this address?
	cpu.goto_addr = 0;
}

#endif
