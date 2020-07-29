#!/bin/bash

bus=`lsusb | grep 0f44:ff21 | awk '{ print substr($2,length(3),3) }'`
device=`lsusb | grep 0f44:ff21 | awk '{ print substr($4,length(3),3) }'`

fxload -v -D /dev/bus/usb/$bus/$device -I /lib/firmware/polhemus/LbtyUsb8051.hex -t fx2 -s /lib/firmware/polhemus/a3load.hex

echo "*** Polhemus Liberty Ready ***"
