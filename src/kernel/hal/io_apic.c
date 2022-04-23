#include "io_apic.h"
#include "lapic.h"
#include "utils/printf.h"
#include "cpu_init.h"

static ioapic_t IOAPICS[MAX_IOAPICS];
static int_attr_t VECTOR_ATTRS[256];

static uint32_t
ioapic_read(ioapic_t *ioapic, uint8_t offset);

static void
ioapic_write(ioapic_t *ioapic, uint8_t offset, uint32_t val);

static void
ioapic_write_entry(ioapic_t *ioapic, uint32_t gsi, ioredtbl_t entry);

static ioredtbl_t
ioapic_read_entry(ioapic_t *ioapic, uint32_t gsi);

static inline pin_polarity_t
get_pin_polarity(uint16_t flags);

static inline trigger_mode_t
get_trigger_mode(uint16_t flags);

static void
initialize_ioapic_ints(ioapic_t *ioapic);

void
register_ioapic(IoApicRecord ioapic_record)
{
	ioapic_t ioapic = {
		.ioregsel 	= (volatile uint32_t *) ((uintptr_t) ioapic_record.io_apic_addr),
		.iowin		= (volatile uint32_t *) ((uintptr_t) ioapic_record.io_apic_addr + 0x10),
		.min_gsi	= ioapic_record.gsib,
		.present	= true
	};
	
	uint32_t ioapic_id_reg 	= ioapic_read(&ioapic, IOAPIC_ID);
	ioapic.apic_id			= ((ioapic_id_reg & (IOAPIC_ID_MASK)) >> 
								IOAPIC_ID_SHIFT);
	
	uint32_t ioapic_ver_reg = ioapic_read(&ioapic, IOAPIC_VER);
	ioapic.num_pins			= ((ioapic_ver_reg & (IOAPIC_MAX_RED_MASK)) >>
								IOAPIC_MAX_RED_SHIFT) + 1;
	ioapic.has_eoi			=  (ioapic_ver_reg & (IOAPIC_VERSION_MASK)) >= 0x20;	

	IOAPICS[ioapic.apic_id] = ioapic;
	initialize_ioapic_ints(&ioapic);
}

bool
ioapic_route_irq_to_bsp(uint8_t irq, uint8_t vector, bool masked)
{
	return ioapic_route_irq(irq, get_bsp_lapic_id(), vector, masked);
}

bool
ioapic_route_irq(uint8_t irq, uint8_t rec_lapic_id, uint8_t vector, bool masked)
{
	PrintK("Routing irq 0x%h to LAPIC 0x%h w/ vector 0x%h\n", 
			irq, rec_lapic_id, vector);
	uint32_t gsi			=	gsi_from_irq(irq);
	ioapic_t *ioapic		=	ioapic_from_gsi(gsi);
	if(!ioapic)
		return false;

	ioredtbl_t entry 		=	ioapic_read_entry(ioapic, gsi);
	uint32_t lower = *((uint32_t*) &entry);
	uint32_t upper = *(((uint32_t*) &entry)+1);
	entry.vector			=	vector;
	entry.polarity			=	ACTIVE_HIGH;
	entry.delivery_mode		= 	ICR_FIXED;
	entry.trigger_mode		=	EDGE_SENSITIVE;
	entry.destination		=	rec_lapic_id;
	entry.mask				=	masked;

	lower = *((uint32_t*) &entry);
	upper = *(((uint32_t*) &entry)+1);
	PrintK("Lower dword: 0x%h\n", lower);
	PrintK("Upper dword: 0x%h\n", upper);

	ioapic_write_entry(ioapic, gsi, entry);
	return true;
}

bool
ioapic_set_smi_gsi(uint32_t gsi)
{
	// I doubt this will ever be used, but I'd rather write it now while the
	// info is fresh in my mind than go back and implement it months later to
	// accomodate some edge case.
	// SMI is the interrupt which takes processor into ring 2 (system management
	// mode).
	ioapic_t *ioapic 		= 	ioapic_from_gsi(gsi);
	if(!ioapic)
		return false;
	
	ioredtbl_t entry;
	
	// Per MP spec, vector of SMI must be 0 for SMI.
	entry.vector 			= 	0;
	entry.delivery_mode 	= 	ICR_SMI;
	entry.polarity 			= 	ACTIVE_HIGH;
	entry.trigger_mode 		= 	EDGE_SENSITIVE;
	entry.mask				= 	0;


	ioapic_write_entry(ioapic, gsi, entry);
	return true;
}

uint8_t
vector_from_gsi(uint32_t gsi)
{
	ioapic_t *ioapic = ioapic_from_gsi(gsi);
	return ioapic_read_entry(ioapic, gsi).vector;
}

