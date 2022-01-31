#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include "graphics_types.h"

typedef struct {
	// Nth row contains an 8-bit bitmap for ASCII code N. 
	const uint8_t (*matrix)[16];
	// This requires memcpy, but is a good idea for later.
	//uint16_t colored_bitmap_[256][128];
	RGB rgb;
	uint8_t height, width;
} Font;

// Once malloc is implemented, have this return a ptr, and accept uint8_t**.
Font InitFont(const uint8_t (*matrix)[16], RGB rgb, Dimensions dims);
Font InitGnuFont(RGB rgb, Dimensions dims);
#endif

