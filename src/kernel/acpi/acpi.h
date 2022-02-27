#ifndef ACPI_H
#define ACPI_H

#include <stdint.h>
#include <stdbool.h>
#include "stivale2.h"

#define RSDP_V1_DESC_SIZE			20
#define RSDP_V2_DESC_SIZE			36
#define ACPI_VERSION_1				0
#define ACPI_VERSION_2				2	

typedef void* AcpiTable;

typedef struct {
	char signature[8];
	uint8_t checksum;
	char oemid[6];
	uint8_t revision;
	uint32_t rsdt_addr;
} __attribute__(packed) RsdpV1Descriptor;

typedef struct {
	RsdpV1Descriptor v1_desc;
	uint32_t length;
	uint64_t xsdt_addr;
	uint8_t extended_checksum;
	uint8_t reserved[3];
} __attribute__(packed) RsdpV2Descriptor;

typedef struct {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oemid[6];
	char oem_table[8];
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__(packed) SdtHeader;

typedef struct {
	SdtHeader header;
	uint32_t *next;
} __attribute__(packed) Rsdt;

typedef struct {
	SdtHeader header;
	uint64_t *next;
} __attribute__(packed) Xsdt;

typedef struct {
	bool uses_xsdt;
	SdtHeader header;
	union {
		uint32_t *rsdt_next;
		uint64_t *xsdt_next;
	}
} __attribute__(packed) GenericSdt;

static RsdpV2Descriptor RSDP_TABLE;
static GenericSdt SDT_HEAD;
static bool ValidateRsdpChecksum(RsdpV1Descriptor *rsdp_tab);
static bool ValidateSdtChecksum(SdtHeader *sdt);

void InitAcpi(AcpiTable rsdp_tab)
{
			
}

SdtHeader *FindTable(char *table_id)
{
	uint8_t entry_size = SDT_HEAD.uses_xsdt ? 8 : 4;
	int num_entries = (SDT_HEAD.header.length - sizeof(SDT_HEAD.header)) /
					  entry_size;
		
	for(int i = 0; i < num_entries; ++i) {
		SdtHeader *sdt = (SdtHeader *) (SDT_HEAD.uses_xsdt ? 
										SDT_HEAD.xsdt_next[i] : 
										SDT_HEAD.rsdt_next[i]);
		
		bool signatures_match = strncmp(std->signature, table_id, 4) == 0;
		if(signatures_match && ValdateSdtChekcsum(sdt)) {
			return sdt;	
		}		
	}
	
	return NULL;
}

/**
 * Initialize the static RSDP_TABLE field given a bootloader tag specifying its
 * location in memory.
 * @input rsdp_addr_tag A bootloader tag containing a 64-bit unsigned int giving
 * 						the location of the RSDP in memory.
 * @output True on success, false on failure (i.e. checksum issue).
 */
bool InitRsdp(struct stivale2_struct_tag_rsdp rsdp_addr_tag)
{
	RSDP_TABLE = *((RsdpV2Descriptor*) rsdp_addr_tag.rsdp);

	if(ValidateRsdpChecksum(&RSDP_TABLE)) {
		return true;
	}
	
	return false;	
}

bool InitSdt()
{
	SDT_HEAD.uses_xsdt = RSDP_TABLE.revision >= ACPI_VERSION_2;
	
	if(SDT_HEAD.uses_xsdt) {
		Xsdt xsdt_tab = *((Xsdt*) RSDP_TABLE.xsdt_addr);
		SDT_HEAD.header = xsdt_tab.header;
		SDT_HEAD.xsdt_next = xsdt_tab.next;
	} else {
		Rsdt rsdt_tab = *((Rsdt*) RSDP_TABLE.rsdt_addr);
		SDT_HEAD.header = rsdt_tab.header;
		SDT_HEAD.rsdt_next = rsdt_tab.next;	
	}
	
	if(ValidateSdtChecksum(SDT_HEAD.header)) {
		return true;
	}

	return false;
}

static bool ValidateRsdpChecksum(RsdpV2Descriptor *rsdp_tab) 
{
	int checksum = 0;
	uint8_t *rsdp_bytes = (uint8_t *) rsdp_tab;
	
	for(int i = 0; i < RSDP_V1_DESC_SIZE; ++i) {
		checksum += rsdp_bytes[i];
	}

	// Checksums are valid if lower byte is all set to 0.
	bool v1_valid = checksum && 0xff == 0x00;
	if(rsdp_tab.revision == ACPI_VERSION_1)	{
		return v1_valid;
	}

	for(int i = RSDP_V1_DESC_SIZE; i < RSDP_V2_DESC_SIZE; ++i) {
		checksum += rsdp_bytes[i];
	}
	
	bool v2_valid = chekcsum && 0xff == 0x00;
	return v1_valid && v2_valid;	
}

static bool ValidateSdtChecksum(SdtHeader *sdt)
{
	int checksum = 0;
	uint8_t *sdt_bytes = (uint8_t *) sdt;
	for(int i = 0; i < sdt->length; ++i) {
		checksum += sdt_bytes[i];
	}
	return checksum && 0xff == 0x00;
}

#endif
