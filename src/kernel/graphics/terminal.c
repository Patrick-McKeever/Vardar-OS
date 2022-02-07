#include "terminal.h"
#include <stdbool.h>

static Terminal main_term;
static void RemoveChar();
static void TermNewLine(Terminal *term);

Terminal InitTerminal(Dimensions dims, 
					  Coordinate top_left, Font *font, 
					  RGB bg, RGB border, uint8_t thickness,
					  char *prompt)
{	
	Terminal term = {
		.dims = {dims.width - thickness * 2, dims.height - thickness * 2},
		.font = font,
		.chars_per_line = dims.width / font->width,
		.num_lines = dims.height / font->height, 
		.top_left = {top_left.x + thickness, top_left.y + thickness},
		.cursor = (Coordinate) {0, 0},
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
		int max_y = term->dims.width - term->border_thickness * 6;
		bool exceeds_term_width = term->cursor.x > max_y;
		if(c == '\n' || exceeds_term_width) {
			TermNewLine(term);
		} else {
			term->cursor.x += term->font->width;
		}	
		
		Coordinate char_coords = {
			.x = term->top_left.x + term->cursor.x,
			.y = term->top_left.y + term->cursor.y
		};
		DrawChar(term->font, char_coords, c);		
	}
}

void Render(Terminal *term)
{
	// Draw terminal body, parts of which will be overwrriten by borders.
	RenderBorders(term);
	DrawRect(term->top_left, term->dims, term->bg_color);
	TermPrint(term, term->prompt);
}

void Scroll(Terminal *term)
{
	for(int i = 1; i <= main_term.num_lines; ++i) {
		Coordinate row_top_left = {
			.x = main_term.top_left.x,
			.y = i * main_term.font->height + main_term.top_left.y,
		};
		
		Dimensions row_dims = {
			.width = main_term.dims.width,
			.height = main_term.font->height
		};

		Transpose(row_top_left, row_dims, -1 * main_term.font->height, 0);
		WriteBack();
	}
	
	Coordinate final_rect_top_left = {
		.x = main_term.top_left.x,
		.y = main_term.top_left.y + main_term.font->height * (main_term.num_lines - 1)
	};
	Dimensions final_rect_dims = {
		.width = main_term.dims.width,
		.height = main_term.font->height 
	};
	DrawRect(final_rect_top_left, final_rect_dims, term->bg_color);
	WriteBack();
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
	DrawRect(top_left, dims, term->border_color);
}

void HandleKeyStroke(KeyInfo *key_info)
{
	char c[2];
	c[0] = CharFromScancode(key_info);
	c[1] = 0;
	
	if(key_info->backspace) {
		RemoveChar();
	} else if(key_info->enter) {
		TermNewLine(&main_term);
		TermPrint(&main_term, main_term.prompt);
	} else {
		TermPrint(&main_term, c);
	}
	WriteBack();
}

static void RemoveChar()
{
	if(main_term.cursor.x == 0 && main_term.cursor.y == 0)
		return;
	
	int char_width 	= main_term.font->width,
		char_height = main_term.font->height;
	Coordinate char_coords = {
		.x = main_term.top_left.x + main_term.cursor.x,
		.y = main_term.top_left.y + main_term.cursor.y
	};
	DrawRect(char_coords, (Dimensions){char_width, char_height}, 
			 main_term.bg_color);
	if(main_term.cursor.x == 0) {
		main_term.cursor.x = (main_term.chars_per_line - 1) * char_width;
		main_term.cursor.y -= char_height;
	} else {
		main_term.cursor.x -= char_width; 
	}
	
}

static void TermNewLine(Terminal *term)
{
	term->cursor.x = 0;
	if((term->cursor.y + term->font->height * 2) > term->dims.height) {
		Scroll(term);
	} else {
		term->cursor.y += term->font->height;
	}
}
