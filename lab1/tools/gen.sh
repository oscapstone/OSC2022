#! /bin/sh
grep -v "-" GPIO | while read s 
do
    name=`echo $s | cut -d " " -f2`
    addr=`echo $s | cut -d " " -f1`
    echo "#define $name $addr"
done

grep -v "-" AUX_PERIPHERALS | while read s 
do
    name=`echo $s | cut -d " " -f2`
    addr=`echo $s | cut -d " " -f1`
    echo "#define $name $addr"
done


