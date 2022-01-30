#ifndef GDT_H
#define GDT_H

// Struct for a GDT entry, identifying the base, limit, and priveleges of a
// given segment.
typedef struct {
	// The lower 16 bits of the segment limit.
	uint16_t 	limit;
	// The lower 16 bits of the segment base.
	uint16_t 	base_low;
	// Bits 16-23 of the segment base.
	uint8_t		base_mid;
	// An access byte of the form:
	//	- Present 	(1) - Set to indicate valid segment.
	//	- Privl 	(2) - 0 for kernel.
	//	- S			(1) - 1 for code/data segment.
	//	- Ex		(1) - 1 for code segment.
	//	- Direction (1) - Set to indicate upward growth.
	//	- RW		(1) - Set for read/writable.
	//	- AC		(1) - 0 to indicate not accessed yet.
	uint8_t		access;
	// Split byte:
	//	- Upper 4 bits are final 4 bits of limit.
	//	- Lower 4 bits are flags in the form of:
	//		- Gr	(1) - Set for 4KiB page granularity.
	//		- Sz	(1)	- Set for 32-bit protected mode.
	//		- L		(1) - Set for 64-bit long mode.
	//		- Reserved.
	uint8_t		limit_and_flags;
	// Bits 24-31 of base.	
	uint8_t		base_high;
} __attribute((packed))__ SegmentDescriptor;

#endif