bool
int_attrs_from_vector(uint8_t vector, ioredtbl_t *entry_to_fill)
{
	uint32_t gsi		=	VECTOR_ATTRS[vector].gsi;
	ioapic_t *ioapic 	= 	ioapic_from_gsi(gsi);
	if(!ioapic || !entry_to_fill)
		return false;

	*(entry_to_fill) 	= 	ioapic_read_entry(ioapic, gsi);
	return true;
}

bool
ioapic_set_gsi_mask(uint32_t gsi, bool masked)
{
	ioapic_t *ioapic = ioapic_from_gsi(gsi);
	if(!ioapic)
		return false;

	ioredtbl_t current_entry = ioapic_read_entry(ioapic, gsi);
	if(current_entry.mask != masked) {
		current_entry.mask = masked;
		ioapic_write_entry(ioapic, gsi, current_entry);
	}

	return true;
}

bool
ioapic_set_gsi_vector(uint32_t gsi, uint8_t vector)
{
	ioapic_t *ioapic = ioapic_from_gsi(gsi);
	bool valid_vector = (vector > 15) && (vector < 256);

	if(!ioapic || !valid_vector)
		return false;

	ioredtbl_t current_entry = ioapic_read_entry(ioapic, gsi);
	if(current_entry.vector != vector) {
		current_entry.vector = vector;
		ioapic_write_entry(ioapic, gsi, current_entry);
		VECTOR_ATTRS[vector].gsi = gsi;
	}

	return true;
}

bool
ioapic_set_gsi_trigger_mode(uint32_t gsi, trigger_mode_t trigger_mode)
{
	// EXTINTs are always level-sensitive.
	if(gsi == 0 && trigger_mode != LEVEL_SENSITIVE)
		return false;

	ioapic_t *ioapic = ioapic_from_gsi(gsi);
	if(!ioapic)
		return false;

	ioredtbl_t current_entry = ioapic_read_entry(ioapic, gsi);
	
	// NMIs/SMIs are always edge-sensitive.
	bool nmi_or_smi = current_entry.delivery_mode == ICR_NMI ||
					  current_entry.delivery_mode == ICR_SMI;
	if(nmi_or_smi && trigger_mode != EDGE_SENSITIVE)
		return false;

	if(current_entry.trigger_mode != trigger_mode) {
		current_entry.trigger_mode = trigger_mode;
		ioapic_write_entry(ioapic, gsi, current_entry);
	}

	return true;
}

bool 
ioapic_set_gsi_polarity(uint32_t gsi, pin_polarity_t polarity)
{
	ioapic_t *ioapic = ioapic_from_gsi(gsi);
	if(!ioapic)
		return false;
	
	ioredtbl_t current_entry = ioapic_read_entry(ioapic, gsi);
	
	// SMIs must always be active high.
	if(current_entry.delivery_mode == ICR_SMI && polarity != ACTIVE_HIGH)
		return false;

	if(current_entry.polarity != polarity) {
		current_entry.polarity = polarity;
		ioapic_write_entry(ioapic, gsi, current_entry);
	}

	return true;
}

bool
ioapic_reroute_gsi(uint32_t gsi, uint8_t lapic_id)
{
	ioapic_t *ioapic = ioapic_from_gsi(gsi);
	if(!ioapic)
		return false;

	ioredtbl_t current_entry = ioapic_read_entry(ioapic, gsi);
	if(current_entry.destination != lapic_id) {
		current_entry.destination = lapic_id;
		ioapic_write_entry(ioapic, gsi, current_entry);
	}

	return true;
}

static uint32_t
ioapic_read(ioapic_t *ioapic, uint8_t offset)
{
	*(ioapic->ioregsel) = offset;
	return *(ioapic->iowin);
}

static void
ioapic_write(ioapic_t *ioapic, uint8_t offset, uint32_t val)
{
	*(ioapic->ioregsel) = offset;
	*(ioapic->iowin) = val;
}

static void
ioapic_write_entry(ioapic_t *ioapic, uint32_t gsi, ioredtbl_t entry)
{
	uint32_t pin 			= 	gsi - ioapic->min_gsi;
	ioapic_write(ioapic, 0x10 + pin * 2, entry.lower_dword);
	ioapic_write(ioapic, 0x11 + pin * 2, entry.upper_dword);
	//uint32_t *entry_words	= 	((uint32_t *) &entry);
	//ioapic_write(ioapic, 0x10 + pin * 2, entry_words[0]);
	//ioapic_write(ioapic, 0x11 + pin * 2, entry_words[1]);
}

