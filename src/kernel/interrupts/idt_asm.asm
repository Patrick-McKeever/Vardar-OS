%macro PUSHALL_NO_RAX 0
	push rbx
	push rcx
	push rdx
	push rdi
	push rsi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro POPALL_NO_RAX 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx
%endmacro

%macro PUSHALL 0
	push rax
	PUSHALL_NO_RAX
%endmacro

%macro POPALL 0
	POPALL_NO_RAX
	pop rax
%endmacro

GLOBAL isr1
[extern Isr1Handler]
isr1:
	PUSHALL
	call Isr1Handler
	POPALL
	iretq

GLOBAL isr2
[extern Isr2Handler]
isr2:
	PUSHALL
	call Isr2Handler
	POPALL
	iretq

GLOBAL isr80
[extern Isr80Handler]
; Syscall. 
isr80:
	; This PUSHALL will be popped inside SAVE_REGISTERS.
	PUSHALL

	; RSI (second arg to Isr80Handler) should point to stack before pushing
	; of all operands.
	lea rsi, [rsp+120]
	mov rdi, rsp

	mov ax, 0x30
	mov ds, ax
	mov fs, ax
	mov ss, ax
	mov gs, ax
	mov es, ax

	call Isr80Handler

	POPALL
	iretq

GLOBAL LoadIdt 
; Loads the IDT from the first parameter passed to it.
; @input rdi The address of the IDT.
LoadIdt:
	lidt [rdi]
	sti	
	ret
