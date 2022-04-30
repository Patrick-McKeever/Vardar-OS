#ifndef ELF_H
#define ELF_H

#include <stdint.h>
#include <stddef.h>
#include "proc/proc.h"

/* 64-bit ELF header. We don't care about 32-bit equivalent. */
typedef struct {
	char		magic[4];
	uint8_t		bits;
	uint8_t		endianness;
	/* Version should always be 1. */
	uint8_t		version;
	uint8_t		abi;
	/* Bit of a simplification. The first byte of "reserved0" can
	 * technically be used to denote ABI version, but it's ignored
	 * in Linux/BSDs and useless for statically-linked binaries. */
	uint64_t	reserved;
	uint16_t	file_type;
	uint16_t	arch;
	/* Version should always be 1. */
	uint32_t	version_con;
	uint64_t	entry_pt;
	uint64_t	phdr_offset;
	uint64_t	shdr_offset;
	/* Values of flags are arch-dependent. */
	uint32_t	flags;
	/* Size of this header. */
	uint16_t	hdr_size;
	/* Size of an entry, num entries in the program header table. */
	uint16_t	phdr_entry_size;
	uint16_t	phdr_num_entries;
	/* Size of an entry, num entries in the section header table. */
	uint16_t	shdr_entry_size;
	uint16_t	shdr_num_entries;
	/* Index of section table (not quite clear on this). */
	uint16_t	shdr_index;
} __attribute__((packed)) elf_hdr_t;

typedef enum {
	ELF_32_BITS				=	1,
	ELF_64_BITS				=	2
} elf_bit_mode_t;

typedef enum {
	ELF_LITTLE_ENDIAN		=	1,
	ELF_BIG_ENDIAN			=	2
} elf_endianness_t;

typedef enum {
	ELF_SYSV_ABI			=	0x0,
	ELF_HP_UX_ABI			=	0x1,
	ELF_NETBSD_ABI			=	0x2,
	ELF_LINUX_ABI			=	0x3,
	ELF_HURD_ABI			=	0x4,
	ELF_SOLARIS_ABI			=	0x6,
	ELF_AIX_ABI				=	0x7,
	ELF_IRIX_ABI			=	0x8,
	ELF_FREEBSD_ABI			=	0x9,
	ELF_TRU64_ABI			=	0xA,
	ELF_NOVELL_MOD_ABI		=	0xB,
	ELF_OPENBSD_ABI			=	0xC,
	ELF_OPENVMS_ABI			=	0xD,
	ELF_NS_KERNEL_ABI		=	0xE,
	ELF_AROS_ABI			=	0xF,
	ELF_FENIX_ABI			=	0x1,
	ELF_NUXI_ABI			=	0x11,
	ELF_STRATUS_ABI			=	0x12
} elf_os_abi_t;

typedef enum {
	ELF_NONE				=	0x0,
	ELF_REL					=	0x1,
	ELF_EXEC				=	0x2,
	/* Shared object file. */
	ELF_DYN					=	0x3,
	ELF_CORE				=	0x4,
	ELF_LOOS				=	0xFE00,
	ELF_HIOS				=	0xFEFF,
	ELF_LOPROC				=	0xFF00,
	ELF_HIPROC				=	0xFFFF
} elf_file_type_t;

typedef enum {
	/* Obviously, there are others, but this is the only arch supported
	 * by Vardar as of now. */
	ELF_AMD64				=	0x3E
} elf_arch_t;

/* 64-bit ELF program header. Tells us how to create process image.
 * The relevant distinction between a segment and a section is that
 * segments contain information necessary for runtime execution (inc.
 * virtual memory addrs to which section should be mapped), while 
 * sections contain info relevant to the linker. */
typedef struct {
	uint32_t	type;
	/* Flags are arch-dependent. */
	uint32_t	flags;
	/* Offset of this segment in the ELF file. */
	uint64_t	offset;
	uint64_t	vaddr;
	/* The physical address field is only useful on very particular
	 * systems, where a program would care about the paddr of its
	 * memory. We will not use it. */
	uint64_t	paddr;
	uint64_t	file_size;
	uint64_t	mem_size;
	/* 0/1 indicate no alignment is needed. Otherwise, number of
	 * bytes to which program must be aligned (should be power of 2).*/
	uint64_t	alignment;
} __attribute((packed)) elf_phdr_t;

