#ifndef APIC_H
#define APIC_H

#include <stdint.h>

/** OFFSETS OF LVT REGISTERS. *************************************************/
#define LVT_EOI_REGISTER				0x0B0
#define LVT_SPURIOUS_INT_R_OFFSET		0x0F0
#define LVT_ICR0_OFFSET					0x300
#define LVT_ICR1_OFFSET					0x310
#define LVT_TIMER_R_OFFSET				0x320
#define LVT_THERMAL_R_OFFSET			0x330
#define LVT_PERFORMANCE_R_OFFSET		0x340
#define LVT_LINT0_OFFSET				0x350
#define LVT_LINT1_OFFSET				0x360
#define LVT_ERROR_R_OFFSET				0x370
#define LVT_INIT_COUNT_R_OFFSET			0x380
#define LVT_CURRENT_COUNT_R_OFFSET		0x390
#define LVT_DIVIDE_CONFIG_R_OFFSET		0x3E0

/** LVT MACROS AND STRUCT. ****************************************************/

/** Delivery modes. 
 * FIXED_INT is default, and specifies that the interrupt in the vector ought
 * to be placed in the Interrupt Request Register (IRR) queue and delivered to
 * core. Other types (NMI, SMI, INIT, EXTINT) are immediately forwarded to core,
 * unless masked.
 **/
#define FIXED_INT						0b000
#define SYS_MAN_INT						0b001
#define NON_MASKABLE_INT				0b100
#define INIT							0b101
#define EXT_INT							0b111

/** Interrupt pin polarity. Rarely ever used. **/
#define ACTIVE_HIGH						0
#define ACTIVE_LOW						1

/** Remote IRR flag.
 * Read only, set to 1 when processor sends an EOI (i.e. finished processing
 * interrupt).
**/
#define EOI_PENDING						0
#define EOI_RECEIVED					1

/** Trigger modes. Rarely ever used. **/
#define EDGE_SENSITIVE					0
#define LEVEL_SENSITIVE					1

/** Interrupt masks. 
 * If a given vector is maked in local LVT, it will be ignored by LAPIC.
**/
#define UNMASKED						0
#define MASKED							1

/** Timer modes. 
 * In one-shot/periodic mode, timer decrements a counter value from the initial
 * count register and generates a local interrupt when it reaches 0. In periodic
 * mode, this value resets once it reaches 0 and begins decrementing again.
 * In TSC deadline mode, the timer mode is determined by another register.
**/
#define ONE_SHOT_TIMER					0b00
#define PERIODIC_TIMER					0b01
#define TSC_DEADLINE					0b10

/** Local Vector Table (LVT) entry. **/
typedef struct {
	uint8_t  vector;
	unsigned delivery_mode 				: 3;
	unsigned pin_polarity 				: 1;
	unsigned remote_irr_flag 			: 1;
	unsigned trigger_mode 				: 1;
	unsigned mask						: 1;
	unsigned timer_mode 				: 2;
	unsigned reserved 					: 15;
} lvt_entry_t;


/** ICR MACROS AND STRUCT. ****************************************************/

/** ICR Delivery modes.
 * Some overlap between this and LVT delivery modes.
 * Difference between INIT_DELIVERY and INIT_LEVEL_DEASSERT is that the latter
 * requires that level flag be set to 0.
**/
#define FIXED_DELIVERY					0b000
#define LOWER_PRIORITY_DELIVERY 		0b001
#define NMI_DELIVERY					0b100
#define INIT_DELIVERY					0b101 
#define INIT_LEVEL_DEASSERT				0b101
#define STARTUP_IPI						0b110

/** Destination modes for IPIs.
 * In physical, destination LAPIC is determined from destination field of ICR 
 * (set equal to target LAPIC ID) or from destination shorthand field.
 * In logical, destination is determined by an 8-bit Message Destination Addr
 * (MDA) in the destination field. Logical mode is rarely used.
**/
#define PHYSICAL_DEST					0
#define LOGICAL_DEST					1

