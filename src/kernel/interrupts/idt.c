#include "idt.h"
#include "../utils/misc.h"
#include "../stivale2.h"
volatile uint8_t SCANCODE;

static IdtEntry IDT[256];

void InitializeIdt() 
{
	//SetIdtEntry(1, (void*) isr1, INTERRUPT_GATE);
	// Keyboard input is IRQ 1 + 0x20 offset = 0x21.
	SetIdtEntry(0x21, (void*) isr1, INTERRUPT_GATE);
		
	// Due to historical quirks, IBM already maps ISRs [0x0,0x1F] to various
	// hardware interrupts. This conflicts with IRQs, which occupy part of the
	// same range (specifically [0x0,0xF]). To resolve this conflicts, PICs
	// add a small "vector offset" to IRQ values before mapping them to ISRs.
	// We'll map IRQs [0x0,0xF] to [0x20,0x2F], which requires the master PIC
	// (responsible for [0x0,0x7]) to have a vector offset of 0x20 and the
	// slave (responsible for [0x8,0xF]) to have an offset of 0x28.
	RemapPic(0x20, 0x28);

	// Ignore all but keyboard interrupts (IRQ 0x1) for now.
	outportb(MASTER_PIC_DATA, 0xfd);
	outportb(SLAVE_PIC_DATA,  0xff);
	

	IdtDescriptor idt_desc = {
		.bounds = 256 * sizeof(IdtEntry) - 1,
		.base	= (uint64_t) IDT
	};
	LoadIdt((uint64_t) &idt_desc);
}

void SetIdtEntry(uint8_t vector, void *isr, uint8_t flags)
{
	IdtEntry *desc 		= 	&IDT[vector];
	desc->handler_low	=	LOW_ISR_OFFSET(isr);
	desc->handler_mid	=	MID_ISR_OFFSET(isr);
	desc->handler_high	=	HIGH_ISR_OFFSET(isr);
	desc->segment		=	0x28;
	desc->ist			=	0;	
	desc->attributes	=	flags;
	desc->reserved		=	0;	
}

void RemapPic(int master_offset, int slave_offset)
{
	// Save Interrupt Masking register (IMR).
	// IMR is a bitmap of lines going into PIC.
	// If bit of relevant line is set, PIC ignores the IRQ.
	uint8_t master_int_mask, slave_int_mask;
	master_int_mask = inportb(MASTER_PIC_DATA);
	slave_int_mask = inportb(SLAVE_PIC_DATA);

	// After receiving 0x11 command, PICs will await 3 more bytes:
	//		- The vector offset (ICW1);
	//		- The master/slave configuration (ICW2);
	//		- Environment info.
	outportb(MASTER_PIC_COMMAND, ICW1 | ICW1_ICW4);
	outportb(SLAVE_PIC_COMMAND, ICW1 | ICW1_ICW4);
	
	// Set master and slave PIC vector offsets.
	outportb(MASTER_PIC_DATA, master_offset);
	outportb(SLAVE_PIC_DATA, slave_offset);
	
	// Tell master PIC that there is a slave PIC connected to IRQ 
	// line 2 (0000 0100).	
	outportb(MASTER_PIC_DATA, 4);
	// Tell slave PIC that it sends through IRQ2 - its 
	// "cascade identity".
	outportb(SLAVE_PIC_DATA, 2);
	
	// I think this tells the PICs that we're using x86 arch?
	outportb(MASTER_PIC_DATA, ICW4_X86);
	outportb(SLAVE_PIC_DATA, ICW4_X86);
	
	// Rather than restoring saved masks, only receive mouse/kb ints.
	outportb(MASTER_PIC_DATA, master_int_mask);
	outportb(SLAVE_PIC_DATA, slave_int_mask);
}


#include "../graphics/font.h"
#include "../graphics/graphics_ctx.h"
#include "keycodes.h"
#include <stdbool.h>

static KeyInfo KEY_INFO;

extern GraphicsCtx *global_ctx;
extern int global_x;
extern Font *global_font;
void Isr1Handler()
{
	// Read byte from keyboard.
	KEY_INFO.scancode = inportb(0x60);
	switch(KEY_INFO.scancode) {
		case BACKSPACE_PRESSED:
			KEY_INFO.backspace = true;
			break;

		case BACKSPACE_RELEASED:
			KEY_INFO.backspace = false;
			break;
		
		case LEFT_SHIFT_PRESSED:
		case RIGHT_SHIFT_PRESSED:
			KEY_INFO.shift = true;
			break;

		case LEFT_SHIFT_RELEASED:
		case RIGHT_SHIFT_RELEASED:
			KEY_INFO.shift = false;
			break;
		
		case CTRL_PRESSED:
			KEY_INFO.ctrl = true;
			break;

		case CTRL_RELEASED:
			KEY_INFO.ctrl = false;
			break;
	}
	
	KeystrokeConsumer key_consumer = GetKeystrokeConsumer();
	key_consumer(&KEY_INFO);
	char c = CharFromScancode(&KEY_INFO);
	DrawChar(global_ctx, global_font, (Coordinate) {global_x, 0}, c);
	WriteBack(global_ctx);
	// Update these vals later.
	if((c <= 122 && c >= 97) || c == ' ')	
		global_x += global_font->width;

	// Tell PIC to resume interrupts now that we've handled this one.
	outportb(MASTER_PIC_COMMAND, 0x20);
}

