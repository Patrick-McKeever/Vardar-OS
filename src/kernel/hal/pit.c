#include "hal/pit.h"
#include "interrupts/io.h"
#include "utils/string.h"

static void (*TIMER_HANDLER)(void);

static void
send_pit_command(pit_command_t);

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

	send_pit_command(reset_com);

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

uint32_t
read_pit_count()
{
	uint32_t count			=	0;
	
	// PIT latch command copies specified channel's current count
	// into register, stops updating until it is fully read, and
	// then transmits this when you call inportb on the channel's
	// port.
	pit_command_t read_cnt	= {
		.binary_mode		=	PIT_BINARY,
		.operating_mode		=	0,
		.access_mode		=	PIT_LATCH,
		.channel			=	0
	};

	send_pit_command(read_cnt);

	count 					=	inportb(PIT_CHAN0_DATA);
	count					|=	inportb(PIT_CHAN0_DATA) << 8;
	return count;
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

static void
send_pit_command(pit_command_t command)
{
	uint8_t command_byte;
	memmove(&command_byte, &command, sizeof(pit_command_t));
	outportb(PIT_COMMAND, command_byte);
}

