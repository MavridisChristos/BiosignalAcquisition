# ### Christos N. Mavridis @ CSL, NTUA ###
# This makefile makes the shared library liblabview_dll.so
# Can be configured to compile cmbiosemi as well

# the shared library needs PKG_CONFIG_PATH
ifndef PKG_CONFIG_PATH
$(error Environment variable PKG_CONFIG_PATH or LIBUSB_PREFIX must be defined)
endif

CC=gcc
CXX=g++
CCFLAGS=-c -Wall #-g
LDFLAGS=-L. -llabview_dll  

all: liblabview_dll.so #cmbiosemi 

# liblabview_dll.so 
liblabview_dll.so.0.0.1: labview_dll.o bsif_libusb.o
	$(CXX) -o $@ -fPIC -lc -shared -Wl,-soname,liblabview_dll.so.0 \
		$^ `pkg-config --libs libusb-1.0`

liblabview_dll.so: liblabview_dll.so.0.0.1
	ln -sf $^ liblabview_dll.so
	/sbin/ldconfig -n .

labview_dll.o: labview_dll.cpp
	$(CXX) $^ $(CCFLAGS) -fPIC -o $@

# cmbiosemi
cmbiosemi: cmbiosemi.o 
	$(CXX) $^ $(LDFLAGS) -o $@
	
cmbiosemi.o: cmbiosemi.cpp
	$(CXX) $^ $(CCFLAGS) -o $@	

# clean
clean:
	rm -f labview_dll.o
	rm -f cmbiosemi.o
	rm -f *~ 2>/dev/null