/** IPI delivery status (read only). 
 * Indicates whether LAPIC has sent all previous IPIs.
**/
#define IDLE							0
#define SEND_PENDING					1

/** Level field. Exists exclusively to indicate init-level deasserts. **/
#define DEASSERT_LEVEL					0
#define NORMAL_LEVEL					1

/** Trigger field. Exists exclusively to indicate init-level deasserts. **/
#define DEASSERT_TRIGGER				0
#define NORMAL_TRIGGER					1

/** Destination shorthands. 
 * Provides simplified means of addressing multiple LAPICs.
**/
#define NO_SHORTHAND					0b00
#define ONLY_SELF						0b01
#define ALL_INCLUDING_SELF				0b10
#define ALL_EXCEPT_SELF					0b11

/** Instruction Control Register (ICR). **/
typedef struct {
	uint8_t  vector;
	unsigned delivery_mode			 	: 3;
	unsigned destination_mode		 	: 1;
	unsigned delivery_status			: 1;
	unsigned reserved0			 		: 1;
	unsigned level			 			: 1;
	unsigned trigger			 		: 1;
	unsigned reserved1			 		: 2;
	unsigned destination_shorthand		: 3;
	unsigned reserved3			 		: 36:
	uint8_t  destination;
} icr_t;


/** IO APIC MACROS AND STRUCT. ************************************************/

/** Redirection table entry (inside I/O APIC). **/
typedef union { 
	struct {
		uint8_t  vector;
		unsigned delivery_mode 			: 3;
		unsigned destination_mode 		: 1;
		unsigned delivery_status		: 1;
		unsigned polarity				: 1;
		unsigned remote_irr_pending		: 1;
		unsigned trigger_mode			: 1;
		unsigned mask					: 1;
		unsigned reserved				: 39;
		uint8_t	 destination;
	};

	struct {
		uint32_t lower_dword;
		uint32_t upper_dword;
	};
} redirection_table_entry_t;


int lapic_read(int reg_offset) 
{
	IoApicList io_apics = GetIoApics();
	return *((volatile uint32_t*) (io_apics.lapic_addr + reg_offset));
}

void lapic_write(int reg_offset, uint32_t val)
{
	IoApicList io_apics = GetIoApics();
	*((volatile uint32_t*) (io_apics.lapic_addr + reg_offset)) = val;
}

void register_nmi(uint8_t vector, uint8_t lint, bool pin, bool trigger)
{	
	struct lvt_entry_t current_entry = *((lvt_entry_t*) lapic_read(lint_offset));
	struct lvt_entry_t nmi = {
		.vector = vector,
		.delivery_mode = NON_MASKABLE_INT,
		.pin_polarity = pin,
		.trigger_mode = trigger
	};

	uint32_t lint_offset = lint ? LVT_LINT1_OFFSET : LVT_LINT2_OFFSET;
	uint32_t packed_nmi = *((uint32_t*) nmi);
	lapic_write(lint_offset, packed_nmi);
}

void enable_lapic()
{
	// Bit 8 of spurious interrupt vector is "enable" bit for a given LAPIC.
	// To enable the lapic, we set bit 8 (i.e. the 9th overall bit) to 1.
	uint32_t current_val = lapic_read(LVT_SPURIOUS_INT_R_OFFSET);
	uint32_t enabled = current_val | (1 << 8);
	lapic_write(LVT_SPURIOUS_INT_R_OFFSET, enabled);
}

void end_of_interrupt(bool broadcast)
{
	// We can disable EOI broadcasting by setting the 12th bit of the lapic's
	// spurious interrupt register.
	if(! broadcast) {
		uint32_t spurious_reg = lapic_read(SPURIOUS_INT_R_OFFSET);
		lapic_write(LVT_SPURIOUS_INT_R_OFFSET, spurious_reg | (1 << 12));
	}

	// To signal end of interrupt, write to lapic's EOI register. Literally any
	// write will trigger EOI.
	lapic_write(LVT_EOI_REGISTER, 0);
}

//void io_apic_set_redirect(uint8_t vector, uint32_t)

#endif
