#ifndef MADT_H
#define MADT_H

#include "sdt.h"
#include "memory_management/physical_memory_manager.h"

// SDT is 0x24 bytes, local APIC addr is 4 bytes, flags are 4 bytes, so actual
// records begin at offset 0x2C of MADT.
#define MADT_RECORDS_OFFSET				0x2C

// Entry types in MADT. (First byte of MADT record header).
#define	PROCESSOR_LOCAL_APIC			0x00
#define IO_APIC							0x01
#define INTERRUPT_SOURCE_OVERRIDE		0x02
#define NON_MASKABLE_INTERRUPT_SOURCE	0x03
#define LOCAL_NON_MASKABLE_INTERRUPTS	0x04
#define LOCAL_APIC_ADDRESS_OVERRIDE		0x05
#define PROCESSOR_LOCAL_X2_APIC			0x09
#define DEFAULT_LAPIC_ADDR				0xFEE00000

typedef struct {
	SdtHeader header;
	uint32_t local_apic_addr;
	uint32_t local_apic_flags;
}  __attribute__((packed)) Madt;

typedef struct {
	uint8_t record_type;
	uint8_t record_len;
} __attribute__((packed)) MadtRecordHeader;

typedef struct {
	MadtRecordHeader header;
	uint8_t acpi_processor_id;
	uint8_t apic_id;
	uint32_t flags;
} __attribute__((packed)) ProcessorLocalApic;

typedef struct {
	MadtRecordHeader header;
	uint8_t reserved;
	uint32_t io_apic_addr;
	// Global system interrupt base (GSIB).
	uint32_t gsib;
} __attribute__((packed)) IoApicRecord;

typedef struct {
	MadtRecordHeader header;
	uint8_t bus_source;
	uint8_t irq_source;
	uint32_t gsi;
	uint16_t flags;
} __attribute__((packed)) IntSourceOverrideRecord;

typedef struct {
	MadtRecordHeader header;
	uint8_t nmi_source;
	uint8_t reserved;
	uint16_t flags;
	uint32_t global_system_interrupt;
} __attribute__((packed)) NmiSourceRecord;

typedef struct {
	MadtRecordHeader header;
	uint8_t acpi_processor_id;
	uint16_t flags;
	bool lint;
} __attribute__((packed)) NmiRecord;

// With this, ACPI gives the physical 64-bit addr of the APIC, since the
// previous addr had been in 32 bits. There will be at most one of these
// records in the MADT.
typedef struct {
	MadtRecordHeader header;
	uint16_t reserved;
	uint64_t local_apic_addr;
} __attribute__((packed)) LApicAddrOverrideRecord;

typedef struct {
	MadtRecordHeader header;
	uint16_t reserved;
	uint32_t local_x2_apic_id;
	uint32_t flags;
	uint32_t acpi_id;
} __attribute__((packed)) LocalX2ApicRecord;

typedef struct {
	size_t num_io_apics;
	IoApicRecord *io_apics;
	size_t num_lapics;
	ProcessorLocalApic *lapics;
	size_t num_isos;
	IntSourceOverrideRecord *isos;	
	uintptr_t lapic_addr;
} smp_info_t;

void ParseMadt();
IoApicRecord *get_io_apic(uint8_t idx);
int find_apic_from_gsi(uint32_t gsib);
ProcessorLocalApic *get_lapic(uint8_t idx);
InterruptSourceOverrideRecord *gsi_get_iso(uint32_t gsi);
InterruptSourceOverrideRecord *get_iso_from_irq(uint32_t irq);
uint32_t gsi_from_irq(uint8_t irq);

#endif
