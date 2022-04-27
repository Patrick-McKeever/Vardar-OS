#ifndef GDT_H
#define GDT_H

// Present (1), kernel (2), code/data (not TSS) (1), executable (CS) (1);
// upwards growth (1), read/writable (1), not accessed (1).
#define KERNEL_CS			0b10011010
// Present (1), kernel (2), code/data (not TSS) (1), 
// non-executable (DS) (1),upwards growth (1), read/writable (1), 
// not accessed (1).
#define KERNEL_DS			0b10010010
// Present (1), user (2), code/data (not TSS) (1), 
// executable (CS) (1),upwards growth (1), read/writable (1), 
// not accessed (1).
#define USER_CS				0b11111010
// Present (1), user (2), code/data (not TSS) (1), 
// non-executable (DS) (1),upwards growth (1), read/writable (1), 
// not accessed (1).
#define USER_DS				0b11110010
#define REAL_M_SEG			0b10000000
#define PROT_M_SEG			0b11001111

#include <stdint.h>
#include <stddef.h>

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
	//	- Lower 4 bits are final 4 bits of limit.
	//	- Upper 4 bits are flags in the form of:
	//		- Gr	(1) - Unit of limit is 4KiB if set, bytes.
	//		- Sz	(1)	- Set for 32-bit protected mode.
	//		- L		(1) - Set for 64-bit long mode.
	//		- Reserved.
	uint8_t		limit_and_flags;
	// Bits 24-31 of base.	
	uint8_t		base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
	uint16_t length;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t flags1;
	uint8_t flags2;
	uint8_t base_high;
	uint32_t base_byte2;
	uint32_t reserved;
} __attribute__((packed)) tss_entry_t;

typedef struct {
    uint32_t reserved0;
    uint64_t RSP0;
    uint64_t RSP1;
    uint64_t RSP2;

    uint32_t reserved1;
    uint32_t reserved2;

    uint64_t IST1;
    uint64_t IST2;
    uint64_t IST3;
    uint64_t IST4;
    uint64_t IST5;
    uint64_t IST6;
    uint64_t IST7;
    
    uint32_t reserved3;
    uint32_t reserved4;

    uint16_t IOBP;
} __attribute__((packed)) tss_t;


typedef struct {
	gdt_entry_t segments[9];
	tss_entry_t tss;
} __attribute__((packed)) gdt_t;

typedef struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) gdt_descriptor_t;

void
initialize_gdt();

#endif
