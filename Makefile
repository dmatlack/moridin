#
#
# Operating System Makefile
#
#
.PHONY: default kernel user tools iso clean

default: iso

#
# Create an .iso (CDROM image) that can boot the os
#
iso: kernel user initrd boot/grub.cfg 
	mkdir -p iso
	mkdir -p iso/boot
	cp kernel/KERNEL.o iso/boot/KERNEL.o
	mkdir -p iso/boot/grub
	cp boot/grub.cfg iso/boot/grub/grub.cfg
	mv initrd iso/boot/
	grub-mkrescue -o OS.iso iso

#
# Build the kernel sources into an object file
#
kernel:
	make -C kernel/

#
# Build the user progams into executables compatable with kernel
#
user:
	make -C user/

initrd: tools
	tools/create_initrd user/bin/*

tools:
	make -C tools/

clean:
	make -C kernel/ clean
	make -C user/ clean
	make -C tools/ clean
	rm -rf iso
	rm -rf OS.iso
