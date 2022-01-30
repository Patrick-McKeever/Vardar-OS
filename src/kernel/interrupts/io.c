#include "io.h"

uint8_t inportb(uint16_t port)
{
	uint8_t val;
	__asm__ __volatile__("inb %1, %0" 
						 : "=a"(val) 
						 : "dN"(port));
	return val;
}

void outportb(uint16_t port, uint8_t byte)
{
	__asm__ __volatile__("outb %1, %0"
						 : /* No outputs */
						 : "dN"(port)
						 , "a"(byte));
}

void io_wait()
{
	// 0x80 is used for motherboard things during boot but useless
	// once in kernel.
	outportb(0x80, 0);
}

