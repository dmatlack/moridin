#
# Bare Bones Tutorial Makefil
#

default: boot kernel
	i586-elf-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc
	mkdir -p isodir
	mkdir -p isodir/boot
	cp myos.bin isodir/boot/myos.bin
	mkdir -p isodir/boot/grub
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir
	
boot: boot.s
	i586-elf-as boot.s -o boot.o

kernel: kernel.c
	i586-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

clean:
	rm -rf boot.o
	rm -rf kernel.o
	rm -rf myos.bin
	rm -rf myos.iso
	rm -rf isodir/
