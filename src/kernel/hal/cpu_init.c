#include "hal/cpu_init.h"
#include "utils/printf.h"
#include "memory_management/physical_memory_manager.h"

void startup_aps(struct stivale2_struct_tag_smp *cpu_info)
{
	for(int i = 0; i < cpu_info->cpu_count; ++i) {
		if(cpu_info->smp_info[i].lapic_id != cpu_info->bsp_lapic_id) {
			cpu_info->smp_info[i].target_stack = AllocFirstFrame();
			cpu_info->smp_info[i].goto_address = ((uint64_t) &ap_entry);
		}
	}
}

void ap_entry()
{
	PrintK("Processor online.\n");
	for(;;)
		asm("hlt");
}
