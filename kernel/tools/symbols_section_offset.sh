#!/bin/bash

set -e

[ $1 ] || exit 42

file=$1

offset=$(readelf -S $file \
	| grep \.symbols \
	| sed -e 's/\[\s\+[0-9]\+\]/ /g' \
	| sed -e 's/\s\+/ /g' \
	| cut -d" "  -f5)

echo 0x${offset}
