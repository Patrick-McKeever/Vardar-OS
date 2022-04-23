#ifndef PIT_H
#define PIT_H

#include <stdint.h>

#define DEFAULT_PIT_HZ		1193180

typedef enum {
	PIT_CHAN0_DATA			=	0x40,
	PIT_CHAN1_DATA			=	0x41,
	PIT_CHAN2_DATA			=	0x42,
	PIT_COMMAND				=	0x43
} pit_reg_t;

typedef enum {
	// Interrupt on terminal count. When command reg is written to, PIT
	// waits for reload register to be set before beginning countdown.
	// When timer decrements to 0, output goes high and reload register
	// is reset to 0xFFFF (though subsequent reloads will not trigger
	// intterrupts) and decrements until register is reset.
	INT_TERMINAL_COUNT		=	0b000,
	// Hardware retriggerable one-shot. Usable only for PIT channel 2.
	// Essentially same as previous mode, but doesn't start decrement
	// until gate input goes to active high.
	RETRIG_ONE_SHOT			=	0b001,
	// Rate generator. When counter is fully decremented, count will be
	// reset to reload value, and will continue to generate interrupts
	// whenever value is decremented to 0.
	RATE_GENERATOR			=	0b010,
	// These 3 are complex and rarely-used. Read the manual if you care.
	SQUARE_WAVE_GENERATOR	=	0b011,
	SOFTWARE_STROBE			=	0b100,
	HARDWARE_STROBE			=	0b101,
	// 0b110 and 0b111 are aliases of the rate generator and square wave
	// generator modes, respectively.
} pit_op_mode_t;

typedef enum {
	PIT_LATCH				=	0b00,
	PIT_LOBYTE_ONLY			=	0b01,
	PIT_HIBYTE_ONLY			=	0b10,
	PIT_LO_AND_HI_BYTES		=	0b11
} pit_access_mode_t;

typedef enum {
	PIT_BINARY				=	0,
	PIT_BCD					=	1
} pit_binary_mode_t;

typedef struct {
	uint8_t binary_mode		:	1;
	uint8_t operating_mode	:	3;
	uint8_t access_mode		:	2;
	uint8_t channel			:	2;
} __attribute__((packed)) pit_command_t;

void
set_pit_periodic(uint16_t hz);

void
register_pit_handler(void (*handler)(void));

void (*get_timer_handler(void))(void);

#endif
