#ifndef LAPIC_H
#define LAPIC_H

#include <stdint.h>
#include "io_apic.h"

/** OFFSETS OF IO APIC REGISTERS. *********************************************/
#define IO_APIC_IRQ_BASE				0x10
#define ALL_LAPICS						0xFF

#define UNSET							0
#define NMI_INT_VECTOR					0xFF

/** OFFSETS OF LVT REGISTERS. *************************************************/
typedef enum {
	LAPIC_ID_REG					=	0x020,
	LAPIC_VERSION_REG				=	0x030,
	LAPIC_EOI_REG					=	0x0B0,
	LAPIC_SPURIOUS_INT_REG			=	0x0F0,
	LAPIC_ICR0_REG					=	0x300,
	LAPIC_ICR1_REG					=	0x310,
	LAPIC_TIMER_REG					=	0x320,
	LAPIC_THERMAL_REG				=	0x330,
	LAPIC_PERFORMANCE_REG			=	0x340,
	LAPIC_LINT0_REG					=	0x350,
	LAPIC_LINT1_REG					=	0x360,
	LAPIC_ERROR_REG					=	0x370,
	LAPIC_INIT_COUNT_REG			=	0x380,
	LAPIC_CURRENT_COUNT_REG			=	0x390,
	LAPIC_DIVIDE_CONFIG_REG			=	0x3E0
} lapic_reg_t;


/** LVT MACROS AND STRUCT. ****************************************************/

/** Delivery modes. 
 * FIXED_INT is default, and specifies that the interrupt in the vector ought
 * to be placed in the Interrupt Request Register (IRR) queue and delivered to
 * core. Other types (NMI, SMI, INIT, EXTINT) are immediately forwarded to core,
 * unless masked.
 **/
typedef enum {
	LVT_FIXED					=		0b000,
	LVT_SMI						=		0b001,
	LVT_NMI						=		0b100,
	LVT_INIT					=		0b101,
	LVT_EXTERNAL				=		0b111	
} lvt_delivery_t;

/** Timer modes. 
 * In one-shot/periodic mode, timer decrements a counter value from the initial
 * count register and generates a local interrupt when it reaches 0. In periodic
 * mode, this value resets once it reaches 0 and begins decrementing again.
 * In TSC deadline mode, the timer mode is determined by another register.
**/
typedef enum {
	ONE_SHOT					=		0b00,
	PERIODIC					=		0b01,
	TSC_DEADLINE				=		0b10
} timer_mode_t;

/** Local Vector Table (LVT) entry. **/
typedef union {
	struct {
		uint8_t  vector;
		unsigned delivery_mode 				: 3;
		unsigned reserved0					: 1;
		unsigned delivery_status			: 1;
		unsigned pin_polarity 				: 1;
		unsigned remote_irr_flag 			: 1;
		unsigned trigger_mode 				: 1;
		unsigned mask						: 1;
		unsigned timer_mode 				: 2;
		unsigned reserved1 					: 12;
	} __attribute__((packed));

	uint32_t dword;
} lvt_entry_t;

typedef union {
	struct {
		uint8_t vector;
		unsigned delivery_mode				: 3;
		unsigned delivery_status			: 1;
		unsigned reserved0					: 1;
		unsigned level						: 1;
		unsigned trigger_mode				: 1;
		unsigned reserved1					: 2;
		unsigned destination_shorthand		: 2;
		uint64_t reserved2					: 36;
		uint8_t destination_field;
	};

	struct {
		uint32_t lower_dword;
		uint32_t upper_dword;
	};
} __attribute__((packed)) ipi_t;


void
lapic_write(lapic_reg_t lapic_reg, uint32_t val);

uint32_t
lapic_read(lapic_reg_t lapic_reg);

void 
enable_lapic();

void
disable_lapic();

uint8_t
get_lapic_id();

uint8_t
get_lapic_version();

void
enable_nmi(NmiRecord *nmi_record);

bool 
eoi_is_broadcast();

void 
set_eoi_broadcast(bool enabled);

void
send_ipi(ipi_t *ipi);

void
lapic_timer_init(uint8_t vector);

#endif
