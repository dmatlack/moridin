#
#
# Operating System Makefile
#
#
.PHONY: default kernel user tools iso clean

default: iso

# 
# Specify ARCH=<architecture> on the commandline invocation in order to
# compile for a specific architecture.
#
# Supported values of ARCH:
# 	ARCH_X86 (386, 486, 586 intel cpus)
#
ARCH:=ARCH_X86

MAKE = make ARCH=$(ARCH) -C

#
# Create an .iso (CDROM image) that can boot the os
#
iso: kernel user initrd boot/grub.cfg 
	mkdir -p iso
	mkdir -p iso/boot
	mkdir -p iso/boot/grub
	cp kernel/KERNEL.o 	iso/boot/KERNEL.o
	cp boot/grub.cfg 	iso/boot/grub/grub.cfg
	cp initrd.img 		iso/boot/
	grub-mkrescue -o OS.iso iso

#
# Build the kernel sources into an object file
#
kernel:
	$(MAKE) kernel/

#
# Build the user progams into executables compatable with kernel
#
user:
	$(MAKE) user/

#
# Create the initial ramdisk needed to use the operating system
#
initrd: tools
	tools/create_initrd initrd.img user/bin/* boot/initrd/* 

#
# Build the tools used to build the kernel and os
#
tools:
	$(MAKE) tools/

clean:
	make -C kernel/ clean
	make -C user/ clean
	make -C tools/ clean
	rm  -f initrd.img
	rm -rf iso
	rm -rf OS.iso
