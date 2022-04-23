#ifndef IO_APIC_H
#define IO_APIC_H

#include "acpi/madt.h"

// APIC IDs are 4 bits, so 2^4 possible APICs. I believe this can go higher
// with X2APIC, so revise if you ever support that.
#define MAX_IOAPICS					16
#define IOAPIC_ID_SHIFT				24
#define IOAPIC_ID_MASK				0b1111 	<< IOAPIC_ID_SHIFT
#define IOAPIC_VERSION_MASK			0xFF
#define IOAPIC_MAX_RED_SHIFT		16
#define IOAPIC_MAX_RED_MASK			0xFF 	<< IOAPIC_MAX_RED_SHIFT
#define MAX_ISA_IRQ					16

typedef enum {
	IOAPIC_ID		=	0,
	IOAPIC_VER		=	1,
	IOAPIC_ARB		=	2,
	IOREDTBL		=	3,
} ioapic_register_t;

/** Redirection table entry (inside I/O APIC). **/                              
typedef struct {
	uint8_t  vector;                                                        
	unsigned delivery_mode          : 3;
	unsigned destination_mode       : 1;
	unsigned delivery_status        : 1;
	unsigned polarity               : 1;
	unsigned remote_irr_pending     : 1;
	unsigned trigger_mode           : 1;
	unsigned mask                   : 1;
	uint64_t reserved               : 39;
	uint8_t  destination;               
} __attribute__((packed)) ioredtbl_t;

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

/** ICR MACROS AND STRUCT. ****************************************************/

/** ICR Delivery modes.
 * Some overlap between this and LVT delivery modes.
 * Difference between INIT_DELIVERY and INIT_LEVEL_DEASSERT is that the latter
 * requires that level flag be set to 0.
**/
typedef enum {
	ICR_FIXED					=	0b000,
	ICR_LOW						= 	0b001,
	ICR_SMI						=	0b010,
	ICR_NMI						= 	0b100,
	ICR_INIT					= 	0b101,
	ICR_STARTUP					= 	0b100
} icr_delivery_t;

typedef struct {                                                                
    ioredtbl_t  attrs;                                                          
    uint32_t    gsi;                                                            
    uint8_t     irq;                                                            
    uint8_t     ioapic_ind;                                                     
} int_attr_t; 

typedef struct {
	bool present;
	volatile uint32_t *ioregsel;
	volatile uint32_t *iowin;
	uint32_t min_gsi;
	uint8_t num_pins;
	uint8_t has_eoi;
	uint8_t apic_id;
} __attribute__((packed)) ioapic_t;

void
register_ioapic(IoApicRecord ioapic_record);

bool
ioapic_route_irq_to_bsp(uint8_t irq, uint8_t vector, bool masked);

bool
ioapic_route_irq(uint8_t irq, uint8_t rec_lapic_id, uint8_t vector, bool masked);

uint8_t
vector_from_gsi(uint32_t gsi);

bool
ioapic_set_gsi_mask(uint32_t gsi, bool masked);

bool
ioapic_set_gsi_vector(uint32_t gsi, uint8_t vector);

bool
set_gsi_trigger_mode(uint32_t gsi, trigger_mode_t trigger_mode);

bool 
set_gsi_polarity(uint32_t gsi, pin_polarity_t polarity);

ioapic_t*
ioapic_from_gsi(uint32_t gsi);

void
end_of_interrupt(bool broadcast, uint8_t vector);

void
mask_irq(uint8_t irq);

void
unmask_irq(uint8_t irq);

#endif
