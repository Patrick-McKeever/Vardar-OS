#include "madt.h"
#include "memory_management/physical_memory_manager.h"

static Madt *MADT;

IoApicList ParseMadt()
{
	MADT = (Madt*) FindTable("APIC");
	IoApicList apic_list;
	apic_list.io_apics = AllocFirstFrame();
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
		case IO_APIC: {	
			apic_list.io_apics[apic_list.num_apics++] = *((IoApicRecord*) record);
		} break;

		case LOCAL_APIC_ADDRESS_OVERRIDE: {
			LApicAddrOverrideRecord addr = *((LApicAddrOverrideRecord*) record);
			apic_list.lapic_addr = addr.local_apic_addr;
		} break;
		}	
	}

	return apic_list;
}
