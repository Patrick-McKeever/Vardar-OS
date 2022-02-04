//#ifndef TERMINAL_H
//#define TERMINAL_H
//
//#include "graphics_ctx.h"
//
//typedef struct {
//	GraphicsCtx *parent_ctx;
//	Dimensions dims;
//	Font *font; 
//	uint16_t chars_per_line, num_lines;
//	Coordinate top_left, cursor;
//	RGB bg_color, border_color;
//	uint8_t border_thickness;
//	char *prompt;
//	// malloc/realloc this once we implement MM.
//	char text[114][48];
//} Terminal;
//
//Terminal InitTerminal(GraphicsCtx *parent_ctx, Dimensions dims, 
//					  Coordinate top_left, Font *font, 
//					  RGB bg, RGB border, uint8_t thickness,
//					  char *prompt)
//{
//	
//	Terminal term = {
//		.parent_ctx = parent_ctx,
//		.dims = dims,
//		.font = font,
//		.chars_per_line = dims.width / font.width,
//		.num_lines = dims.height / font.height, 
//		.top_left = top_left,
//		.cursor = (Coordinate) {term->border_thickness, 0},
//		.bg_color = bg,
//		.border_color = border,
//		.border_thickness = thickness,	
//		.prompt = prompt
//	};
//	Render(&term);
//	return term;
//}
//
//void TermPrint(Terminal *term, const char *str)
//{
//	int x = term->cursor.x, y = term->cursor.y;
//	char c;
//	for(int k = 0; (c = *(str + k)) != 0; ++k) {
//		bool exceeds_term_width = (term->cursor.x + x) > term->dims.width;
//		if(c == '\n' || exceeds_term_width) {
//			x = coords.x;
//			y += term->font.height;
//		} else {
//			x += term->font.width;
//			Coordinate newCoords = {coords.x + x, coords.y + y};
//			DrawChar(ctx, term->font, newCoords, c);
//		}
//	}
//}
//
//void Render(Terminal *term)
//{
//	// Draw terminal body, parts of which will be overwrriten by borders.
//	DrawRect(term->parent_ctx, term->top_left, term->dims, term->bg_color);
//	RenderBorders(term);
//		
//}
//
//void RenderBorders(Terminal *term)
//{
//	// Top border.
//	DrawRect(term->parent_ctx, term->top_left, 
//			(Dimensions) {term->dims.width, term->border_thickness}, 
//			term->border_color);
//
//	// Left border.
//	DrawRect(term->parent_ctx, term->top_left, 
//			 (Dimensions) {term->border_thickness, term->dims.height}, 
//			 term->border_color);
//
//	// Bottom border.	
//	Coordinate bottom_left = {
//		.x = term->top_left.x,
//		.y = term->top_left.y + terms->dims.height - term->border_thickness 
//	};
//	DrawRect(term->parent_ctx, bottom_left, 
//			 (Dimensions) {term->dims.width, term->border_thickness}, 
//			 term->border_color);
//	
//	// Right border.
//	Coordinate top_right = {
//		.x = term->top_left.x + term->dims.width - term->border_thickness,
//		.y = term->top_left.y 
//	};
//	DrawRect(term->parent_ctx, top_right,
//			 (Dimensions) {term->border_thickness, term->dims.height},
//			 term->border_color);
//
//
//}
//
//#endif
