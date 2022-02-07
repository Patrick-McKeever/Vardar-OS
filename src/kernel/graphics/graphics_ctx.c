#include "graphics_ctx.h"
#include "utils/misc.h"
#include "utils/string.h"
#include <stddef.h>
#include <limits.h>
#include <stdbool.h>

// Solution until we implement PMM.
static uint32_t BACK_BUFFER[4147200];
static GraphicsCtx GLOBAL_CTX;

void InitGraphicsCtx(struct stivale2_struct_tag_framebuffer *fb)
{
	GraphicsCtx ctx = {
		.fb = fb,
		.dirty_block_str = 0,
		.buffer = BACK_BUFFER,
		.num_rows = 64,
		.row_height = fb->framebuffer_height / 64
	};
	GLOBAL_CTX = ctx;
	//return ctx;
}

void ClearScreen(RGB rgb)
{
	int num_pixels = GLOBAL_CTX.fb->framebuffer_height * GLOBAL_CTX.fb->framebuffer_width;
	uint32_t packed_rgb = PackRgb(rgb, GLOBAL_CTX.fb);

	for(int i = 0; i < num_pixels; ++i) {
		*(GLOBAL_CTX.buffer + i) = packed_rgb;
	}
	GLOBAL_CTX.dirty_block_str = 0xFFFFFFFFFFFFFFFF;
}

void WriteBack()
{
	for(int x = 0; x < GLOBAL_CTX.num_rows; ++x) {
		if(GetNthBit(GLOBAL_CTX.dirty_block_str, x)) {
			WriteBackCell(x);
		} 
		//WriteBackCell(x);
	}
}

void WriteBackCell(uint8_t cell_ind)
{
	int buff_index = cell_ind * GLOBAL_CTX.row_height * GLOBAL_CTX.fb->framebuffer_width;
	int screen_index = cell_ind * GLOBAL_CTX.row_height * GLOBAL_CTX.fb->framebuffer_pitch;

	for(int pixel_row = 0; pixel_row < GLOBAL_CTX.row_height; ++pixel_row) {
		void *dest = (void *) GLOBAL_CTX.fb->framebuffer_addr + screen_index;
		uint32_t *src = GLOBAL_CTX.buffer + buff_index;
		size_t size = GLOBAL_CTX.fb->framebuffer_width * GLOBAL_CTX.fb->framebuffer_bpp / 8;
		memmove(dest, src, size);

		buff_index += GLOBAL_CTX.fb->framebuffer_width;
		screen_index += GLOBAL_CTX.fb->framebuffer_pitch;
	}
	
	ClearNthBit(&GLOBAL_CTX.dirty_block_str, cell_ind);
}

void DrawRect(Coordinate coords, Dimensions dims, RGB rgb)
{
	uint32_t packed_rgb = PackRgb(rgb, GLOBAL_CTX.fb);	
	uint32_t *dest = GLOBAL_CTX.buffer + PixelIndex(coords);
	
	// Fill in the first row.
	for(int pixel = 0; pixel < dims.width; ++pixel) {
		*(dest+pixel) = packed_rgb;
	}
	
	// And now, since all subsequent rows are identical, just memmove that row
	// to all subsequent rows.
	// Is this method actually more efficient than pixel-by-pixel? Would the'
	
	size_t bytes_per_rect_row = dims.width * GLOBAL_CTX.fb->framebuffer_bpp / 8;
	uint32_t *src = dest;
	for(int row = 1; row < dims.height; ++row) {
		dest += GLOBAL_CTX.fb->framebuffer_width;
		memmove(dest, src, bytes_per_rect_row);
	}
	
	//uint32_t *dest = GLOBAL_CTX.buffer + PixelIndex(ctx, coords);
	//for(int i = 0; i < dims.height; ++i) {
	//	for(int j = 0; j < dims.width; ++j) {
	//		*(where + j) = packed_rgb;
	//	}
	//	where += GLOBAL_CTX.fb->framebuffer_width;
	//}

	int starting_row = coords.y / GLOBAL_CTX.row_height;
	int ending_row = (coords.y + dims.height) / GLOBAL_CTX.row_height;
	ending_row += (coords.y % GLOBAL_CTX.row_height) == 0;
	for(int i = starting_row; i <= ending_row; ++i) {
		SetNthBit(&GLOBAL_CTX.dirty_block_str, i);
	}
}

