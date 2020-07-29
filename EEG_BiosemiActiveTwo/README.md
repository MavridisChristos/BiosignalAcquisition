################## Christos N. Mavridis @ CSL, NTUA ############################
################################################################################
How to install:
USB drivers to read from biosemi device.
Actiview Software to check electrode connections.
Custom program 'cmbiosemi' to read EEG data and do-whatever in real-time.
################################################################################

1. Install libusb-1.0.08 in it's default location
   (/usr/local/include and /usr/local/lib):

	cd Biosemi/include/Linux32or64/libusb-1.0.8/
	./configure (may need chmod +x ./configure)
	make
	sudo su
	make install
	exit

2. Install the shared library liblabview_dll.so:
	
	cd Biosemi/include/Linux32or64/
	export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
  make (make sure you use the new Makefile)
  echo 'export LD_LIBRARY_PATH=/path-to-Linux32or64:$LD_LIBRARY_PATH' >> ~/.bashrc
  sudo su
  echo 'export LD_LIBRARY_PATH=/path-to-Linux32or64:$LD_LIBRARY_PATH' >> ~/.bashrc
  exit
	
3. Install LabVIEW-8.5:

  cd Biosemi/Actiview
  sudo dpkg -i *.deb
  echo 'export LD_LIBRARY_PATH=/usr/local/lib/LabVIEW-8.5:$LD_LIBRARY_PATH' >> ~/.bashrc
  sudo su
  echo 'export LD_LIBRARY_PATH=/usr/local/lib/LabVIEW-8.5:$LD_LIBRARY_PATH' >> ~/.bashrc
  exit

4. Compile cmbiosemi:

  cd Biosemi/build
  cmake .. (may need to delete CMakeCache.txt first)
  make
  
################################################################################
How to run:
Actiview Software to check electrode connections.
Custom program 'cmbiosemi' to read EEG data and do-whatever in real-time.
################################################################################

1. Actiview Software
  
  cd Biosemi/Actiview/Actiview707-Linux/  	
  sudo su
  ./Actiview707-Linux (may need chmod +x)
  
2. cmbiosemi

  cd Biosemi/ (labview_dll.ini should be in this folder)
  sudo su
  ./build/cmbiosemi file-to-write seconds-to-last
  (may need chmod +x)
  
################################################################################
