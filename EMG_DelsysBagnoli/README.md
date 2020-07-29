################## Christos N. Mavridis @ CSL, NTUA ############################
################################################################################
How to install:
Comedi drivers & libraries to read from delsys device through DAQ- PCI 6036E (NI)
Custom program 'cmdelsys' to read EMG data and do-whatever in real-time.
################################################################################

1. Install 'Comedi kernel module source' & 'Library for Comedi' 
  may be available in app store / can be downloaded by www.comedi.org

2. Compile cmdelsys:

  cd Delsys/build
  cmake .. (may need to delete CMakeCache.txt first)
  make 
 
############################# How to use #######################################

Communicates with DAQ- PCI 6036E by National Instruments using comedi library.

Reads a buffer of 100 samples at a time by default (can be changed).

Writes to file (1st argument):
emg[1-16],\n

Stops when time (2nd argument in sec) is reached.

############################# How to run #######################################
0. Connect the sensors and the power supply to the amplifier. 
		Plug in the amplifier (DAQ- PCI 6036E).
		Turn on the power. **

1. Load comedilib (may need not be implemented):  
		$ . ./initdelsys 	

2. as root run cmdelsys:
		root:$ ./cmdelsys file-to-write seconds-to-last  

**	
The best maching values of amplitude gains for the amplifier are 1K.
Turn on the beeper to alert for poor sensor placement. 

--------------------------------------------------------------------------------

Prerequisites:
	comedi installed
	LDFLAGS=-lcomedi -lm
