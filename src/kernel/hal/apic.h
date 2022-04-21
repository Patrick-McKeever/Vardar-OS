#ifndef APIC_H
#define APIC_H

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


/** ICR MACROS AND STRUCT. ****************************************************/

/** ICR Delivery modes.
 * Some overlap between this and LVT delivery modes.
 * Difference between INIT_DELIVERY and INIT_LEVEL_DEASSERT is that the latter
 * requires that level flag be set to 0.
**/
typedef enum {
	ICR_FIXED					=	0b000,
	ICR_LOW						= 	0b001,
	ICR_NMI						= 	0b100,
	ICR_INIT					= 	0b101,
	ICR_STARTUP					= 	0b100
} icr_delivery_t;

/** Destination modes for IPIs.
 * In physical, destination LAPIC is determined from destination field of ICR 
 * (set equal to target LAPIC ID) or from destination shorthand field.
 * In logical, destination is determined by an 8-bit Message Destination Addr
 * (MDA) in the destination field. Logical mode is rarely used.
**/
typedef enum {
	PHYSICAL				=	0,
	LOGICAL					=	1
} destination_mode_t;

/** IPI delivery status (read only). 
 * Indicates whether LAPIC has sent all previous IPIs.
**/
typedef enum {
	IDLE					=	0,
	SEND_PENDING			=	1
} ipi_status_t;

/** Level field. Exists exclusively to indicate init-level deasserts. **/
typedef enum {
	DEASSERT_LEVEL			=	0,
	NORMAL_LEVEL			=	1
} icr_level_t;

/** Trigger field. Exists exclusively to indicate init-level deasserts. **/
typedef enum {
	DEASSERT_TRIGGER		=	0,
	NORMAL_TRIGGER			=	1
} icr_trigger_t;

/** Destination shorthands. 
 * Provides simplified means of addressing multiple LAPICs.
**/
typedef enum {
	NO_SHORTHAND			=	0b00,
	ONLY_SELF				=	0b01,
	ALL_WITH_SELF			=	0b10,
	ALL_BUT_SELF			=	0b11
} dest_shorthand_t;

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
	unsigned reserved3			 		: 36;
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
} ioredtbl_t;

typedef struct {
	ioredtbl_t 	attrs;
	uint32_t 	gsi;
	uint8_t		irq;
	uint8_t		ioapic_ind;
} int_attr_t;

static 
static int_attr_t VECTOR_ATTRS[256];

static inline uintptr_t 
ioapic_base(uint8_t idx)
{
	IoApicRecord *apic = get_ioapic(idx);
	if(apic == NULL)
		return NULL;
	return ((uintptr_t) apic->ioapic_addr);
}

static inline uintptr_t 
irq_to_addr(uint8_t apic_idx, uint8_t irq) 
{
	// Each entry takes up 2 32 bit entries, beginning at an offset of 0x10 from
	// the IO APIC base.
	return ioapic_base(apic_idx) + IO_APIC_IRQ_BASE + (irq * 2);
}

static inline uint32_t 
ioapic_read(uint8_t idx, uint32_t reg_offset)
{
	uint32_t *apic_base = ioapic_base(idx);
	return *((volatile uint32_t*) (apic_base + reg_offset));
}

static inline void 
ioapic_write(uint8_t idx, uint32_t reg_offset, uint32_t val)
{
	uint32_t *apic_base = ioapic_base(idx);
	*((volatile uint32_t*) (apic_base + reg_offset)) = val;
}

static inline uint32_t 
lapic_read(uint32_t reg_offset) 
{
	smp_info_t smp_info = get_smp_info();
	return *((volatile uint32_t*) (ioapics.lapic_addr + reg_offset));
}

static inline void 
lapic_write(uint32_t reg_offset, uint32_t val)
{
	smp_info_t smp_info = get_smp_info();
	*((volatile uint32_t*) (smp_info.lapic_addr + reg_offset)) = val;
}

void 
register_nmi(uint8_t vector, uint8_t lint, bool pin, bool trigger)
{	
	lvt_entry_t current_entry = *((lvt_entry_t*) lapic_read(lint_offset));
	lvt_entry_t nmi = {
		.vector = vector,
		.delivery_mode = NON_MASKABLE_INT,
		.pin_polarity = pin,
		.trigger_mode = trigger
	};

	uint32_t lint_offset = lint ? LVT_LINT1_OFFSET : LVT_LINT2_OFFSET;
	uint32_t packed_nmi = *((uint32_t*) nmi);
	lapic_write(lint_offset, packed_nmi);
}

void 
enable_lapic()
{
	// Bit 8 of spurious interrupt vector is "enable" bit for a given LAPIC.
	// To enable the lapic, we set bit 8 (i.e. the 9th overall bit) to 1.
	uint32_t current_val = lapic_read(LVT_SPURIOUS_INT_R_OFFSET);
	uint32_t enabled = current_val | (1 << 8);
	lapic_write(LVT_SPURIOUS_INT_R_OFFSET, enabled);
}

