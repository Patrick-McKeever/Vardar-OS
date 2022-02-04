#include "keycodes.h"

static const char LOWERCASE_CHARS_BY_SCANCODE[128] = {
    0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,
    0,'q','w','e','r','t','y','u','i','o','p','[',']',0,0,
    'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
    'z','x','c','v','b','n','m',',','.','/',0,0,0,' ',0
};

static const char UPPERCASE_CHARS_BY_SCANCODE[128] = {
        0,0,'!','@','#','$','%','^','&','*','(',')','_','+',0,
        0,'Q','W','E','R','T','Y','U','I','O','P','{','}',0,0,
        'A','S','D','F','G','H','J','K','L',':','"','~',0,'|',
        'Z','X','C','V','B','N','M','<','>','?',0,0,0,' ',0
};

// The function that will be called inside a keystroke handler so that a GUI
// application may handle a keystroke (e.g. display it to screen, interpret it,
// etc.).
static KeystrokeConsumer KEYSTROKE_CONSUMER;

char CharFromScancode(KeyInfo *key_info)
{
	if(key_info->shift)
		return UPPERCASE_CHARS_BY_SCANCODE[key_info->scancode];
	return LOWERCASE_CHARS_BY_SCANCODE[key_info->scancode];
}

KeystrokeConsumer GetKeystrokeConsumer()
{
	return KEYSTROKE_CONSUMER;
}

void SetKeystrokeConsumer(KeystrokeConsumer ksc)
{
	KEYSTROKE_CONSUMER = ksc;
}
