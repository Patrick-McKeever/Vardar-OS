#ifndef SDT_H
#define SDT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oemid[6];
	char oem_table[8];
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed)) SdtHeader;

bool ValidateSdtChecksum(SdtHeader *sdt);

#endif
