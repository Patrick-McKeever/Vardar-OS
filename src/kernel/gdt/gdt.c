#include "gdt/gdt.h"
#include "utils/string.h"

extern void load_gdt(uint64_t gdtr);
static gdt_t GDT;
static gdt_descriptor_t GDT_DESC; 
static tss_t TSS;

void set_tss_entry(uint64_t base, uint8_t flags, uint8_t access)
{
    GDT.tss.length = 104;
    GDT.tss.base_low = base & 0xFFFF;
    GDT.tss.base_mid = (base >> 16) & 0xFF;
    GDT.tss.flags1 = access;
    GDT.tss.flags2 = flags;
    GDT.tss.base_high = (base >> 24) & 0xFF;
    GDT.tss.base_byte2 = (base >> 32);
    GDT.tss.reserved = 0;
}

void init_tss(uint64_t stack)
{
    set_tss_entry((uintptr_t)&TSS, 0x20, 0x89);
    memset((void *)&TSS, 0, sizeof(tss_t));

    TSS.RSP0 = stack;
    TSS.IST1 = 0; // Disable IST
}

static inline void load_tss(uint16_t tss_selector)
{
    asm volatile("ltr %%ax" :: "a"(tss_selector) : "memory");
}

void
initialize_gdt(uint64_t stack)
{
	// Bear in mind that the x86-64 architecture does not use GDT for
	// segmentation.  Although there are base and bound entries, they are
	// effectively ignored by the OS. Instead, the GDT is used solely to
	// specify the existence of certain code/data segments and TSSs.
	//
	// Null descriptor
    GDT.segments[0].limit = 0;
    GDT.segments[0].base_low = 0;
    GDT.segments[0].base_mid = 0;
    GDT.segments[0].access = 0;
    GDT.segments[0].limit_and_flags = 0;
    GDT.segments[0].base_high = 0;

    // 16 bit kernel CS
    GDT.segments[1].limit = 0xffff;
    GDT.segments[1].base_low = 0;
    GDT.segments[1].base_mid = 0;
    GDT.segments[1].access = 0x9A;
    GDT.segments[1].limit_and_flags = 0x80;
    GDT.segments[1].base_high = 0;

    // 16 bit kernel DS
    GDT.segments[2].limit = 0xffff;
    GDT.segments[2].base_low = 0;
    GDT.segments[2].base_mid = 0;
    GDT.segments[2].access = 0x9A;
    GDT.segments[2].limit_and_flags = 0x80;
    GDT.segments[2].base_high = 0;

    // 32 bit kernel CS
    GDT.segments[3].limit = 0xffff;
    GDT.segments[3].base_low = 0;
    GDT.segments[3].base_mid = 0;
    GDT.segments[3].access = 0x9A;
    GDT.segments[3].limit_and_flags = 0xCF;
    GDT.segments[3].base_high = 0;

    // 32 bit kernel DS
    GDT.segments[4].limit = 0xffff;
    GDT.segments[4].base_low = 0;
    GDT.segments[4].base_mid = 0;
    GDT.segments[4].access = 0x92;
    GDT.segments[4].limit_and_flags = 0xCF;
    GDT.segments[4].base_high = 0;

    // 64 bit kernel CS
    GDT.segments[5].limit = 0;
    GDT.segments[5].base_low = 0;
    GDT.segments[5].base_mid = 0;
    GDT.segments[5].access = 0x9A;
    GDT.segments[5].limit_and_flags = 0xA2;
    GDT.segments[5].base_high = 0;

    // 64 bit kernel DS
    GDT.segments[6].limit = 0;
    GDT.segments[6].base_low = 0;
    GDT.segments[6].base_mid = 0;
    GDT.segments[6].access = 0x92;
    GDT.segments[6].limit_and_flags = 0xA0;
    GDT.segments[6].base_high = 0;

    // 64 bit user CS
    GDT.segments[7].limit = 0;
    GDT.segments[7].base_low = 0;
    GDT.segments[7].base_mid = 0;
    GDT.segments[7].access = 0xFA;
    GDT.segments[7].limit_and_flags = 0x20;
    GDT.segments[7].base_high = 0;

    // 64 bit user DS
    GDT.segments[8].limit = 0;
    GDT.segments[8].base_low = 0;
    GDT.segments[8].base_mid = 0;
    GDT.segments[8].access = 0xF2;
    GDT.segments[8].limit_and_flags = 0;
    GDT.segments[8].base_high = 0;


	init_tss(stack);

	GDT_DESC.limit = sizeof(gdt_t) - 1;
	GDT_DESC.base = (uintptr_t) &GDT;

	load_gdt((uintptr_t) &GDT_DESC);
	load_tss(0x48);	
}

