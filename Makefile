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
	mkdir -p isodir
	mkdir -p isodir/boot
	cp kernel/KERNEL.o isodir/boot/KERNEL.o
	mkdir -p isodir/boot/grub
	cp boot/grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o OS.iso isodir

#
# Build the kernel sources into an object file
#
kernel:
	make -C kernel/


clean:
	make -C kernel/ clean
	rm -rf isodir
	rm -rf OS.iso
