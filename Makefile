#
#
# Operating System Makefile
#
#
.PHONY: default kernel iso clean

default: iso

#
# Create an .iso (CDROM image) that can boot the os
#
iso: kernel boot/grub.cfg
	mkdir -p iso
	mkdir -p iso/boot
	cp kernel/KERNEL.o iso/boot/KERNEL.o
	mkdir -p iso/boot/grub
	cp boot/grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o OS.iso iso

#
# Build the kernel sources into an object file
#
kernel:
	make -C kernel/


clean:
	make -C kernel/ clean
	rm -rf iso
	rm -rf OS.iso
