#include "idt.h"

uint8_t KEYSTROKE;

static IdtEntry IDT[256];

void InitializeIdt() 
{
	SetIdtEntry(1, (void*) isr1, INTERRUPT_GATE);
	RemapPic(0, 8);
	// Ignore all but keyboard interrupts for now.
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

void Isr1Handler()
{
	// Read byte from keyboard.
	KEYSTROKE = inportb(0x60);
	// Tell PIC to resume interrupts now that we've handled this one.
	outportb(MASTER_PIC_COMMAND, 0x20);
	outportb(SLAVE_PIC_COMMAND,  0x20);
}

