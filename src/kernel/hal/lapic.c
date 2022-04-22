#include "hal/lapic.h"
#include "acpi/madt.h"
#include "utils/printf.h"

static uintptr_t LAPIC_BASE;

void
lapic_write(lapic_reg_t lapic_reg, uint32_t val)
{
	*((volatile uint32_t*) (LAPIC_BASE + lapic_reg)) = val;
}

uint32_t
lapic_read(lapic_reg_t lapic_reg)
{
	return *((uint32_t*) (LAPIC_BASE + lapic_reg));
}

void 
enable_lapic()
{
	// Sth to note is that this uses the hardcoded LAPIC MMIO mapping.
	// A more generic implementation would accomodate remappings.
	smp_info_t smp_info = get_smp_info();
	LAPIC_BASE = smp_info.lapic_addr;
	
	// No NMIs.
	NmiRecord *nmi;
	for(int i = 0; (nmi = get_nmi_record(i)); ++i) {
		enable_nmi(nmi);
	}

	// Bit 8 of spurious interrupt vector is "enable" bit for a given LAPIC.
	// To enable the lapic, we set bit 8 (i.e. the 9th overall bit) to 1.
	// (Should we or by 0x1ff instead?)
	//uint32_t current_val = lapic_read(LAPIC_SPURIOUS_INT_REG);
	uint32_t current_val = lapic_read(LAPIC_SPURIOUS_INT_REG);
	uint32_t enabled = current_val | (1 << 8);
	lapic_write(LAPIC_SPURIOUS_INT_REG, enabled);
	PrintK("Completed\n");
}

void
disable_lapic()
{
	smp_info_t smp_info = get_smp_info();
	LAPIC_BASE = (volatile uint32_t*) smp_info.lapic_addr;

	uint32_t current_val = lapic_read(LAPIC_SPURIOUS_INT_REG);
	uint32_t enabled = current_val | (0 << 8);
	lapic_write(LAPIC_SPURIOUS_INT_REG, enabled);
}

uint8_t
get_lapic_id() 
{
	// Bits 24-31 of spurious int register.
	uint32_t id_register 	= 	lapic_read(LAPIC_SPURIOUS_INT_REG);
	uint8_t id 				=	(id_register >> 24);
	return id;
}

uint8_t
get_lapic_version()
{
	// First 8 bits of version reg.
	uint8_t version 		= 	(uint8_t) lapic_read(LAPIC_VERSION_REG);
	return version;
}

void
enable_nmi(NmiRecord *nmi_record)
{
	// Make a handler for this.
	uint8_t lapic_id 	=	nmi_record->acpi_processor_id;

	if(get_lapic_id() != lapic_id && lapic_id != ALL_LAPICS) {
		return;
	}

	lvt_entry_t nmi;
	nmi.vector			=	NMI_INT_VECTOR;
	nmi.mask			=	0;
	nmi.delivery_mode 	= 	LVT_NMI;
	nmi.pin_polarity 	= 	nmi_record->flags & 2 ? ACTIVE_HIGH : 
													ACTIVE_LOW;
	nmi.trigger_mode 	= 	nmi_record->flags & 8 ? LEVEL_SENSITIVE : 
													EDGE_SENSITIVE;
	
	uint16_t lint_reg 	= 	nmi_record->lint ? LAPIC_LINT0_REG : 
											   LAPIC_LINT1_REG;
	uint32_t lvt_entry	=	*((uint32_t*) &nmi);
	lapic_write(lint_reg, lvt_entry);
}

bool 
eoi_is_broadcast()
{
	return !(lapic_read(LAPIC_SPURIOUS_INT_REG) & (1 << 12));
}

void 
set_eoi_broadcast(bool enabled)
{
	if(eoi_is_broadcast() == enabled)
		return;
	uint32_t spurious_reg	= lapic_read(LAPIC_SPURIOUS_INT_REG);
	uint32_t new_reg_val	= spurious_reg | ((!enabled) << 12);
	lapic_write(LAPIC_SPURIOUS_INT_REG, new_reg_val);
}

void
send_ipi(ipi_t *ipi)
{
	uint32_t lower_dword = *((uint32_t *) &ipi);
	uint32_t upper_dword = *(((uint32_t *) &ipi) + 1);
	lapic_write(LAPIC_ICR1_REG, upper_dword);
	lapic_write(LAPIC_ICR0_REG, lower_dword);
}

//void
//apic_start_timer()
//{
//	lapic_write(LAPIC_DIVIDE_CONFIG_REG, 0x3);
//	lapic_write(LAPIC_INIT_COUNT_REG, 0xFFFFFFFF);
//	lapic_write()
//}
