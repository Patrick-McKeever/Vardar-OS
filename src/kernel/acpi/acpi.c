#include "acpi.h"
#include "utils/string.h"

static RsdpV2Descriptor RSDP_TABLE;
static GenericSdt SDT_HEAD;
static bool ValidateRsdpChecksum(RsdpV1Descriptor *rsdp_tab);

AcpiTables InitAcpi(struct stivale2_struct_tag_rsdp rsdp_addr_tag)
{
	AcpiTables acpi_tabs;
	InitRsdp(rsdp_addr_tag);
	acpi_tabs.apic_list = ParseMadt();
	return acpi_tabs;
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
		
		bool signatures_match = strncmp(sdt->signature, table_id, 4) == 0;
		if(signatures_match && ValidateSdtChecksum(sdt)) {
			return sdt;	
		}		
	}
	
	return NULL;
}

bool InitRsdp(struct stivale2_struct_tag_rsdp rsdp_addr_tag)
{
	RSDP_TABLE = *((RsdpV2Descriptor*) rsdp_addr_tag.rsdp);

	if(ValidateRsdpChecksum(&RSDP_TABLE.v1_desc)) {
		return true;
	}
	
	return false;	
}

bool InitSdt()
{
	SDT_HEAD.uses_xsdt = RSDP_TABLE.v1_desc.revision >= ACPI_VERSION_2;
	
	if(SDT_HEAD.uses_xsdt) {
		Xsdt xsdt_tab = *((Xsdt*) RSDP_TABLE.xsdt_addr);
		SDT_HEAD.header = xsdt_tab.header;
		SDT_HEAD.xsdt_next = xsdt_tab.next;
	} else {
		Rsdt rsdt_tab = *((Rsdt*) (uintptr_t) RSDP_TABLE.v1_desc.rsdt_addr);
		SDT_HEAD.header = rsdt_tab.header;
		SDT_HEAD.rsdt_next = rsdt_tab.next;	
	}
	
	if(ValidateSdtChecksum(&SDT_HEAD.header)) {
		return true;
	}

	return false;
}

static bool ValidateRsdpChecksum(RsdpV1Descriptor *rsdp_tab) 
{
	int checksum = 0;
	uint8_t *rsdp_bytes = (uint8_t *) rsdp_tab;
	
	for(int i = 0; i < RSDP_V1_DESC_SIZE; ++i) {
		checksum += rsdp_bytes[i];
	}

	// Checksums are valid if lower byte is all set to 0.
	bool v1_valid = checksum && 0xff == 0x00;
	if(rsdp_tab->revision == ACPI_VERSION_1)	{
		return v1_valid;
	}

	for(int i = RSDP_V1_DESC_SIZE; i < RSDP_V2_DESC_SIZE; ++i) {
		checksum += rsdp_bytes[i];
	}
	
	bool v2_valid = checksum && 0xff == 0x00;
	return v1_valid && v2_valid;	
}

