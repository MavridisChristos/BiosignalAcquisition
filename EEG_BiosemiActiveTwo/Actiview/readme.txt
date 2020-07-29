19 Februari 2015,

Actiview 7.07 depends on:
-Labview85-rte-8.5-1.i386.rpm
-Labview-rte-aal-1.1-1.i386.rpm
-libusb-1.0-0:i386 (32 bit version)
-xfonts-75dpi-transcoded (required for proper font formatting)
-xfonts-100dpi-transcoded (required for proper font formatting)

Labview85-rte-8.5-1.i386.rpm and Labview-rte-aal-1.1-1.i386.rpm can be downloaded from "http://www.ni.com/download/labview-run-time-engine-8.5/868/en/".

If libusb-1.0-0 is not included in your linux distribution, you can install it from the biosemi driver for linux or download and install  libusb from libusb.org (in both cases Actiview requires the 32 bit version of libusb).

The biosemi driver is included in Actiview 7.07, therefore it is not necessarily to install the driver separately.

USB Access Permissions
======================

In Linux, USB devices often have ownership and permissions set to
restrict access to "root" only.  This access must be opened
whenever the BioSemi Receiver's cable is plugged into one of
the computer's USB ports.  Newer systems use "udev" or "udevadm"
to do this; older systems use "hotplug".

udev
----

Our development system is CentOS 5, Linux 2.6.18 (similar
to Redhat AS5).  This system uses "udev" to open USB access.
First check the current access:
1) Plugin the Biosemi Receiver's USB cable into the computer.
2) Open a terminal or xterm window and type "dir -l /dev/usb*".
3) If the output shows that the protections are "crw-------" and
   the owner is "root", use the following steps to have the
   permissions change automatically wheneven the BioSemi Receiver is
   plugged in.

   a) As "superuser", create file "/etc/udev/rules.d/60-biosemi.rules"
      containing these six lines:

# biosemi usb receiver
ATTR{idVendor}=="0547", ATTR{idProduct}=="1005", MODE="0666"
ATTR{idVendor}=="0547", ATTR{idProduct}=="1005", SYMLINK+="biosemi"
ATTR{idVendor}=="04b4", ATTR{idProduct}=="8613", MODE="0666"
ATTR{idVendor}=="04b4", ATTR{idProduct}=="8613", SYMLINK+="biosemi2"
LABEL="biosemi_rules_end"

   b) Reload the rule changes and restart "udev' by using these two
      commands:

      /sbin/udevcontrol reload_rules
      /sbin/udevstart        (or sometimes: /sbin/start_udev)          

      This should create a link called "/dev/biosemi" to the actual
      USB device with all access for everyone.  Check by re-inserting
      the BioiSemi Receiver's USB cable into the computer's USB slot
      and use the command "dir -l /dev/biosemi".  Note, sometimes a
      re-boot is required to make these rules work.        

   c) As an ordinary user try to run Actiview


udevadm
-------

A newer test system is Fedora11, Linux 2.6.29.  This system uses
"udevadm" to open USB access.

The procedure here is identical to the "udev" described above except
in step 3b the commands to reload the rule changes and restart "udev'
are:

      udevadm control --reload-rules
      udevadm control --start-exec-queue


hotplug
-------

An older system that was used for testing runs Redhat AS4,
Linux 2.6.9.  This system uses "hotplug" to control USB access.
To open access to the USB port using "hotplug" perform the following
steps, as "superuser".
1) Create file "/etc/hotplug/usb/usbbiosemi.usermap" containing this
   line:

usbbiosemi 0x0003 0x0547 0x1005 0 0 0 0 0 0 0 0 0

2) Create file "/etc/hotplug/usb/usbbiosemi" containing this
   script:

#!/bin/bash

if [ "${ACTION}" = "add" ] && [ -f "${DEVICE}" ]
then
   chown root "${DEVICE}"
   chgrp root "${DEVICE}"
   chmod 666 "${DEVICE}"
fi

3) Make this script file executable with this command:
   chmod a+x /etc/hotplug/usb/usbbiosemi 

4) As an ordinary user try to run Actiview

