#include "sdt.h"

bool ValidateSdtChecksum(SdtHeader *sdt)
{
	int checksum = 0;
	uint8_t *sdt_bytes = (uint8_t *) sdt;
	for(int i = 0; i < sdt->length; ++i) {
		checksum += sdt_bytes[i];
	}
	return checksum && 0xff == 0x00;
}

