symbol-file kernel.elf
target remote | qemu-system-x86_64 bin/image.iso -S -gdb stdio -smp 2
layout src

set tui tab-width 4

define q
	kill inferiors 1
	quit
end
