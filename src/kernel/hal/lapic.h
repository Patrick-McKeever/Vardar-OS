#ifndef LAPIC_H
#define LAPIC_H

#include <stdint.h>

/** OFFSETS OF IO APIC REGISTERS. *********************************************/
#define IO_APIC_IRQ_BASE				0x10

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

#define UNSET							0


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

/** Interrupt pin polarity. Rarely ever used. **/
typedef enum {
	ACTIVE_HIGH					=		0,
	ACTIVE_LOW					=		1
} pin_polarity_t;

/** Remote IRR flag.
 * Read only, set to 1 when processor sends an EOI (i.e. finished processing
 * interrupt).
**/
typedef enum {
	EOI_PENDING					=		0,
	EOI_RECEIVED				=		1
} eoi_status_t;

/** Trigger modes. **/
typedef enum {
	EDGE_SENSITIVE				=		0,
	LEVEL_SENSITIVE				=		1
} trigger_mode_t;

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



#endif