void PrintStr(Font *font, Coordinate coords, char* str)
{
	int x = coords.x, y = coords.y;
	char c;
	// Iterate through chars, terminate at /0.
	for(int k = 0; (c = *(str + k)) != 0; ++k) {
		bool exceeds_fb_width = (coords.x + x) > GLOBAL_CTX.fb->framebuffer_width;
		if(c == '\n' || exceeds_fb_width) {
			x = coords.x;
			y += font->height;
		} else {
			x += font->width;
			Coordinate new_coords = {coords.x + x, coords.y + y};
			DrawChar(font, new_coords, c);
		}

	}
}

void DrawChar(Font *font, Coordinate coords, char c)
{
	const uint8_t *char_bmp = font->matrix[(int) c];
	uint32_t packed_rgb = PackRgb(font->rgb, GLOBAL_CTX.fb);

	for(int j = 0; j < font->height; ++j) {
		const uint8_t row = char_bmp[j];
		for(int i = 0; i < font->width; ++i) {
			if(GetNthBit(row, i)) {
				int index = coords.x + i + (coords.y + j) * GLOBAL_CTX.fb->framebuffer_width;
				*((uint32_t*) GLOBAL_CTX.buffer + index) = packed_rgb;
			}
		}
	}
	
	int starting_row = coords.y / GLOBAL_CTX.row_height;
	int ending_row = (coords.y + font->height) / GLOBAL_CTX.row_height;
	ending_row += 1 * (((coords.y + font->height) % GLOBAL_CTX.row_height) > 0);
	for(int i = starting_row; i < ending_row; ++i) {
		SetNthBit(&GLOBAL_CTX.dirty_block_str, i);
	}
}

void Transpose(Coordinate top_left, Dimensions dims, int down, int left)
{
	// Transpose each individual row of the requested rectangle.
	for(int y = top_left.y; y < top_left.y + dims.height; ++y) {
		// Where are we transposing from?
		Coordinate src_coords = { 
			.x = top_left.x, 
			.y = y 
		};
		uint32_t *src = GLOBAL_CTX.buffer + PixelIndex(src_coords);
		// Where are we transposing to?
		Coordinate dest_coords = { 
			.x = top_left.x + left,
			.y = y + down
		};
		uint32_t *dest = GLOBAL_CTX.buffer + PixelIndex(dest_coords);
		// 4 bytes per pixel.
		size_t num_bytes_to_move = dims.width * GLOBAL_CTX.fb->framebuffer_bpp / 8;
		memmove(dest, src, num_bytes_to_move);
	}
	
	// We must now invalidate the range of rows which were transposed from,
	// transposed to, or in between.
	int top_row 	= down > 0 ? top_left.y : top_left.y + down;
	int bottom_row 	= down > 0 ? top_left.y + dims.height + down :
								 top_left.y + dims.height;

	int top_invalid_row 	= top_row / GLOBAL_CTX.row_height;
	int bottom_invalid_row 	= bottom_row / GLOBAL_CTX.row_height;
	// Poor man's ceil. 
	bottom_invalid_row += ((bottom_row % GLOBAL_CTX.row_height) != 0);

	// Note that, since y increases as we go *down* the screen, top_invalid_row
	// will be less than bottom_invalid_row.
	for(int i = top_invalid_row; i < bottom_invalid_row; ++i) {
		SetNthBit(&GLOBAL_CTX.dirty_block_str, i);
	}
}

int PixelIndex(Coordinate coords)
{
	return coords.x + coords.y * GLOBAL_CTX.fb->framebuffer_width;
}
