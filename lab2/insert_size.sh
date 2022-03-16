#!/bin/bash
if [[ "$2" == "" ]]; then
    echo "Usage $0 <input filename> <output filename>"
    exit 1
fi
filename="$1"
target="$2"
filesize=`stat --printf="%s" $filename`
hexfilesize=`printf "0x%08x" $filesize`
echo $hexfilesize
# echo $hexfilesize | xxd -r | xxd -e -g4 | xxd -r > __header
echo $hexfilesize | xxd -r > __header
cat __header $filename > $target
chmod +x $target
rm -f __header
