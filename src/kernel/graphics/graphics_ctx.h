#ifndef GRAPHICS_CTX_H
#define GRAPHICS_CTX_H

#include "stivale2.h"
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
void InitGraphicsCtx(struct stivale2_struct_tag_framebuffer  *fb);

/**
 * Clear all pixels in screen to certain color.
 * @input ctx The context containing the framebuffer to clear.
 * @input rgb The color to which we should clear the screen.
 */
void ClearScreen(RGB rgb);

/**
 * Write the dirty rectangles of the back buffer to video memory.
 * @input ctx The graphics context containing the buffer to write back.
 */
void WriteBack();

/**
 * Write back a given cell from the back buffer to video memory.
 * @input ctx The graphics context containing the back buffer.
 * @input ind The index [0,64) that identifies the cell to write back.
 */
void WriteBackCell(uint8_t ind);

/**
 * Draw a rectangle to the back buffer of a given screen.
 * @input ctx The graphics context to whose back buffer the rect will be 
 *            written.
 * @input coord The coordinates of the rectangle's top left corner.
 * @input dim The dimensions of the rectangle.
 * @input rgb The color of the rectangle.
 */
void DrawRect(Coordinate coord, Dimensions dim, RGB rgb);

/**
 * Draw character c in the given font at the given coordinates.
 */
void DrawChar(Font *font, Coordinate coord, char c);

/**
 * Print the given string at the desired location in a graphics context's back
 * buffer.
 * @input ctx The context to whose back buffer the screen will be written.
 * @input font A font object containing a character bitmap.
 * @input coord The top left corner of the space containing the screen.
 * @input str The string to be printed.
 */
void PrintStr(Font *font, Coordinate coord, char *str);

/**
 * Take the pixels in the rectangle with the specified top left corner and the
 * specified dimensions, shift them upwards by the specified num pixels and
 * leftwards by the speicified num pixels.
 * @input ctx The graphics context on which this operation is to be performed.
 * @input top_left The top left of the rectangle to transpose.
 * @input dims The dimensions of rectangle to transpose.
 * @input down The number of pixels downwards (negative for upwards) to 
 *			   transpose the pixels.
 * @input left The number of pixels leftwards (negative for rightwards) to tran-
 *			   spose the specified rectangle.
 */
void Transpose(Coordinate top_left, Dimensions dim, int down, int left);

/**
 * Calculate the index in the framebuffer of the pixel identified by the given
 * coordinates.
 * @input ctx The graphics ctx whose framebuffer will be searched.
 * @input coords The coordinate whose index we seek to identify.
 * @output The offset from the beginning of ctx's framebuffer of the given
 *		   pixel.
 */
int PixelIndex(Coordinate coords);

#endif
