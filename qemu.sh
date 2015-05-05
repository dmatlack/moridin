#!/bin/bash

qemu-system-i386 -m 1024 -serial stdio -display none -cdrom OS.iso -enable-kvm
