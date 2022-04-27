#include <stdint.h>
#include <stddef.h>
#include "stivale2.h"
//#include <stivale2.h>
 
// We need to tell the stivale bootloader where we want our stack to be.
// We are going to allocate our stack as an array in .bss.
static uint8_t stack[8192];
 
// stivale2 uses a linked list of tags for both communicating TO the
// bootloader, or receiving info FROM it. More information about these tags
// is found in the stivale2 specification.
 
// stivale2 offers a runtime terminal service which can be ditched at any
// time, but it provides an easy way to print out to graphical terminal,
// especially during early boot.
// Read the notes about the requirements for using this feature below this
// code block.
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    // All tags need to begin with an identifier and a pointer to the next tag.
    .tag = {
        // Identification constant defined in stivale2.h and the specification.
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
        // If next is 0, it marks the end of the linked list of header tags.
        .next = 0
    },
    // The terminal header tag possesses a flags field, leave it as 0 for now
    // as it is unused.
    .flags = 0
};

static struct stivale2_header_tag_smp smp_hdr_tag = {
    .tag = {
		.identifier = STIVALE2_HEADER_TAG_SMP_ID,
		.next = (uintptr_t) &terminal_hdr_tag,
	},
	.flags = 0
};

// We are now going to define a framebuffer header tag.
// This tag tells the bootloader that we want a graphical framebuffer instead
// of a CGA-compatible text mode. Omitting this tag will make the bootloader
// default to text mode, if available.
static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    // Same as above.
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        // Instead of 0, we now point to the previous header tag. The order in
        // which header tags are linked does not matter.
		.next = (uintptr_t) &smp_hdr_tag
    },
    // We set all the framebuffer specifics to 0 as we want the bootloader
    // to pick the best it can.
    .framebuffer_width  = 1920,
    .framebuffer_height = 1080/*768*/,
    .framebuffer_bpp    = 16 
};

static struct stivale2_struct_tag_framebuffer framebuffer = {
	.tag = {
		.identifier = STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID,
		.next = (uintptr_t)&framebuffer_hdr_tag
	}
};

static struct stivale2_struct_tag_kernel_base_address base_addr_tag = {
    .tag = {
		.next = (uintptr_t) &framebuffer,
		.identifier = STIVALE2_STRUCT_TAG_KERNEL_BASE_ADDRESS_ID
	}
};

static struct stivale2_struct_tag_pmrs pmr_tag = {
	.tag = {
		.next = (uintptr_t) &base_addr_tag,
		.identifier = STIVALE2_STRUCT_TAG_PMRS_ID
	}
};

static struct stivale2_struct_tag_rsdp rsdp_tag = {
	.tag = {
		.next = (uintptr_t) &pmr_tag,
		.identifier = STIVALE2_STRUCT_TAG_RSDP_ID
	}
};

// The stivale2 specification says we need to define a "header structure".
// This structure needs to reside in the .stivale2hdr ELF section in order
// for the bootloader to find it. We use this __attribute__ directive to
// tell the compiler to put the following structure in said section.
__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {
    // The entry_point member is used to specify an alternative entry
    // point that the bootloader should jump to instead of the executable's
    // ELF entry point. We do not care about that so we leave it zeroed.
    .entry_point = 0,
    // Let's tell the bootloader where our stack is.
    // We need to add the sizeof(stack) since in x86(_64) the stack grows
    // downwards.
    .stack = (uintptr_t)stack + sizeof(stack),
    // Bit 1, if set, causes the bootloader to return to us pointers in the
    // higher half, which we likely want since this is a higher half kernel.
    // Bit 2, if set, tells the bootloader to enable protected memory ranges,
    // that is, to respect the ELF PHDR mandated permissions for the executable's
    // segments.
    // Bit 3, if set, enables fully virtual kernel mappings, which we want as
    // they allow the bootloader to pick whichever *physical* memory address is
    // available to load the kernel, rather than relying on us telling it where
    // to load it.
    // Bit 4 disables a deprecated feature and should always be set.
    .flags = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4),
    // This header structure is the root of the linked list of header tags and
    // points to the first one in the linked list.
    .tags = (uintptr_t)&rsdp_tag
	//.tags = (uintptr_t)&pmr_tag
};
 

