#!/bin/sh

cp 70-polhemus.rules /etc/udev/rules.d/

mkdir -p /lib/firmware/polhemus
cp a3load.hex /lib/firmware/polhemus
cp LbtyUsb8051.hex /lib/firmware/polhemus

udevadm control --reload-rules

echo "Now when you plug the Liberty tracker, it should show the 0f44:ff21"
echo "vendor/product id for a couple of seconds, and then switch to 0f44:ff20"
echo "(after it loaded the firmware)"



