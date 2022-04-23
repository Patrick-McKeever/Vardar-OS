#include "hal/lapic.h"
#include "hal/pit.h"
#include "acpi/madt.h"
#include "utils/printf.h"
#include "utils/spin_lock.h"

// Base address of LAPIC MMIO patch.
static uintptr_t LAPIC_BASE;
// Entry n stores the frequency in HZ of the timer of theLAPIC with ID n.
static uint16_t LAPIC_TIMER_HZs[256];

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
	LAPIC_BASE = smp_info.lapic_addr;

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
	// You need to write upper dword first.
	lapic_write(LAPIC_ICR1_REG, ipi->upper_dword);
	lapic_write(LAPIC_ICR0_REG, ipi->lower_dword);
}

static uint32_t PIT_COUNT;
static spin_lock_t PIT_COUNT_LOCK;

static void
pit_increment(void)
{
	++PIT_COUNT;
}

void
lapic_timer_init(uint8_t vector)
{
	__asm__("cli");

	// Set LAPIC timer on long countdown, but don't interrupt when
	// done.
	lvt_entry_t timer_entry;
	timer_entry.dword			=	lapic_read(LAPIC_TIMER_REG);
	timer_entry.vector			=	vector;
	timer_entry.delivery_mode	=	LVT_FIXED;
	timer_entry.mask			=	1;
	timer_entry.timer_mode		=	ONE_SHOT;

	// Set LAPIC timer on long countdown from 0xFFFFFFFF, decrementing 
	// every 8 (2 ^ (0b010 + 1), as per divide config reg) ticks.
	lapic_write(LAPIC_TIMER_REG, timer_entry.dword);
	lapic_write(LAPIC_DIVIDE_CONFIG_REG, 0b010);
	lapic_write(LAPIC_INIT_COUNT_REG, 0xFFFFFFFF);
	
	wait(&PIT_COUNT_LOCK);

	PIT_COUNT					=	0;
	register_pit_handler(&pit_increment);
	
	// Between starting the countdown of each timer, some time has already
	// passed, so let's get the PIT and LAPIC current counts at (roughly)
	// the same time.
	uint32_t init_lapic_count	=	lapic_read(LAPIC_CURRENT_COUNT_REG);
	uint32_t pit_rate_hz 		=	1000;
	uint32_t total_pit_tics		=	100;

	// PIT now has a rate of (roughly) 1000hz.
	set_pit_periodic(pit_rate_hz);
	__asm__("sti");

	// Busy-wait until PIT counter reaches certain value.
	// W/ rate of 1000hz and 100 tics, this will take 1μs.
	while(PIT_COUNT < total_pit_tics)
	{
		PrintK("");
	}

	uint32_t final_lapic_count	=	lapic_read(LAPIC_CURRENT_COUNT_REG);
	uint32_t total_lapic_tics	=	(init_lapic_count - final_lapic_count) * 8;

	release(&PIT_COUNT_LOCK);
	
	uint8_t lapic_id			=	get_lapic_id();
	LAPIC_TIMER_HZs[lapic_id] 	=	(total_lapic_tics / total_pit_tics) * pit_rate_hz;
	PrintK("LAPIC timer for LAPIC #%d has frequency of %d hz.\n",
			lapic_id, LAPIC_TIMER_HZs[lapic_id]);
}

//void
//apic_start_timer()
//{
//	lapic_write(LAPIC_DIVIDE_CONFIG_REG, 0x3);
//	lapic_write(LAPIC_INIT_COUNT_REG, 0xFFFFFFFF);
//	lapic_write()
//}
