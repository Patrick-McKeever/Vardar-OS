#ifndef GRAPHICS_CTX_H
#define GRAPHICS_CTX_H

#include "../stivale2.h"
#include "graphics_types.h"
#include "font.h"

typedef struct {
	// Provides info about VBE mode and a ptr to a linear framebuffer.
	struct stivale2_struct_tag_framebuffer *fb;
	// The nth bit of this str provides bool value of whether nth row is dirty.
	uint64_t dirty_block_str;
	// The back buffer in our double-buffering scheme, segmented into 64 rows. 
	uint32_t *buffer;
	// Number, dimensions of rows and columns in the "grid" of rectangles.
	uint8_t num_rows, row_height;
} GraphicsCtx;

/**
 * Provide a graphics context with an empty screen
 */
GraphicsCtx InitGraphicsCtx(struct stivale2_struct_tag_framebuffer  *fb);

/**
 * Write the dirty rectangles of the back buffer to video memory.
 * @input ctx The graphics context containing the buffer to write back.
 */
void WriteBack(GraphicsCtx *ctx);

/**
 * Write back a given cell from the back buffer to video memory.
 * @input ctx The graphics context containing the back buffer.
 * @input ind The index [0,64) that identifies the cell to write back.
 */
void WriteBackCell(GraphicsCtx *ctx, uint8_t ind);

/**
 * Draw a rectangle to the back buffer of a given screen.
 * @input ctx The graphics context to whose back buffer the rect will be 
 *            written.
 * @input coord The coordinates of the rectangle's top left corner.
 * @input dim The dimensions of the rectangle.
 * @input rgb The color of the rectangle.
 */
void DrawRect(GraphicsCtx *ctx, Coordinate coord, Dimensions dim, RGB rgb);

/**
 * Draw character c in the given font at the given coordinates.
 */
void DrawChar(GraphicsCtx *ctx, Font *font, Coordinate coord, char c);

/**
 * Print the given string at the desired location in a graphics context's back
 * buffer.
 * @input ctx The context to whose back buffer the screen will be written.
 * @input font A font object containing a character bitmap.
 * @input coord The top left corner of the space containing the screen.
 * @input str The string to be printed.
 */
void PrintStr(GraphicsCtx *ctx, Font *font, Coordinate coord, char *str);

#endif
