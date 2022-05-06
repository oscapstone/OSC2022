#! /bin/sh
BUS_IO_BASE=0x7E000000
PHY_IO_BASE=0x3F000000 
echo "/* Register's offset of GPIO */"
grep -v " - " GPIO | while read s 
do
    name=`echo $s | cut -d " " -f2`
    addr=`echo $s | cut -d " " -f1`
    addr=`(printf "(IO_BASE + 0x%X)\n" $((addr - BUS_IO_BASE)))`
    echo "#define $name $addr"
done

echo ""
echo ""
echo "/* Register's offset of UART and SPI */"
grep -v " - " AUX_PERIPHERALS | while read s 
do
    name=`echo $s | cut -d " " -f2`
    addr=`echo $s | cut -d " " -f1`
    addr=`(printf "(IO_BASE + 0x%X)\n" $((addr - BUS_IO_BASE)))`
    echo "#define $name $addr"
done


