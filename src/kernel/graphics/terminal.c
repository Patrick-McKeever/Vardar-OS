#include "terminal.h"
#include <stdbool.h>

static Terminal main_term;
Terminal InitTerminal(GraphicsCtx *parent_ctx, Dimensions dims, 
					  Coordinate top_left, Font *font, 
					  RGB bg, RGB border, uint8_t thickness,
					  char *prompt)
{	
	Terminal term = {
		.parent_ctx = parent_ctx,
		.dims = {dims.width - thickness * 2, dims.height - thickness * 2},
		.font = font,
		.chars_per_line = dims.width / font->width,
		.num_lines = dims.height / font->height, 
		.top_left = {top_left.x + thickness, top_left.y + thickness},
		.cursor = (Coordinate) {top_left.x + thickness, 0},
		.bg_color = bg,
		.border_color = border,
		.border_thickness = thickness,	
		.prompt = prompt
	};
	main_term = term;
	Render(&main_term);
	return term;
}

void TermPrint(Terminal *term, const char *str)
{
	char c;
	for(int k = 0; (c = *(str + k)) != 0; ++k) {
		// Leave a little room on the right side of the terminal.
		int max_y = term->dims.width - term->border_thickness * 5;
		bool exceeds_term_width = term->cursor.x > max_y;
		if(c == '\n' || exceeds_term_width) {
			term->cursor.x = 0;
			term->cursor.y += term->font->height;

			if(term->cursor.y > term->dims.height) {
				Scroll(term);
			}
		} else {
			term->cursor.x += term->font->width;
		}
		
		Coordinate char_coords = {
			.x = term->top_left.x + term->cursor.x,
			.y = term->top_left.y + term->cursor.y
		};
		DrawChar(term->parent_ctx, term->font, char_coords, c);		
	}
}

void Render(Terminal *term)
{
	// Draw terminal body, parts of which will be overwrriten by borders.
	RenderBorders(term);
	DrawRect(term->parent_ctx, term->top_left, term->dims, term->bg_color);
	TermPrint(term, term->prompt);
}

void Scroll(Terminal *term)
{
	for(int i = 1; i < term->num_lines; ++i) {
		Coordinate row_top_left = {
			.x = term->top_left.x,
			.y = i * term->font->height + term->top_left.y,
		};
		
		Dimensions row_dims = {
			.width = term->dims.width,
			.height = term->font->height
		};

		Transpose(term->parent_ctx, row_top_left, row_dims, 
				  term->font->height, 0);
	}
	WriteBack(term->parent_ctx);
}

void RenderBorders(Terminal *term)
{
	// Why is this necessary? Because term's "top left" and dimensions are
	// the top left and dimensions of the *main body* of the terminal - i.e.
	// excluding the borders.
	Coordinate top_left = {
		.x = term->top_left.x - term->border_thickness,
		.y = term->top_left.y - term->border_thickness
	};
	Dimensions dims = {
		.width = term->dims.width + term->border_thickness * 2,
		.height = term->dims.height + term->border_thickness * 2
	};
	DrawRect(term->parent_ctx, top_left, dims, term->border_color);
}

void HandleKeyStroke(KeyInfo *key_info)
{
	char c[2];
	c[0] = CharFromScancode(key_info);
	c[1] = 0;
	TermPrint(&main_term, c);
	WriteBack(main_term.parent_ctx);
}
