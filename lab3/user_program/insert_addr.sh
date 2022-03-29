#!/bin/bash
if [[ "$2" == "" ]]; then
    echo "Usage $0 <hex_begin_addr> <input filename> <output filename>"
    exit 1
fi
hex_addr="$1"
filename="$2"
target="$3"
printf "0x%08x\n" $hex_addr
printf "0x%08x" $hex_addr | xxd -r | xxd -e -g4 | xxd -r > __header
cat __header $filename > $target
chmod +x $target
rm -f __header
