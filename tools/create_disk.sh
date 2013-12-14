#! /bin/bash

# Make sure to change bochsrc if you change these values
cylinders=306
heads=4
spt=17

sectors="$(expr $cylinders \* $heads \* $spt)"

dd if=/dev/zero of=DISK.$cylinders.$heads.$spt bs=512 count=$sectors
