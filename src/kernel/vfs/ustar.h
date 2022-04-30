#ifndef USTAR_H
#define USTAR_H

#define BLOCK_SIZE 512

#include <stddef.h>
#include <stdint.h>


typedef union {
	struct {
		char 		name[100];     
		char 		mode[8];       		
		char 		uid[8];        
		char 		gid[8];        
		char 		size[12];      
		char 		mtime[12];     
		char 		checksum[8];      
		char 		filetype;          
		char 		link_name[100];
		char 		signature[6];
		uint16_t 	version;
		char 		owner[32];
		char 		group[32];
		uint64_t 	device_major;
		uint64_t 	deivce_minor;
		uint8_t 	prefix[155];
	} __attribute__((packed));
	char block[512];
} ustar_entry_t;

typedef enum {
	USTAR_REGULAR		=		0x30,
	USTAR_HARDLINK		=		0x31,
	USTAR_SYMLINK		=		0x32,
	USTAR_CHAR_DEV		=		0x33,
	USTAR_BLOCK_DEV		=		0x34,
	USTAR_DIR			=		0x35,
	USTAR_PIPE			=		0x36
} ustar_filetype_t;

char *
ustar_read(void *ustar, const char *const filename);

ustar_entry_t**
ustar_readdir(void *ustar, const char *const dirname);

#endif
