
%macro PUSHALL 0
	push rax
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11
%endmacro

%macro POPALL 0
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdx
	pop rcx
	pop rax
%endmacro

GLOBAL isr1
[extern Isr1Handler]
isr1:
	PUSHALL
	call Isr1Handler
	POPALL
	iretq

GLOBAL LoadIdt 
; Loads the IDT from the first parameter passed to it.
; @input rdi The address of the IDT.
LoadIdt:
	lidt [rdi]
	sti	
	ret
