#ifndef IDT_H
#define IDT_H

#include "io.h"
#include "key_input.h"

// Calculating offset of ISR from void ptr.
#define LOW_ISR_OFFSET(isr)		(uint64_t) 	isr 		& 0xFFFF
#define MID_ISR_OFFSET(isr)		((uint64_t) isr >> 16) 	& 0xFFFF 
#define HIGH_ISR_OFFSET(isr)	((uint64_t) isr >> 32) 	& 0xFFFFFFFF

// Command/data ports for PICs.
#define MASTER_PIC_COMMAND 		0x20
#define MASTER_PIC_DATA 		0x21
#define SLAVE_PIC_COMMAND 		0xa0
#define SLAVE_PIC_DATA 			0xa1

// Initialization control words (ICWs) for PICs.
#define ICW1 					0x10
#define ICW1_ICW4 				0x01
#define ICW4_X86 				0x01

// Flags for 64-bit gate types.
#define INTERRUPT_GATE			0x8E
#define TRAP_GATE				0x8F
#define TASK_GATE				0x85


// IDT entry.
typedef struct {
	// Lower 16 bits of ISR ptr.
	uint16_t	handler_low;
	// Defines index, permissions, and table (LDT/GDT) of IDT segment.
	uint16_t	segment;
	// Offset of interrupt stack table (IST), which will be loaded into memory 
	// when this interrupt occurs.
	uint8_t		ist;
	// Is trap gate, interrupt gate, or task gate used? 
	uint8_t		attributes;
	// Bits 16-32 of ISR ptr.
	uint16_t	handler_mid;
	// Bits 32-64 of ISR ptr.
	uint32_t	handler_high;
	uint32_t	reserved;
} __attribute__((packed)) IdtEntry; 

// Gives location of IDT in memory.
typedef struct {
	uint16_t bounds;
	uint64_t base;
} __attribute__((packed)) IdtDescriptor;


// The ISR1 handler defined in the asm file.
extern void 	isr1();
// Loads the IDT referenced by given IDT descriptor as the IDT. 
extern void 	LoadIdt(uint64_t idtr);
extern uint8_t	volatile SCANCODE;

/**
 * Set relevant IDT entries. At this point in time, we're only concerned with
 * keyboard interrupts.
 */
void InitializeIdt(); 

/**
 * Set the IDT entry of the given IRQ vector to point to the given ISR.
 * @input vector The IRQ corresponding to the desired interrupt.
 * @input isr A pointer to the interrupt service routine (ISR).
 * @input flags flags Flag of IDT entry, either 0xE (interrupt) or 0xF (trap).
 */
void SetIdtEntry(uint8_t vector, void *isr, uint8_t flags);

/**
 * Set offsets of PICs. (Offset + IRQ = Interrupt Vector). Also configure 
 * master/slave relation of PICs, tell them about the environment, and set 
 * their IMRs (so they know which IRQ lines to ignore).
 * @input master_offset Vector offset of master PIC. 
 * @input slave_offset  Vector offset of slave PIC. 
 */
void RemapPic(int master_offset, int slave_offset);

/**
 * Keyboard interrupt handler. Store keystroke in global variable (KEYSTROKE),
 * and instruct PIC to resume interrupts.
 */
void Isr1Handler();

#endif
