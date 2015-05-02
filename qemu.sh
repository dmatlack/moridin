#!/bin/bash

qemu-system-i386 -m 1024 -serial stdio -display none -enable-kvm -cdrom OS.iso
