#include "utils/printf.h"

// Needless to say, this is not a faithful implementation of the C stdlib's
// printf. I treat all ints as quadwords and leave out a bunch of formats,
// and also don't provide floating point support yet. I will eventually edit
// the code to provide a faithful implementation, but, for now, this is more
// than sufficient.
void PrintK(char *str,...)
{
	char *traverse;
	char *s;
	uint64_t i;	
	va_list arg;
	va_start(arg, str);

	for(traverse = str; *traverse != '\0'; ++traverse) {
		do {
			if(*traverse == '\0')
				return;
			PutChar(*traverse);
		} while(*(++traverse) != '%'); 

		char data_type = *(++traverse);
		switch(data_type) {
			case 'c':
				i = va_arg(arg, int);
				PutChar(i);
				break;
	
			case 'd':
				i = va_arg(arg, uint64_t);
				if(i < 0)
					PutChar('-');
				Puts(Convert(i, 10));
				break;
				
			case 'o':
				i = va_arg(arg, uint64_t);
				Puts(Convert(i, 8));
				break;

			case 'h':
				i = va_arg(arg, uint64_t);
				Puts(Convert(i, 16));
				break;

			case 's':
				s = va_arg(arg, char*);
				Puts(s);
				break;
		}
	}
	va_end(arg);	
}

char *Convert(uint64_t num, int base)
{
	static char hex_digits[] = "0123456789ABCDEF";
	static char buffer[32];
	char *ptr = &buffer[31];
	*ptr = '\0';

	do {
		*(--ptr) = hex_digits[num % base];
		num /= base;
	} while(num != 0);

	return ptr;
}
