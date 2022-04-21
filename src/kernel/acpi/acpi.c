#include "acpi.h"
#include "utils/string.h"


static RsdpDescriptor RSDP_TABLE;
static GenericSdt SDT_HEAD;
static bool ValidateRsdpChecksum(RsdpDescriptor *rsdp_tab);
static bool ValidateSdtChecksum(AcpiTable generic_sdt, int len);

int InitAcpi(struct stivale2_struct_tag_rsdp rsdp_addr_tag)
{
	if(! InitRsdp(rsdp_addr_tag))
		return -1;
	InitSdt();
	return 0;
}

// FindTable fails when trying to find "APIC" signature (i.e. MADT).
// Resolve tomorrow.
AcpiTable FindTable(char *table_id)
{
	uint8_t entry_size = SDT_HEAD.uses_xsdt ? 8 : 4;
	int num_entries = (SDT_HEAD.header.length - sizeof(SDT_HEAD.header)) /
					  entry_size;
		
	for(int i = 0; i < num_entries; ++i) {
		SdtHeader *sdt = (SdtHeader *) (SDT_HEAD.uses_xsdt ? 
										 SDT_HEAD.xsdt_next[i] : 
										 SDT_HEAD.rsdt_next[i]);
		
		bool signatures_match = strncmp(sdt->signature, table_id, 4) == 0;
		bool valid_checksum = ValidateSdtChecksum((AcpiTable) sdt, sdt->length);
		if(signatures_match && valid_checksum) {
			return (AcpiTable) sdt;	
		}		
	}
	
	return NULL;
}

bool InitRsdp(struct stivale2_struct_tag_rsdp rsdp_addr_tag)
{
	RSDP_TABLE = *((RsdpDescriptor*) rsdp_addr_tag.rsdp);

	if(ValidateRsdpChecksum(&RSDP_TABLE)) {
		return true;
	}
	
	return false;	
}

bool InitSdt()
{
	SDT_HEAD.uses_xsdt = RSDP_TABLE.revision >= ACPI_VERSION_2;
	bool validated;
	if(SDT_HEAD.uses_xsdt) {
		Xsdt *xsdt_tab = ((Xsdt*) (uintptr_t) RSDP_TABLE.xsdt_addr);
		validated = ValidateSdtChecksum((void*) xsdt_tab, xsdt_tab->header.length);
		SDT_HEAD.header = xsdt_tab->header;
		SDT_HEAD.xsdt_next = xsdt_tab->next;
	} else {
		Rsdt *rsdt_tab = ((Rsdt*) (uintptr_t) RSDP_TABLE.rsdt_addr);
		validated = ValidateSdtChecksum((void*) rsdt_tab, rsdt_tab->header.length);
		SDT_HEAD.header = rsdt_tab->header;
		SDT_HEAD.rsdt_next = rsdt_tab->next;
	}
	
	if(validated) {
		return true;
	}

	return false;
}

static bool ValidateRsdpChecksum(RsdpDescriptor *rsdp_tab) 
{
	int checksum = 0;
	uint8_t *rsdp_bytes = (uint8_t *) rsdp_tab;
	
	for(int i = 0; i < RSDP_V1_DESC_SIZE; ++i) {
		checksum += rsdp_bytes[i];
	}

	// Checksums are valid if lower byte is all set to 0.
	bool v1_valid = (checksum & 0xff) == 0x00;
	if(rsdp_tab->revision == ACPI_VERSION_1)	{
		return v1_valid;
	}

	for(int i = RSDP_V1_DESC_SIZE; i < RSDP_V2_DESC_SIZE; ++i) {
		checksum += rsdp_bytes[i];
	}
	
	bool v2_valid = (checksum & 0xff) == 0x00;
	return v1_valid && v2_valid;	
}

static bool ValidateSdtChecksum(AcpiTable sdt, int len)
{
	int checksum = 0;

	// First byte is actually a bool telling us if this is XSDT or RSDT. All
	// subsequent bytes are identical to the RSDT/XSDT from memory.
	uint8_t *sdt_bytes = (uint8_t *) (sdt);
	for(int i = 0; i < len; ++i) {
		checksum += sdt_bytes[i];
	}
	return (checksum & 0xff) == 0x00;
}
