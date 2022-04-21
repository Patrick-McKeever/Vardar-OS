// TODO: Restructure use of global statics (ParseMadt probably shouldn't return
// anything and should just write to static globals).

#include "madt.h"
#include "acpi.h"
#include "hal/io_apic.h"
#include "memory_management/physical_memory_manager.h"
#include "utils/printf.h"

static Madt *MADT;

static smp_info_t SMP_INFO = {
	.num_io_apics 		= 	0,
	.io_apics			=	NULL,
	.num_lapics			=	0,
	.lapics				=	NULL,
	.num_isos			=	0,
	.isos				=	NULL,
	.num_nmi_sources 	= 	0,
	.nmi_sources		=	NULL,
	.num_nmis			=	0,
	.nmis				=	NULL,
	.lapic_addr			=	DEFAULT_LAPIC_ADDR
};

void ParseMadt()
{
	MADT = (Madt*) FindTable("APIC");
	int madt_len = MADT->header.length;

	SMP_INFO.io_apics 		= 	AllocFirstFrame();
	SMP_INFO.lapics 		= 	AllocFirstFrame();
	SMP_INFO.isos 			= 	AllocFirstFrame();
	SMP_INFO.nmi_sources 	=	AllocFirstFrame();
	SMP_INFO.nmis			=	AllocFirstFrame();
	
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
			ProcessorLocalApic pla_record = *((ProcessorLocalApic*) record);
			SMP_INFO.lapics[SMP_INFO.num_lapics++] = pla_record;
			PrintK("Detected LAPIC w/ id %d and processor id %d\n", 
					pla_record.apic_id, pla_record.acpi_processor_id);
		} break;

		case IO_APIC: {
			IoApicRecord ioapic_record = *((IoApicRecord*) record);
			SMP_INFO.io_apics[SMP_INFO.num_io_apics++] = ioapic_record;
			PrintK("Detected IO APIC at addr 0x%h w/ GSI base 0x%h\n", 
					ioapic_record.io_apic_addr, ioapic_record.gsib);
			register_ioapic(ioapic_record);
		} break;
		
		case INTERRUPT_SOURCE_OVERRIDE: {
			IntSourceOverrideRecord iso_record = *((IntSourceOverrideRecord*) record);
			SMP_INFO.isos[SMP_INFO.num_isos++] = iso_record;
			PrintK("Detected ISO w/ GSI %d, IRQ %d\n", 
					iso_record.gsi, iso_record.irq_source);
		} break;

		case NON_MASKABLE_INTERRUPT_SOURCE: {
			NmiSourceRecord nmi_record = *((NmiSourceRecord*) record);
			SMP_INFO.nmi_sources[SMP_INFO.num_nmi_sources++] = nmi_record;
			PrintK("Detected NMI source w/ GSI %d, source %d\n",
					nmi_record.gsi, nmi_record.nmi_source);
		} break;

		case LOCAL_NON_MASKABLE_INTERRUPT: {
			NmiRecord nmi_record = *((NmiRecord*) record);
			SMP_INFO.nmis[SMP_INFO.num_nmis++] = nmi_record;
			PrintK("Detected NMI w/ LAPIC ID %d, LINT %d\n", 
					nmi_record.acpi_processor_id, nmi_record.lint);	
		} break;

		case LOCAL_APIC_ADDRESS_OVERRIDE: {
			LApicAddrOverrideRecord addr = *((LApicAddrOverrideRecord*) record);
			SMP_INFO.lapic_addr = addr.local_apic_addr;
			PrintK("Detected new LAPIC addr of %d\n", SMP_INFO.lapic_addr);
		} break;
		}	
	}	
}

smp_info_t get_smp_info()
{
	return SMP_INFO;
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
	for(int i = 0; i < SMP_INFO.num_io_apics; ++i) {
		uint32_t apic_gsib = SMP_INFO.io_apics[i].gsib;
		if(gsi >= apic_gsib && apic_gsib >= max_gsib) {
			apic_ind = i;
			max_gsib = apic_gsib;
		}
	}

	return apic_ind;
}

IntSourceOverrideRecord *get_iso_from_irq(uint8_t irq)
{
	uint32_t gsi = gsi_from_irq(irq);
	return gsi_get_iso(gsi);
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

NmiSourceRecord *gsi_get_nmi_source(uint32_t gsi)
{
	for(int i = 0; i < SMP_INFO.num_nmi_sources; ++i) {
		if(SMP_INFO.nmi_sources[i].gsi == gsi) {
			return &SMP_INFO.nmi_sources[i];
		}
	}
	return NULL;
}

NmiRecord *get_nmi_record(uint8_t idx)
{
	if(idx > SMP_INFO.num_nmis - 1)
		return NULL;
	return &SMP_INFO.nmis[idx];
}

IntSourceOverrideRecord *gsi_get_iso(uint32_t gsi)
{
	for(size_t i = 0; i < SMP_INFO.num_isos; ++i) {
		if(SMP_INFO.isos[i].gsi == gsi) {
			return &SMP_INFO.isos[i];
		}
	}
	return NULL;
}
