; Loads the 64 bit gdt
bits 64

global load_gdt
load_gdt:
    lgdt [rdi]
    push 0x28
    lea rax, [rel .flush]
    push rax
    retfq

    .flush:
        mov ax, 0x30
        mov ds, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        mov es, ax
        ret
