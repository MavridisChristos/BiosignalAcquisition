################## Christos N. Mavridis @ CSL, NTUA ############################
################################################################################
How to install:
Appropriate Firmware
Custom program 'cmliberty' to read motion data and do-whatever in real-time.
################################################################################

1. Install firmware
  cd Liberty/include/firmware_load
  ./install_udev_firmware.sh (may need chmod +x)
  or read the README file

2. Compile cmliberty:

  cd Liberty/build
  cmake .. (may need to delete CMakeCache.txt first)
  make 

############################# How to use #######################################

Writes to file (1st argument):
#channel,x,y,z,q0,q1,q2,q3,timestamp,framecount,distortion

Writes to terminal:
x,y,z, DCM of sensor 1
for online checking

Stops with Ctrl-C.

############################# How to run #######################################
Instructions:
	0. Connect the base, the sensors and the power supply. **
		Plug in the device (usb).
		Turn on the power.
	1. source initliberty.sh (productID must become 0xff20 from 0xff21 
		after loading the firmwire): 
		$ . ./initliberty 
		The LED in the liberty device should light constantly.	
	2. as root run cmliberty:
		root:$ ./cmliberty test  
		[run sudo modprobe pcspkr for the beep sound]

**	
Liberty tracks the position and orientation of the sensors using magnetic fields 
produced by the base. Magnetic symmetry commands that the sensors must remain 
in one hemisphere.

The zenith of the hemisphere is set by default to point x axis.
One can change this in Liberty_Acq::Liberty_Acq_Start.

During measurements, all sensors must be in this hemisphere and the range of 
the sensors' positions should be tested so as the LED to light constantly green. 

--------------------------------------------------------------------------------

Prerequisites:
	liberty.h
	liberty.o
	protocol.h
	LDFLAGS=-lusb
