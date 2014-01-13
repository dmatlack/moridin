.PHONY: all initrd

all: initrd

initrd: create_initrd.c
	gcc -std=c99 create_initrd.c -o create_initrd

clean:
	rm -f create_initrd
