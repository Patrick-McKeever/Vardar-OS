C := ../crosstools/x86_64-elf-gcc
AS := nasm
LD := ../crosstools/x86_64-elf-ld

CFLAGS := 				 	\
	-std=gnu11			 	\
	-fms-extensions			\
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


KERN_CFILES := $(shell find ./kernel -type f -name '*.c')
KERN_ASMFILES := $(shell find ./kernel -type f -name '*.asm')
KERN_OBJ_C := $(KERN_CFILES:.c=.o)
KERN_OBJ_ASM := $(KERN_ASMFILES:.asm=.o)
KERN_OBJ := $(KERN_OBJ_C) $(KERN_OBJ_ASM)

USER_CFILES := $(shell find ./userspace -type f -name '*.c')
USER_ASMFILES := $(shell find ./userspace -type f -name '*.asm')
USER_OBJ_C := $(USER_CFILES:.c=.elf)
USER_OBJ_ASM := $(USER_ASMFILES:.asm=.elf)
USER_OBJ := $(USER_OBJ_C) $(USER_OBJ_ASM)

KERNEL := kernel.elf
.PHONY: all clean

all: $(KERNEL) $(USER_OBJ)

$(KERNEL): $(KERN_OBJ)
	$(LD) $(LDFLAGS) $(KERN_OBJ) -o $@

$(USERSPACE): $(USER_OBJ)

# These .o rules should only be called for kernel binaries.
%.o: %.c
	$(CC) $(CFLAGS) -I ./kernel/ -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# These elf rules will only be called for userspace binaries.
%.elf: %.c
	$(CC) $(CFLAGS) $< -o $@

%.elf: %.asm
	$(eval INP_NO_EXT := $(basename $<))
	$(AS) $(ASFLAGS) $< -o "$(INP_NO_EXT).o"
	$(LD) "$(INP_NO_EXT).o" -o $@

INITRD := userspace/initrd.ustar
USERSPACE_ELFS := $(shell find ./userspace -type f -name '*.elf' | xargs echo)

run:
	tar --create --file $(INITRD) $(USERSPACE_ELFS)
	cp -v kernel.elf $(INITRD) limine.cfg iso_root/
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
