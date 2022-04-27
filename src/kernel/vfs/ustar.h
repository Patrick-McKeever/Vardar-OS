#ifndef USTAR_H
#define USTAR_H
#include <stddef.h>
#include <stdint.h>

typedef struct {
	char		name[100];
	uint64_t 	mode;
	uint64_t	uid;
	uint64_t	gid;
	uint8_t		size[12];
	uint64_t	checksum;
	uint8_t		filetype;
	char		link_name[100];
	char		signature[6];
	uint16_t	version;
	uint8_t		owner[32];
	uint8_t		group[32];
	uint64_t	device_major;
	uint64_t	device_minor;
	uint8_t		prefix[155];
} ustar_header_t;

typedef enum {
	USTAR_REGULAR		=		0x30,
	USTAR_HARDLINK		=		0x31,
	USTAR_SYMLINK		=		0x32,
	USTAR_CHAR_DEV		=		0x33,
	USTAR_BLOCK_DEV		=		0x34,
	USTAR_DIR			=		0x35,
	USTAR_PIPE			=		0x36
} ustar_filetype_t;

#endif