static ioredtbl_t
ioapic_read_entry(ioapic_t *ioapic, uint32_t gsi)
{
	uint64_t entry_raw;
	uint32_t pin 	 = 	gsi - ioapic->min_gsi;
	entry_raw 		 = ioapic_read(ioapic, 0x10 + pin * 2);
	entry_raw 		|= (ioapic_read(ioapic, 0x11 + pin * 2) << 31);
	return *((ioredtbl_t*) &entry_raw);
}

static inline pin_polarity_t
get_pin_polarity(uint16_t flags)
{
	if(flags & 2)
		return ACTIVE_HIGH;
	return ACTIVE_LOW;
}

static inline trigger_mode_t
get_trigger_mode(uint16_t flags)
{
	if(flags & 8)
		return LEVEL_SENSITIVE;
	return EDGE_SENSITIVE;
}

ioapic_t*
ioapic_from_gsi(uint32_t gsi)
{
	for(int i = 0; i < MAX_IOAPICS; ++i) {
		if(! IOAPICS[i].present)
			continue;

		uint32_t min_gsi = IOAPICS[i].min_gsi,
				 max_gsi = IOAPICS[i].min_gsi + IOAPICS[i].num_pins - 1;
		if(gsi >= min_gsi && gsi <= max_gsi)
			return &IOAPICS[i];
	}

	return NULL;
}

void
end_of_interrupt(bool broadcast, uint8_t vector)
{
    // To signal end of interrupt, write to lapic's EOI register. Literally any 
    // write will trigger EOI.
    lapic_write(LAPIC_EOI_REG, 0);
}

static void
initialize_ioapic_ints(ioapic_t *ioapic)
{
	for(int gsi = ioapic->min_gsi; gsi < ioapic->min_gsi + ioapic->num_pins; 
		++gsi) 
	{
		ioredtbl_t entry;
		NmiSourceRecord *nmi_info;
		IntSourceOverrideRecord *iso_info;
	
		// GSI 0: External interrupts from 8259A PIC. Not supported at present.
		if(gsi == 0) {
			continue;
		} 
		
		// GSIs 1-15: ISA IRQs. GSIs are identity mapped to IRQs unless an
		// interrupt source override exists.
		else if(gsi < MAX_ISA_IRQ) {
			PrintK("Writing GSI 0x%h to vector 0x%h\n", gsi, gsi + 0x20);
			entry.vector			= 	gsi + 0x20;
			entry.delivery_mode 	= 	ICR_FIXED;
			entry.polarity			= 	ACTIVE_HIGH;
			entry.trigger_mode		=	EDGE_SENSITIVE;
			entry.mask				=	1;
		} 
		
		// Some GSIs outside of [0,16) may be map to IRQs, if specified as such
		// in a MADT interrupt source override (ISO).
		else if((iso_info = gsi_get_iso(gsi))) {
			PrintK("Iso for GSI %d\n", gsi);
			entry.vector			=	iso_info->irq_source + 0x20;
			entry.delivery_mode 	=	ICR_FIXED;
			entry.polarity			=	get_pin_polarity(iso_info->flags);
			entry.trigger_mode		=	get_trigger_mode(iso_info->flags);
			entry.mask				=	1;
		}

		// Non-maskable interrupt source.
		else if((nmi_info = gsi_get_nmi_source(gsi))) {
			entry.vector			=	gsi + 0x20;
			entry.delivery_mode 	= 	ICR_NMI;
			entry.polarity			= 	get_pin_polarity(nmi_info->flags);
			entry.trigger_mode		=	get_trigger_mode(nmi_info->flags);
			entry.mask				=	0;
		}

		// GSIs [16, 256) (excepting ISOs/NMIs): PCI bus interrupts.
		else {
			entry.vector			= 	gsi + 0x20;
			entry.delivery_mode		=	ICR_FIXED;
			entry.polarity			= 	ACTIVE_HIGH;
			entry.trigger_mode		= 	LEVEL_SENSITIVE;
			entry.mask				=	1;
		}
		
		entry.destination_mode					=	IOAPIC_PHYSICAL;
		VECTOR_ATTRS[entry.vector].attrs 		= 	entry;
		VECTOR_ATTRS[entry.vector].gsi 			= 	gsi;
		VECTOR_ATTRS[entry.vector].irq 			= 	entry.vector - 0x20;
		VECTOR_ATTRS[entry.vector].ioapic_ind 	= 	ioapic->apic_id;

		entry.destination	=	get_bsp_lapic_id();
		ioapic_write_entry(ioapic, gsi, entry);
	}
}

void
mask_irq(uint8_t irq)
{
	uint32_t gsi = gsi_from_irq(irq);
	ioapic_set_gsi_mask(gsi, 1);
}

void
unmask_irq(uint8_t irq)
{
	uint32_t gsi = gsi_from_irq(irq);
	ioapic_set_gsi_mask(gsi, 0);
}

