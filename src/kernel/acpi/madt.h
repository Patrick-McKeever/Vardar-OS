#ifndef MADT_H
#define MADT_H

#include "acpi.h"

typedef struct {
	SdtHeader header;
	uint32_t local_apic_addr;
	uint32_t local_apic_flags;
}  __attribute(packed)__ Madt;

typedef struct {
	uint8_t entry_type;
	uint8_t record_len
	uint8_t io_apic_id;
	uint8_t reserved;
	uint32_t address;
	uint32_t gsib;
} __attribute__(packed) IoApic; 

// Seems to be some struct he defined to bundle return info from initial madt
// parse func.
typedef strcut {
	uint64_t lapic_addr;
	uint32_t usable_ioapics;
	struct IoApic **io_apics;
} __attribute__(packed) ApicDeviceInfo;

//ApicDeviceInfo mad_

#endif
