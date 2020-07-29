#!/bin/bash

if [ "$0" != "bash" ]
then # you are not bash

	echo "Usage: root# . $0"
	exit

else 

	if [ "$EUID" -ne 0 ]
    then 
    
	    echo "Usage: run as root."
	    echo "Use 'sudo su'"
	    
	else # you are root
		
	    /sbin/modprobe comedi
		/sbin/modprobe ni_pcimio
		sudo comedi_config /dev/comedi0 ni_pcimio
	    echo "*** Delsys is Ready ***"
	    
	fi

fi
