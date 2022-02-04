#ifndef KEYCODES_H
#define KEYCODES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define BACKSPACE_PRESSED		0x0E
#define BACKSPACE_RELEASED		0x8E
#define LEFT_SHIFT_PRESSED		0x2A
#define LEFT_SHIFT_RELEASED		0xAA
#define RIGHT_SHIFT_PRESSED		0x36
#define RIGHT_SHIFT_RELEASED	0xB6
#define CTRL_PRESSED			0x1E
#define CTRL_RELEASED			0x9E

// Struct to provide information about the current keyboard state.
typedef struct {
	bool shift, backspace, ctrl;
	uint8_t scancode;
} KeyInfo;

/**
 * Return the character associated with the current key state.
 * @input key_info The current key state, including shift, backspace, ctrl vals.
 * @output The ASCII character that the user has typed, 0 if the char does not
 		   correspond to a particular ASCII char (e.g. if they pressed CTRL).
 */
char CharFromScancode(KeyInfo *key_info);

// This is how we're going to handle keystroke consumption for now. The 
// currently running process will be required to set a function pointer as
// the keystroke consumer at all times, and the ISR1 handler will pass KeyInfo
// to the consumer function. This allows the currently dominant GUI process to
// determine how keystrokes are handled at any given time.
typedef void(*KeystrokeConsumer)(KeyInfo*);

/**
 * Retrieve the value of the keystroke consumer function, declared as a static
 * var in the corresponding "keycodes.c" file.
 * @output Function ptr to current keystroke consumer function.
 */
KeystrokeConsumer GetKeystrokeConsumer();

/**
 * Set the current keystroke consumer function equal to the function ptr passed
 * in.
 * @input ksc The function pointer to be called in order to handle a keystroke.
 */
void SetKeystrokeConsumer(KeystrokeConsumer ksc);

#endif

