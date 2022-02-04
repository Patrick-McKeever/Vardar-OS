#include "graphics_types.h"

uint32_t PackRgb(RGB rgb, struct stivale2_struct_tag_framebuffer *fb)
{
	// This function assumes 32 bpp with packed pixels (memory model of 0x04).
	if(fb->framebuffer_bpp != 32/* || fb->memory_model != 0x04*/) {
		return 0;
	}

	// Our RGB struct expresses R, G, and B as 8-bit values.
	// The framebuffer will assign a different number of bits per value.
	// We must thus "compress" the 8-bit values into an appropriately-sized
	// approximation by right-shifting them by the difference between their
	// current size (8) and their desired size (whatever the fb says).
	// If the FB's size is actually larger, then we should left shift by the
	// absolute value of that difference, for which we'll use the macro
	// ABS_RIGHT_SHIFT.
	uint32_t r_shift = 8 - fb->red_mask_size;
	uint32_t shifted_r = ABS_RIGHT_SHIFT(rgb.r, r_shift);
	// We should clear all bits ouside the range of the color mask, so "and"
	// it with a string of 1s with a length equivalent to the color mask size.
	uint32_t r_and = (2 << fb->red_mask_size) - 1;
	// And now shift by the color shift value to position the bits correctly.
	uint32_t r = (shifted_r & r_and) << fb->red_mask_shift;

	uint32_t b_shift = 8 - fb->blue_mask_size;
	uint32_t shifted_b = ABS_RIGHT_SHIFT(rgb.b, b_shift);
	uint32_t b_and = (2 << fb->blue_mask_size) - 1;
	uint32_t b = (shifted_b & b_and) << fb->blue_mask_shift;
	
	uint32_t g_shift = 8 - fb->green_mask_size;
	uint32_t shifted_g = ABS_RIGHT_SHIFT(rgb.g, g_shift);
	uint32_t g_and = (2 << fb->green_mask_size) - 1;
	uint32_t g = (shifted_g & g_and) << fb->green_mask_shift;

	// "Or" to pack bits together.
	return r | g | b;
}