typedef enum {
	ELF_PHDR_NULL			=	0x0,
	ELF_PHDR_LOAD			=	0x1,
	ELF_PHDR_DYNAMIC		=	0x2,
	ELF_PHDR_INTERP			=	0x3,
	ELF_PHDR_NOTE			=	0x4,
	ELF_PHDR_SHLIB			=	0x5,
	ELF_PHDR_PHDR			=	0x6,
	ELF_PHDR_TLS			=	0x7,
	ELF_PHDR_LOOS			=	0x60000000,
	ELF_PHDR_HIO			=	0x6FFFFFFF,
	ELF_PHDR_LOPROC			=	0x70000000,
	ELF_PHDR_HIPRO			=	0x7FFFFFFF
} elf_phdr_seg_t;

/* 64-bit ELF section header. */
typedef struct {
	/* There is one particular section called ".shstrtab" which serves
	 * as a table of section names. This variable gives the offset of this
	 * particular section's name within that table. We can tell which
	 * section is the shstrtb based on the value of sh_type, which will
	 * be 0x3 in the case of the shstrtab.*/
	uint32_t	name_offset;
	uint32_t	type;
	uint64_t	flags;
	uint64_t	vaddr;
	/* The offset of this section in the ELF file. */
	uint64_t	offset;
	uint64_t	size;
	/* Contains index of associated section. */
	uint32_t	link;
	/* Not 100% sure on this one. */
	uint32_t	info;
	/* The alignment of this field (always power of 2). */
	uint64_t	aligment;
	/* The size of a fixed-size entry, in bytes. */
	uint64_t	ent_size;
} __attribute__((packed)) elf_shdr_t;

typedef enum {
	ELF_SHDR_NULL			=	0x0,
	ELF_SHDR_PROGBITS		=	0x1,
	ELF_SHDR_SYMTAB			=	0x2,
	ELF_SHDR_STRTAB			=	0x3,
	ELF_SHDR_RELA			=	0x4,
	ELF_SHDR_HASH			=	0x5,
	ELF_SHDR_DYNAMIC		=	0x6,
	ELF_SHDR_NOTE			=	0x7,
	ELF_SHDR_NOBITS			=	0x8,
	ELF_SHDR_REL			=	0x9,
	ELF_SHDR_SHLIB			=	0x0A,
	ELF_SHDR_DYNSYM			=	0x0B,
	ELF_SHDR_INIT_ARRAY		=	0x0E,
	ELF_SHDR_FINI_ARRAY		=	0x0F,
	ELF_SHDR_PREINIT_ARRAY	=	0x10,
	ELF_SHDR_GROUP			=	0x11,
	ELF_SHDR_SYMTAB_SHNDX	=	0x12,
	ELF_SHDR_NUM			=	0x13,
	ELF_SHDR_LOOS			=	0x60000000	
} shdr_sec_t;

typedef enum {
	ELF_SHF_WRITE			=	0x1,
	ELF_SHF_ALLOC			=	0x2,
	ELF_SHF_EXECINSTR		=	0x4,
	ELF_SHF_MERGE			=	0x10,
	ELF_SHF_STRINGS			=	0x20,
	ELF_SHF_INFO_LINK		=	0x40,
	ELF_SHF_LINK_ORDER		=	0x80,
	ELF_SHF_OS_NONCONF		=	0x100,
	ELF_SHF_GROUP			=	0x200,
	ELF_SHF_TLS				=	0x400,
	ELF_SHF_MASKOS			=	0x0FF00000,
	ELF_SHF_MASKPROC		=	0xF0000000,
	ELF_SHF_ORDERED			=	0x4000000,
	ELF_SHF_EXCLUDE			=	0x8000000
} shdr_flags_t;

int
parse_elf(uint8_t *raw_elf, pcb_t *pcb);

#endif
