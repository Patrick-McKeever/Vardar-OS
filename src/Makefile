CC := /usr/local/x86_64-elfgcc/bin/x86_64-elf-gcc
#CC := gcc
#LD := ld
AS := nasm
LD := /usr/local/x86_64-elfgcc/bin/x86_64-elf-ld 

CFLAGS := 				 	\
	-I ./kernel/		 	\
	-std=gnu11			 	\
	-O0	-pipe -g		 	\
	-ffreestanding       	\
	-fno-stack-protector 	\
	-fno-pic             	\
	-m64					\
	-mno-80387           	\
	-mno-mmx             	\
	-mno-3dnow           	\
	-fpie				 	\
	-lgcc				 	\
	-mno-sse			 	\
	-mno-sse2			 	\
	-Wall				 	\
	-fno-omit-frame-pointer \
	-mno-red-zone

ASFLAGS := -felf64
LDFLAGS :=  -Tlinker.ld -nostdlib 
XORISSOFLAGS := -as mkisofs -b limine-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot limine-eltorito-efi.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label
QEMUFLAGS := -m 2G -d int -M smm=off -D ./log.txt -smp 2


CFILES := $(shell find ./ -type f -name '*.c')
ASMFILES := $(shell find ./ -type f -name '*.asm')
OBJ_C := $(CFILES:.c=.o)
OBJ_ASM := $(ASMFILES:.asm=.o)
OBJ := $(OBJ_C) $(OBJ_ASM)

KERNEL := kernel.elf
.PHONY: all clean

all: $(KERNEL)

$(KERNEL): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

run:
	cp -v kernel.elf limine.cfg iso_root/
	xorriso $(XORISSOFLAGS) iso_root -o bin/image.iso
	../limine/limine-install bin/image.iso
	qemu-system-x86_64 -drive format=raw,file=bin/image.iso $(QEMUFLAGS)
	
run-monitor:
	cp -v kernel.elf limine.cfg iso_root/
	xorriso $(XORISSOFLAGS) iso_root -o bin/image.iso
	../limine/limine-install bin/image.iso
	qemu-system-x86_64 -drive format=raw,file=bin/image.iso -monitor stdio $(QEMUFLAGS)

run-no-shutdown:
	cp -v kernel.elf limine.cfg iso_root/
	xorriso $(XORISSOFLAGS) iso_root -o bin/image.iso
	../limine/limine-install bin/image.iso
	qemu-system-x86_64 -drive format=raw,file=bin/image.iso -no-reboot -no-shutdown $(QEMUFLAGS) 

debug:
	cp -v kernel.elf limine.cfg iso_root/                                       
	xorriso $(XORISSOFLAGS) iso_root -o bin/image.iso 
	../limine/limine-install bin/image.iso
	gdb --command=debug.gdb

clean:
	find -type f -name "*.o" -delete
	find -type f -name "*.elf" -delete
