#!/bin/bash

qemu-system-i386 -m 1024 OS.iso -serial stdio -display none -enable-kvm