// We will now write a helper function which will allow us to scan for tags
// that we want FROM the bootloader (structure tags).
void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id) {
    struct stivale2_tag *current_tag = (void *)stivale2_struct->tags;
    for (;;) {
        // If the tag pointer is NULL (end of linked list), we did not find
        // the tag. Return NULL to signal this.
        if (current_tag == NULL) {
            return NULL;
        }
 
        // Check whether the identifier matches. If it does, return a pointer
        // to the matching tag.
        if (current_tag->identifier == id) {
            return current_tag;
        }
 
        // Get a pointer to the next tag in the linked list and repeat.
        current_tag = (void *)current_tag->next;
    }
}


//#include "kernel/interrupts/idt.h"
#include "interrupts/idt.h"
#include "graphics/graphics_ctx.h"
struct stivale2_struct_tag_terminal *term_str_tag_g;
#include "interrupts/keycodes.h"


static Font *global_font;
//static GraphicsCtx *global_ctx;
#include "graphics/terminal.h"
#include "memory_management/physical_memory_manager.h"
#include "memory_management/virtual_memory_manager.h"
#include "utils/printf.h"
#include "acpi/acpi.h"
#include "hal/cpu_init.h"
#include "hal/io_apic.h"
#include "memory_management/kheap.h"
#include "gdt/gdt.h"
Terminal term;

void (*term_write)(const char *string, size_t length);

void print_key(KeyInfo *key_info)
{
	char c[2];
	c[0] = CharFromScancode(key_info);
	c[1] = 0;
	PrintK(c);
}

void _start(struct stivale2_struct *stivale2_struct) {
	__asm__("cli");
	initialize_gdt((uint64_t) &stack);

	struct stivale2_struct_tag_framebuffer *fb;
	fb = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);
	struct stivale2_struct_tag_memmap *memmap;
	memmap = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MEMMAP_ID);

	struct stivale2_struct_tag_kernel_base_address *kern_base_addr;
	kern_base_addr = stivale2_get_tag(stivale2_struct,
									  STIVALE2_STRUCT_TAG_KERNEL_BASE_ADDRESS_ID);
	struct stivale2_struct_tag_pmrs *pmrs;
	pmrs = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_PMRS_ID);
	
	struct stivale2_struct_tag_terminal *term_str_tag;
    term_str_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_TERMINAL_ID);
	struct stivale2_struct_tag_rsdp *rsdp_addr_tag;
	rsdp_addr_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_RSDP_ID);
	void *term_write_ptr = (void*) term_str_tag->term_write;
	term_write = term_write_ptr;

	struct stivale2_struct_tag_smp *smp_info;
	smp_info = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_SMP_ID);

	// Temporary.
	InitGraphicsCtx(fb);
	//global_ctx = &ctx;
	Font font_obj = InitGnuFont((RGB) {255, 255, 255}, (Dimensions) {9,16});
	global_font = &font_obj;
	InitPmm(memmap);
	InitAcpi(*rsdp_addr_tag);
	ParseMadt();
	InitPageTable(memmap, kern_base_addr, pmrs);
	InitializeIdt();

	unmask_irq(0x1);
	unmask_irq(0x2);
	startup_aps(smp_info);
	
	PrintK("BSP Lapic ID is 0x%h\n", smp_info->bsp_lapic_id);
	//void *a = kalloc(20);

	// This works, but keyboard input breaks.
	//ioapic_route_irq_to_bsp(1, 0x21, 0);
	//ioapic_set_gsi_mask(0x1, 0);
	// Try moving this up later.
	
	font_obj.rgb = (RGB) {255, 0, 0};
	ClearScreen((RGB) {0, 0, 0});
	term = InitTerminal((Dimensions){ fb->framebuffer_width, fb->framebuffer_height/2}, (Coordinate) {0,0},
					 &font_obj, (RGB) {15,90,94}, (RGB){255,255,255}, 3, "VardarOS:~$ ");
	
	WriteBack();
	
	SetKeystrokeConsumer(&HandleKeyStroke);
	//SetKeystrokeConsumer(&print_key);
	__asm__("sti");

    for (;;) {
        asm ("hlt");
    }
}
