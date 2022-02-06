symbol-file kernel.elf
target remote | qemu-system-x86_64 bin/image.iso -S -gdb stdio
layout src
