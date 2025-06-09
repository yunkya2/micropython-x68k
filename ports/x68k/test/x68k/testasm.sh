#!/bin/sh
# MicroPython for X680x0 inline assmembler test script
RUN68=run68
MPY="${RUN68} ../../build/micropython.x"
MPC="${RUN68} ../../mpyconv/mpyconv.x"
MPT="../../../../tools/mpy-tool.py"

rm -f *.mpy *.bin.*.xd *.o *.s *.dis

# convert the asm file to a mpy file
${MPC} asmm68k.py
${MPT} -x asmm68k.mpy
${MPT} -d asmm68k.mpy

# extract the binary data from the mpy file
binsize=`${MPT} -d asmm68k.mpy | grep 'raw data:' | sed 's/.*: \([0-9][0-9]*\).*/\1/'`
tail -c $((${binsize}+3)) asmm68k.mpy | head -c -3 | head -c -8 | dd bs=1 skip=8 > asmm68k.bin
xd asmm68k.bin > asmm68k.xd

# convert and assemble the asm file in gas format
./py2gas.py asmm68k.py > asmgas.s
m68k-xelf-as asmgas.s -o asmgas.o
m68k-xelf-objcopy -O binary asmgas.o asmgas.bin
m68k-xelf-objdump -D asmgas.o > asmgas.dis
xd asmgas.bin > asmgas.xd

# compare the binary data
cat asmm68k.xd
diff asmm68k.xd asmgas.xd
if [ $? -eq 0 ]; then
    echo "### Binary data matches ###"
else
    echo "XXX Binary data does not match XXX"
fi
