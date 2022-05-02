#ifndef TERMINAL_H
#define TERMINAL_H

#include "graphics_ctx.h"
#include "interrupts/keycodes.h"

typedef struct {
	Dimensions dims;
	Font *font; 
	uint16_t chars_per_line, num_lines;
	Coordinate top_left, cursor;
	RGB bg_color, border_color;
	uint8_t border_thickness;
	char *prompt;
	// malloc/realloc this once we implement MM.
} Terminal;

Terminal InitTerminal(Dimensions dims, 
					  Coordinate top_left, Font *font, 
					  RGB bg, RGB border, uint8_t thickness,
					  char *prompt);

void TermPrint(Terminal *term, const char *str);
void TermPrintMain(const char *str);
void RenderMain();

void Render(Terminal *term);

void Scroll(Terminal *term);

void RenderBorders(Terminal *term);

void HandleKeyStroke(KeyInfo *key_info);
#endif