void 
end_of_interrupt(bool broadcast, uint8_t vector)
{
	// We can disable EOI broadcasting by setting the 12th bit of the lapic's
	// spurious interrupt register.
	if(broadcast != eoi_is_broadcast()) {
		uint32_t spurious_reg 	= lapic_read(SPURIOUS_INT_R_OFFSET);
		uint32_t new_val		= spurious_reg | ((!broadcast) << 12);
		lapic_write(LVT_SPURIOUS_INT_R_OFFSET, new_val);
	}

	// To signal end of interrupt, write to lapic's EOI register. Literally any
	// write will trigger EOI.
	lapic_write(LVT_EOI_REGISTER, 0);
	
	int_attr_t int_attrs = VECTOR_ATTRS[vector];
	if(broadcast && int_attrs->attrs->trigger_mode == EDGE_SENSITIVE)
		return;
	
	ioapic_write(int_attrs->ioapic_ind, 0x40, vector);
	// TODO: Lower versions of APIC standard don't support per-IOAPIC EOIs.
	// There's a semi-hackish workaround which involves temporarily switching 
	// the gsi to edge-triggered, since edge-triggered gsis can't disable
	// broadcasts. Implement this at some point.
}

void set_redirect(uint8_t vector, uint8_t lapic_id, uint32_t gsi, 
				  trigger_mode_t t, pin_polarity_t p, bool masked)
{
	ioredtbl_t entry = {
		.vector				=	vector,
		.delivery_mode 		= 	ICR_FIXED,
		.destination_mode 	= 	PHYSICAL,
		.delivery_status	=	UNSET,
		.polarity			=	p,
		.remote_irr_pending	=	UNSET,
		.trigger_mode		=	t,
		.mask				= 	masked,
		.destination		=	lapic_id	
	};

	ioapic_write_entry(apic_ind, gsi, entry);
}

void 
ioapic_mask(int gsi) 
{
	
}

void 
ioapic_unmask(uint8_t gsi) {}



void 
ioapic_write_entry(int apic, int gsi, ioredtbl_t entry)
{
	// As soon as lower 32 bits is written to, the entry will be transmitted
	// to the IO APIC over the bus. Hence, we must write the upper 32 bits
	// first.
	volatile uint32_t *entry = (volatile uint32_t*) irq_to_addr(apic, gsi);
	*(entry + 1) = entry.lower_dword;
	*(entry) = entry.upper_dword;
}

ioredtbl_t
ioapic_read_entry(int apic, gsi)

int 
enable_irq_for_cpu(uint8_t irq, int cpu)
{
	uint8_t lapic_id = get_lapic(cpu).apic_id;
	
	IntSourceOverrideRecord *iso = get_iso_from_irq(irq);
	if(iso == NULL)
		return;
	
	int gsi = get_gsi_from_irq(irq);
	int apic_ind = find_apic_from_gsi(gsi);
	if(apic_ind == -1)
		return -1;	
	
	ioredtbl_t entry = {
		.vector				=	irq + 0x20,
		.mask				= 	0,
		.destination		=	lapic_id	
	};
	
	parse_flags(&entry, iso->flags);
	ioapic_write_entry(apic_ind, gsi, entry);
}

int
enable_irq_for_all_cpus(uint8_t irq)
{
	IntSourceOverrideRecord *iso = get_iso_from_irq(irq);
	if(iso == NULL)
		return;
	
	int gsi = get_gsi_from_irq(irq);
	int apic_ind = find_apic_from_gsi(gsi);
	if(apic_ind == -1)
		return -1;	
	
	ioredtbl_t entry = {
		.vector				=	irq + 0x20,
		.mask				= 	0,
		.destination		=	lapic_id	
	};
	
	parse_flags(&entry, iso->flags);
	ioapic_write_entry(apic_ind, gsi, entry);
	
	// It will be useful to quickly lookup information about interrupts by their
	// vectors, so we'll create a simple array for that purpose. This should
	// also be substantially quicker than looking up info from memory-mapped
	// IO (i.e. IOAPIC/LAPIC registers).
	VECTOR_ATTRS[entry.vector].attrs 		= entry;
	VECTOR_ATTRS[entry.vector].gsi	 		= gsi;
	VECTOR_ATTRS[entry.vector].irq	 		= irq;
	VECTOR_ATTRS[entry.vector].ioapic_ind	= find_apic_from_gsi(gsi);
}

void 
parse_flags(ioredtbl_t *entry, uint16_t flags)
{
	// Clear 9 bits of flags, beginning at bit 8.
	*entry &= ~(0x1FF << 8);
	// Now, add the given flags beginning at position 8.
	*entry |= (flags << 8);
}

void 
enable_legacy_irq(int cpu, uint8_t irq_num)

bool eoi_is_broadcast()
{
	return !(lapic_read(LVT_SPURIOUS_INT_REG) & (1 << 12));
}

void set_eoi_broadcast(bool enabled)
{
	if(eoi_is_broadcast() == enabled)
		return;
	
	uint32_t new_reg_val	= spurious_reg | ((!enabled) << 12);
	lapic_write(LVT_SPURIOUS_INT_REG, new_reg_val)
}
#endif
