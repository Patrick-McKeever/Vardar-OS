#include "hal/pit.h"
#include "interrupts/io.h"
#include "utils/string.h"

static void (*TIMER_HANDLER)(void);

void
set_pit_periodic(uint16_t hz)
{
	pit_command_t reset_com = {
		.binary_mode		= 	PIT_BINARY,
		.operating_mode		= 	RATE_GENERATOR,
		// We will be transmitting a 16-bit reset value, so PIT data 
		// should be told to expect a low byte and a high byte.
		.access_mode		=	PIT_LO_AND_HI_BYTES,
		.channel			=	0
	};

	uint8_t command_byte;
	memmove(&command_byte, &reset_com, sizeof(pit_command_t));
	outportb(PIT_COMMAND, command_byte);
	
	// Approximate the requested hz. For instance, given 1000hz,
	// we set the reset value to 1,193,180 / 1000 = 1193, a close-
	// enough approximation. Then tell PIT channel 0 to set this
	// as its reset value.
	uint16_t reset_value	=	DEFAULT_PIT_HZ / hz;
	uint8_t reset_lower		=	(uint8_t) reset_value;
	uint8_t reset_upper		=	(uint8_t) (reset_value >> 8);
	outportb(PIT_CHAN0_DATA, reset_lower);
	outportb(PIT_CHAN0_DATA, reset_upper);
}
void
register_pit_handler(void (*handler)(void))
{
	TIMER_HANDLER 			=	handler;
}

void (*get_timer_handler(void))(void)
{
	return TIMER_HANDLER;
}
