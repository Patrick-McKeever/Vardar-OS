#include "font.h"

static const uint8_t GnuFontBitmap[128][16] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x48, 0x48, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x24, 0x24, 0x7e, 0x24, 0x24, 0x7e, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x10, 0x7c, 0x92, 0x12, 0x14, 0x38, 0x50, 0x90, 0x92, 0x7c, 0x10, 0x00, 0x00 },
	{ 0x00, 0x00, 0x84, 0x4a, 0x4a, 0x24, 0x10, 0x10, 0x48, 0xa4, 0xa4, 0x42, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x0c, 0x12, 0x12, 0x12, 0x0c, 0x8c, 0x52, 0x22, 0x52, 0x8c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x20, 0x10, 0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x20, 0x00, 0x00 },
	{ 0x00, 0x08, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x10, 0x08, 0x00, 0x00 },
	{ 0x00, 0x00, 0x10, 0x92, 0x54, 0x38, 0x54, 0x92, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0xfe, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x20, 0x20, 0x10 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x80, 0x40, 0x40, 0x20, 0x10, 0x10, 0x08, 0x04, 0x04, 0x02, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x10, 0x18, 0x14, 0x12, 0x10, 0x10, 0x10, 0x10, 0x10, 0xfe, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x7c, 0x82, 0x82, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0xfe, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xfe, 0x80, 0x40, 0x20, 0x70, 0x80, 0x80, 0x80, 0x82, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x40, 0x60, 0x50, 0x48, 0x44, 0x42, 0xfe, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xfe, 0x02, 0x02, 0x7a, 0x86, 0x80, 0x80, 0x80, 0x82, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x78, 0x04, 0x02, 0x02, 0x7a, 0x86, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xfe, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x38, 0x44, 0x82, 0x44, 0x38, 0x44, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x7c, 0x82, 0x82, 0x82, 0xc2, 0xbc, 0x80, 0x80, 0x40, 0x3c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x30, 0x30, 0x20, 0x20, 0x10 },
	{ 0x00, 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x7c, 0x82, 0x82, 0x80, 0x40, 0x20, 0x10, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x7c, 0x82, 0x82, 0xf2, 0x8a, 0xca, 0xb2, 0x02, 0x02, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x10, 0x28, 0x44, 0x82, 0x82, 0x82, 0xfe, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x3e, 0x42, 0x82, 0x42, 0x3e, 0x42, 0x82, 0x82, 0x42, 0x3e, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x7c, 0x82, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x82, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x3e, 0x42, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x42, 0x3e, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xfe, 0x02, 0x02, 0x02, 0x3e, 0x02, 0x02, 0x02, 0x02, 0xfe, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xfe, 0x02, 0x02, 0x02, 0x3e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x7c, 0x82, 0x02, 0x02, 0x02, 0xe2, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x82, 0x82, 0x82, 0x82, 0xfe, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xf1, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x42, 0x3c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x82, 0x42, 0x22, 0x12, 0x0e, 0x0a, 0x12, 0x22, 0x42, 0x82, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0xfe, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x82, 0x82, 0xc6, 0xaa, 0xaa, 0x92, 0x92, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x82, 0x82, 0x86, 0x8a, 0x92, 0xa2, 0xc2, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x7e, 0x82, 0x82, 0x82, 0x7e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x8a, 0x92, 0x7c, 0x20, 0xc0, 0x00 },
	{ 0x00, 0x00, 0x7e, 0x82, 0x82, 0x82, 0x7e, 0x12, 0x22, 0x42, 0x82, 0x82, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x7c, 0x82, 0x82, 0x02, 0x1c, 0x60, 0x80, 0x82, 0x82, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xfe, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x82, 0x82, 0x82, 0x44, 0x44, 0x44, 0x28, 0x28, 0x28, 0x10, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x82, 0x82, 0x82, 0x82, 0x92, 0x92, 0x92, 0x92, 0xaa, 0x44, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x82, 0x82, 0x44, 0x28, 0x10, 0x10, 0x28, 0x44, 0x82, 0x82, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x82, 0x82, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xfe, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x02, 0xfe, 0x00, 0x00, 0x00 },
	{ 0x00, 0x78, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x78, 0x00, 0x00 },
	{ 0x00, 0x00, 0x02, 0x04, 0x04, 0x08, 0x10, 0x10, 0x20, 0x40, 0x40, 0x80, 0x00, 0x00, 0x00 },
	{ 0x00, 0x3c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x00, 0x00 },
	{ 0x00, 0x00, 0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00 },
	{ 0x00, 0x08, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x80, 0x80, 0xfc, 0x82, 0xc2, 0xbc, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x02, 0x02, 0x02, 0x7a, 0x86, 0x82, 0x82, 0x82, 0x86, 0x7a, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x82, 0x02, 0x02, 0x02, 0x82, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x80, 0x80, 0x80, 0xbc, 0xc2, 0x82, 0x82, 0x82, 0xc2, 0xbc, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x82, 0x82, 0xfe, 0x02, 0x02, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x70, 0x88, 0x88, 0x08, 0x08, 0x3e, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x42, 0x42, 0x42, 0x3c, 0x02, 0x7c, 0x82, 0x82, 0x7c },
	{ 0x00, 0x00, 0x02, 0x02, 0x02, 0x7a, 0x86, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x18, 0x00, 0x00, 0x1c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x60, 0x00, 0x00, 0x70, 0x40, 0x40, 0x40, 0x40, 0x40, 0x42, 0x42, 0x42, 0x3c },
	{ 0x00, 0x00, 0x02, 0x02, 0x02, 0x82, 0x62, 0x1a, 0x06, 0x1a, 0x62, 0x82, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x1c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x92, 0x92, 0x92, 0x92, 0x92, 0x82, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x86, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x86, 0x82, 0x82, 0x82, 0x86, 0x7a, 0x02, 0x02, 0x02 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0xbc, 0xc2, 0x82, 0x82, 0x82, 0xc2, 0xbc, 0x80, 0x80, 0x80 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0x8c, 0x84, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x82, 0x02, 0x7c, 0x80, 0x82, 0x7c, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x08, 0x08, 0x7e, 0x08, 0x08, 0x08, 0x08, 0x88, 0x70, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0xbc, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x82, 0x44, 0x44, 0x28, 0x28, 0x10, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x82, 0x92, 0x92, 0x92, 0xaa, 0x44, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x44, 0x28, 0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x62, 0x5c, 0x40, 0x42, 0x3c },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x40, 0x20, 0x10, 0x08, 0x04, 0xfe, 0x00, 0x00, 0x00 },
	{ 0x00, 0xe0, 0x10, 0x10, 0x10, 0x20, 0x18, 0x18, 0x20, 0x10, 0x10, 0x10, 0xe0, 0x00, 0x00 },
	{ 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00 },
	{ 0x00, 0x0e, 0x10, 0x10, 0x10, 0x08, 0x30, 0x30, 0x08, 0x10, 0x10, 0x10, 0x0e, 0x00, 0x00 },
	{ 0x00, 0x00, 0x8c, 0x92, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};


Font InitFont(const uint8_t (*matrix)[16], RGB rgb, Dimensions dims)
{
	Font font = {
		.matrix = matrix,
		.rgb = rgb,
		.height = dims.height,
		.width = dims.width
	};

	return font;

}
Font InitGnuFont(RGB rgb, Dimensions dims)
{
	return InitFont(GnuFontBitmap, rgb, dims);
}