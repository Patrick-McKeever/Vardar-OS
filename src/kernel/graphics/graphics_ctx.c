#include "graphics_ctx.h"
#include "../utils/misc.h"
#include <stddef.h>
#include <limits.h>
#include <stdbool.h>

// Solution until we implement PMM.
static uint32_t BACK_BUFFER[786432];

GraphicsCtx InitGraphicsCtx(struct stivale2_struct_tag_framebuffer *fb)
{
	GraphicsCtx ctx = {
		.fb = fb,
		.dirty_block_str = 0,
		.buffer = BACK_BUFFER,
		.num_rows = 64,
		.row_height = fb->framebuffer_height / 64
	};
	return ctx;
}

void WriteBack(GraphicsCtx *ctx)
{
	for(int x = 0; x < ctx->num_rows; ++x) {
		if(GetNthBit(ctx->dirty_block_str, x)) {
			WriteBackCell(ctx, x);
		}
	}
}

void WriteBackCell(GraphicsCtx *ctx, uint8_t cell_ind)
{
	int buff_index = cell_ind * ctx->row_height * ctx->fb->framebuffer_width;
	int screen_index = cell_ind * ctx->row_height * ctx->fb->framebuffer_pitch;

	for(int pixel_row = 0; pixel_row < ctx->row_height; ++pixel_row) {
		void *dest = (void *) ctx->fb->framebuffer_addr + screen_index;
		unsigned short *src = ctx->buffer + buff_index;
		size_t size = ctx->fb->framebuffer_width * ctx->fb->framebuffer_bpp / 8;
		memmove(dest, src, size);
		
		buff_index += ctx->fb->framebuffer_width;
		screen_index += ctx->fb->framebuffer_pitch;
	}
	
	ClearNthBit(&ctx->dirty_block_str, cell_ind);
}

void DrawRect(GraphicsCtx *ctx, Coordinate coords, Dimensions dims, RGB rgb)
{
	uint32_t packed_rgb = PackRgb(rgb, ctx->fb);
	for(int j = coords.y; j < coords.y + dims.height; ++j) {
		int index = j * ctx->fb->framebuffer_pitch + coords.x >> 1;
		size_t size = dims.width * ctx->fb->framebuffer_bpp / 8;
		memset(ctx->buffer + index, packed_rgb, size);
	}
	int starting_row = coords.y / ctx->row_height;
	int ending_row = (coords.y + dims.height) / ctx->row_height;
	for(int i = starting_row; i < ending_row; ++i) {
		SetNthBit(&ctx->dirty_block_str, i);
	}
}

void PrintStr(GraphicsCtx *ctx, Font *font, Coordinate coords, char* str)
{
	int x = coords.x, y = coords.y;
	char c;
	// Iterate through chars, terminate at /0.
	for(int k = 0; (c = *(str + k)) != 0; ++k) {
		bool exceeds_fb_width = (coords.x + x) > ctx->fb->framebuffer_width;
		if(c == '\n' || exceeds_fb_width) {
			x = coords.x;
			y += font->height;
		} else {
			x += font->width;
			Coordinate new_coords = {coords.x + x, coords.y + y};
			DrawChar(ctx, font, new_coords, c);
		}

	}
}

void DrawChar(GraphicsCtx *ctx, Font *font, Coordinate coords, char c)
{
	const uint8_t *char_bmp = font->matrix[(int) c];
	uint32_t packed_rgb = 0xFFFFFFFF/*PackRgb(font->rgb, ctx->fb)*/;
	
	for(int j = 0; j < font->height; ++j) {
		const uint8_t row = char_bmp[j];
		for(int i = 0; i < font->width; ++i) {
			if(GetNthBit(row, i)) {
				int index = coords.x + i + 
							(coords.y + j) * ctx->fb->framebuffer_width;
				*((uint32_t*) ctx->buffer + index) = packed_rgb;
			}
		}
	}
	
	int starting_row = coords.y / ctx->row_height;
	int ending_row = (coords.y + font->height) / ctx->row_height;
	for(int i = starting_row; i < ending_row; ++i) {
		SetNthBit(&ctx->dirty_block_str, i);
	}
}

