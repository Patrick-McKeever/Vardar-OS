#include "hal/cpu_init.h"
#include "hal/lapic.h"
#include "utils/printf.h"
#include "memory_management/physical_memory_manager.h"

static uint8_t bsp_lapic_id;

void startup_aps(struct stivale2_struct_tag_smp *cpu_info)
{
	enable_lapic();
	lapic_timer_init(0xFF);
	bsp_lapic_id = cpu_info->bsp_lapic_id;
	for(int i = 0; i < cpu_info->cpu_count; ++i) {
		if(cpu_info->smp_info[i].lapic_id != cpu_info->bsp_lapic_id) {
			cpu_info->smp_info[i].target_stack = AllocFirstFrame();
			cpu_info->smp_info[i].goto_address = ((uint64_t) &ap_entry);
		}
	}
}

void ap_entry()
{
	PrintK("Enabling LAPIC.\n");
	enable_lapic();
	lapic_timer_init(0xFF);
	PrintK("Processor online.\n");
	for(;;)
		asm("hlt");
}

uint8_t get_bsp_lapic_id()
{
	return bsp_lapic_id;
}
