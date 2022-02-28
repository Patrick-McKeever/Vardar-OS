#ifndef ACPI_H
#define ACPI_H

#include <stdint.h>
#include <stdbool.h>
#include "stivale2.h"
#include "sdt.h"
#include "madt.h"

#define RSDP_V1_DESC_SIZE			20
#define RSDP_V2_DESC_SIZE			36
#define ACPI_VERSION_1				0
#define ACPI_VERSION_2				2	

typedef void* AcpiTable;

typedef struct {
	// V1.
	char signature[8];
	uint8_t checksum;
	char oemid[6];
	uint8_t revision;
	uint32_t rsdt_addr;
	
	// V2.
	uint32_t length;
	uint64_t xsdt_addr;
	uint8_t extended_checksum;
	uint8_t reserved[3];
} __attribute__((packed)) RsdpDescriptor;

// Give this one a non-typedef-ed name because of circular dependency w/
// madt.h
struct SdtHeader {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oemid[6];
	char oem_table[8];
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed));

typedef struct {
	SdtHeader header;
	uint32_t next[];
} __attribute__((packed)) Rsdt;

typedef struct {
	SdtHeader header;
	uint64_t next[];
} __attribute__((packed)) Xsdt;

typedef union {
	SdtHeader header;
	// It thinks of these as addresses. Really, they're 32-bit ints. 
	union {
		uint32_t *rsdt_next;
		uint64_t *xsdt_next;
	};
} __attribute__((packed)) XsdtOrRsdt;

typedef struct {
	SdtHeader header;
	union {
		uint32_t *rsdt_next;
		uint64_t *xsdt_next;
	};
	bool uses_xsdt;
} __attribute__((packed)) GenericSdt;

typedef struct {
	IoApicList apic_list;
} __attribute__((packed)) AcpiTables;

int InitAcpi(struct stivale2_struct_tag_rsdp rsdp_addr_tag,
			 AcpiTables *acpi_tabs);

AcpiTable FindTable(char *table_id);

/**
 * Initialize the static RSDP_TABLE field given a bootloader tag specifying its
 * location in memory.
 * @input rsdp_addr_tag A bootloader tag containing a 64-bit unsigned int giving
 * 						the location of the RSDP in memory.
 * @output True on success, false on failure (i.e. checksum issue).
 */
bool InitRsdp(struct stivale2_struct_tag_rsdp rsdp_addr_tag);

bool InitSdt();

#endif
