#ifndef GRAPHICS_TYPES
#define GRAPHICS_TYPES

#include <stdint.h>
#include "stivale2.h"

// These macros will perform right or left shifts but allow negatives for shift
// values. If a negative is passed as a shift value, the number is shifted in
// the opposite direction by the absolute value of the shift value.
#define ABS_RIGHT_SHIFT(num, shift) 	 \
	(num >> shift) 		* (shift >= 0) + \
	(num << (shift*-1)) * (shift <  0)

#define ABS_LEFT_SHIFT(num, shift) 	 	 \
	(num << shift) 		* (shift >= 0) + \
	(num >> (shift*-1)) * (shift <  0)

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} RGB;

typedef struct {
	uint16_t x;
	uint16_t y;
} Coordinate;

typedef struct {
	uint16_t width;
	uint16_t height;
} Dimensions;

/**
 * Create a 32-bit packed RGB value given a 3-tuple of 8-bit R, G, and B vals.
 * @input rgb The RGB 3-tuple of 8 bit ints.
 * @input fb  The framebuffer containing positions and sizes of RGB vals.
 */
uint32_t PackRgb(RGB rgb, struct stivale2_struct_tag_framebuffer *fb);

#endif
