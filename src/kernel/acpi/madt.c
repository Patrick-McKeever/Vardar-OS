// TODO: Restructure use of global statics (ParseMadt probably shouldn't return
// anything and should just write to static globals).

#include "madt.h"
#include "acpi.h"
#include "memory_management/physical_memory_manager.h"
#include "utils/printf.h"

static Madt *MADT;

static smp_info_t SMP_INFO = {
	.num_io_apics 	= 	0,
	.io_apics		=	AllocFirstFrame(),
	.num_lapics		=	0,
	.lapics			=	AllocFirstFrame(),
	.num_isos		=	0,
	.isos			=	AllocFirstFrame(),
	.lapic_addr		=	DEFAULT_LAPIC_ADDR
};

void ParseMadt()
{
	MADT = (Madt*) FindTable("APIC");
	int madt_len = MADT->header.length;
	
	// MADT consists of:
	// 	- SDT header (0x24 bytes);
	// 	- 4-byte int giving physical addr of local APIC;
	// 	- 4-byte flag field (mostly not of interest to us);
	// 	- a series of records, each consisting of two parts (offset 0x2C):
	// 		- 16-bit record header giving record type (byte 1) and len (byte 2);
	// 		- a variable length record body, whose length and form are
	// 		  determined by value in record header.
	uint8_t *madt_record_base = ((uint8_t*) MADT + MADT_RECORDS_OFFSET);
	uint8_t *record, record_type, record_len;
	for(int i = 0; (MADT_RECORDS_OFFSET + i) < madt_len; i += record_len) {
		record 		= (madt_record_base + i);
		record_type = record[0];
		record_len 	= record[1];
		
		switch(record_type) {
		case PROCESSOR_LOCAL_APIC: {
			SMP_INFO.lapics[SMP_INFO.num_lapics++] = 
				*((ProcessorLocalApic *) record);
			PrintK("Detected LAPIC w/ id %d and processor id %d\n", 
					rec.apic_id, rec.acpi_processor_id);
		} break;

		case IO_APIC: {
			SMP_INFO.io_apics[SMP_INFO.num_apics++] = *((IoApicRecord*) record);
			PrintK("Detected IO APIC w/ GSI %d\n", 
					SMP_INFO.io_apics[SMP_INFO.num_apics].gsib);
		} break;
		
		case INTERRUPT_SOURCE_OVERRIDE: {
			SMP_INFO.isos[SMP_INFO.num_isos++] = 
				*((IntSourceOverrideRecord*) record);
			PrintK("Detected ISO w/ GSI %d, IRQ %d\n", 
					SMP_INFO.isos[SMP_INFO.num_isos].gsi,
					SMP_INFO.isos[SMP_INFO.num_isos].irq_source);
		} break;

		case LOCAL_APIC_ADDRESS_OVERRIDE: {
			LApicAddrOverrideRecord addr = *((LApicAddrOverrideRecord*) record);
			SMP_INFO.lapic_addr = addr.local_apic_addr;
			PrintK("Detected new LAPIC addr of %d\n", SMP_INFO.lapic_addr);
		} break;
		}	
	}	
}

IoApicRecord *get_io_apic(uint8_t idx)
{
	if(idx >= SMP_INFO.num_io_apics)
		return NULL;
	return &SMP_INFO.io_apics[idx];
}


ProcessorLocalApic *get_lapic(uint8_t idx)
{
	if(idx >= SMP_INFO.num_lapics)
		return NULL;
	return &SMP_INFO.lapics[idx];
}

int find_apic_from_gsi(uint32_t gsi)
{
	// We must find the APIC ind with the largest GSIB that is still less than
	// the given GSI. apic_ind tracks the index which currently satisfies this,
	// and max_gsib tracks the maximum APIC GSIB which is still less than the
	// given GSI.
	int apic_ind = -1, max_gsib = 0;
	for(int i = 0; i < SMP_INFO.num_apics; ++i) {
		uint32_t apic_gsib = SMP_INFO.io_apics[i].gsib;
		if(gsi >= apic_gsib && apic_gsib >= max_gsib) {
			apic_ind = i;
			max_gsib = apic_gsib;
		}
	}

	return apic_ind;
}

InterruptSourceOverrideRecord *get_iso_from_irq(uint8_t irq)
{
	int gsi = get_gsi_from_irq(irq);
	int apic_ind = find_apic_from_gsi(gsi);
	if(ind == -1)
		return NULL;
	return &SMP_INFO.isos[apic_ind];
}



uint32_t gsi_from_irq(uint8_t irq)
{
	for(int i = 0; i < SMP_INFO.num_isos; ++i) {
		if(SMP_INFO.isos[i].irq_source == irq) {
			return SMP_INFO.isos[i].gsi;
		}
	}
	// If no interrupt source override exists, then IRQ is identity-mapped.
	return (uint32_t) irq;
}
