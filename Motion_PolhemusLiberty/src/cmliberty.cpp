/**************************************
 ** Christos N. Mavridis @ CSL, NTUA **
 **************************************
 
prerequisites:
	liberty.h
	liberty.o
	protocol.h
	LDFLAGS=-lusb

run:
	0. plug in the device
	1. source initliberty.sh (check productID from 0xff21 to 0xff20 after loading the firmwire)
	2. as root run cmliberty 

***************************************

Writes to file (1st argument):
#channel,x,y,z,q0,q1,q2,q3,timestamp,framecount,distortion

Writes to terminal:
x,y,z, DCM of sensor 1
for online checking

***************************************

The zenith of the hemisphere is set to point x axis.
(in Liberty_Acq::Liberty_Acq_Start)

During measurements, all sensors must be in this hemisphere and the range of the sensors' positions should be tested so as the green LED to light constantly. 

*/

#include <usb.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include "liberty.h"
#include "protocol.h"

#define VENDOR 0xf44 // polhemus vendor 0x0f44
#define PRODUCT 0xff20 // productID from 0xff21 to
					  // 0xff20 after loading the firmwire
#define control(c) ((c) & 0x1f) /// make control char out of ordinary char

using namespace std;

static int got_signal = 0;
FILE * motionfile;

class Liberty_Acq
{
private:
	struct usb_device * dev; // liberty device
    buffer_t buffer; // 8Kbytes
	usb_dev_handle * handle;
	
	int count_bits(uint16_t v);
	struct usb_device *find_device_by_id(uint16_t vendor, uint16_t product);
	int request_num_of_stations(void);
	void set_hemisphere(int x, int y, int z);
public:
    int n_stations;
    station_t *stations; // stations = sensors
    
    Liberty_Acq();
	~Liberty_Acq();	
    int Liberty_Acq_Init(void);
    int Liberty_Acq_Start(void);
    int Liberty_Acq_Get(void); 
    int Liberty_Acq_Close(void);  
};

static void 
sighandler(int signo)
{
	printf("Terminating by signal %d\n", signo);
	got_signal = 1;
}



/*****************************************************************/



int main(int argc, char **argv)
{
   
    Liberty_Acq liberty;
	int n = 0;
	float q0 = 0;
	float q1 = 0;
	float q2 = 0;
	float q3 = 0;	

	struct 	sigaction sigact;

	/*** User Input ***/
	if (argc<2)
	{
		printf("Usage: 1st argument is file to write\n");
		return 1;
	}
	else	
	{
		/* open and clear file */
		motionfile = fopen (argv[1],"w");
		printf("Output file: '%s'\n",argv[1]);
	}
	
	printf("*** Unlimitted acquisition. Press Ctrl-C to end. ***\n");
		
	/*** catch signals ***/
	sigact.sa_handler = sighandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);


	if(liberty.Liberty_Acq_Init()<0)
		return -1;
	
	if(liberty.Liberty_Acq_Start()<0)
    	return -1;
    	
    /* Past this point continuous emg data are buffering */	
	printf("*** Beep.. *** \a\n"); // play beep sound
    	
	while (!got_signal) 
    {
    	/* Read one sample of measurements */
        if(liberty.Liberty_Acq_Get()<0)
        	return -1;
        
		n++;
		///*        
		// Print measurements to file 
        for (int i = 0 ; i != liberty.n_stations; ++i) 
        {
			fprintf(motionfile,"%u\t", i+1);            
			fprintf(motionfile,"%f\t", liberty.stations[i].x);
            fprintf(motionfile,"%f\t", liberty.stations[i].y);
            fprintf(motionfile,"%f\t", liberty.stations[i].z);
            fprintf(motionfile,"%f\t", liberty.stations[i].quaternion[0]);
            fprintf(motionfile,"%f\t", liberty.stations[i].quaternion[1]);
            fprintf(motionfile,"%f\t", liberty.stations[i].quaternion[2]);
            fprintf(motionfile,"%f\t", liberty.stations[i].quaternion[3]);
        	fprintf(motionfile,"%u\t", liberty.stations[i].timestamp);
            fprintf(motionfile,"%u\t", liberty.stations[i].framecount);            
            fprintf(motionfile,"%u\t", liberty.stations[i].distortion);
            //fprintf(motionfile,"\n");
        }
		fprintf(motionfile,"\n");
		//*/

		q0 = liberty.stations[0].quaternion[0];
		q1 = liberty.stations[0].quaternion[1];
		q2 = liberty.stations[0].quaternion[2];
		q3 = liberty.stations[0].quaternion[3];

		// print to terminal
		if(n%240==0)
		{	
			printf("SENS 1 - xyz:\t");
			printf("%1.2f\t", liberty.stations[0].x);
			printf("%1.2f\t", liberty.stations[0].y);
			printf("%1.2f\t", liberty.stations[0].z);	
			printf("\n");
			printf("SENS 1 - DCM:\n");			
			printf("%1.2f\t", q0*q0+q1*q1-q2*q2-q3*q3);
			printf("%1.2f\t", 2*(q1*q2-q0*q3));
			printf("%1.2f\t", 2*(q1*q3+q0*q2));	
			printf("\n"); 
			printf("%1.2f\t", 2*(q3*q0+q1*q2));
			printf("%1.2f\t", q0*q0-q1*q1+q2*q2-q3*q3);
			printf("%1.2f\t", 2*(q2*q3-q0*q1));	
			printf("\n"); 
			printf("%1.2f\t", 2*(q1*q3-q0*q2));
			printf("%1.2f\t", 2*(q1*q0+q3*q2));
			printf("%1.2f\t", q0*q0-q1*q1-q2*q2+q3*q3);	
			printf("\n"); 
			n=0;	
		}   
	}

	if(liberty.Liberty_Acq_Close()<0)
		return -1;
	
    fclose(motionfile);
    
    // All done
	printf("*** cmLiberty: Done. ***\n");
    
    return 0;
}



/*****************************************************************/



Liberty_Acq::Liberty_Acq(){}

Liberty_Acq::~Liberty_Acq() {}

int 
Liberty_Acq::Liberty_Acq_Init(void)
{
	char * libcmd = NULL;
	
	/* Step.1 Find and open device */ 
    usb_init();

    dev = find_device_by_id(VENDOR, PRODUCT);
    if (!dev) 
    {
      printf("Could not find the Polhemus Liberty Device.\n");
      return -1;
    }

    handle = usb_open(dev);
    if (!handle) 
    {
    	printf("Could not get a handle to the Polhemus Liberty Device.\n");
		return -1;
    }

    if (!liberty_init(handle)) 
    {
        printf("Could not initialize the Polhemus Liberty Device.\n");
		usb_close(handle);
        return -1;
    }

	libcmd = strdup("f1\r");
	if (!liberty_send(handle, libcmd)) 
    {
    	printf ("liberty_send(f1) trouble %d\n", errno); 
		return -1;
	}
	
	/* Step.2 Initialize the liberty buffer */
	init_buffer(&buffer);
	
	/* Step.3 Find number of connected sensors */
    for(int i=0; i<5; i++) 
	{
    	n_stations = request_num_of_stations();
    	printf("Liberty found %d stations.\n", n_stations);
    	if (n_stations == 0) 
    	{
    		printf("Re-requesting number of stations.\n");
    		sleep(2);
    	}
    	else
    		break;
    }	
    
    if (n_stations == 0) 
    	return -1;
    
    stations = (station_t*)(malloc(sizeof(station_t) * n_stations));
    if (!stations) 
    	return -1;
	
	free(libcmd);
	return 0;
}

int 
Liberty_Acq::Liberty_Acq_Start(void)
{
	char * libcmd = NULL;
	
	/* Step.4 Enable the handshake */
    
    /* define which information to get per sensor
       if this is changed, the station_t struct has to be edited accordingly */
    libcmd = strdup("o*,8,9,11,3,7\r");
    if (!liberty_send(handle, libcmd)) 
    {
    	printf ("liberty_send(o*) trouble %d\n", errno); 
		return -1;
	} // o* applies to all stations
	
    // setting output hemisphere will produce a response which we're ignoring
	// zenith of hemisphere pointing x axis 
    set_hemisphere(1, 0, 0);
    
    /* switch output to centimeters */
    libcmd = strdup("u1\r");
    liberty_send(handle, libcmd);
    liberty_clear_input(handle); // right now, we just ignore the answer

    /* enable continuous mode (get data points continously) */
    libcmd = strdup("c\r");
    if (!liberty_send(handle, libcmd)) 
    {
    	printf ("liberty_send(c) trouble %d\n", errno); 
		return -1;
	}

	/* Past this point data are being continuously buffered */
	
	free(libcmd);
	return 0;
}

int 
Liberty_Acq::Liberty_Acq_Get(void)
{
	/* Step.5 Read Buffer */
    if (!liberty_receive(handle, &buffer, 
    					stations, sizeof(station_t)*n_stations)) 
    {
        fprintf(stderr, "receive failed\n");
    	return -1;
    } // try to read until TIMEOUT = 1000 ms
       
    /* Note: timestamp is the time in mS after the first read to the
    system after turning it on
    at 240Hz, the time between data sample is 1/240 = .00416_
    seconds.
    The framecount is a more exact way of finding out the time:
    timestamp = framecount*1000/240 (rounded down to an int)* */
	return 0;
}

int 
Liberty_Acq::Liberty_Acq_Close(void)
{
	char * libcmd = NULL;
	/* Step.7 Disable the Handshake */ 
    libcmd = strdup("p"); 
	if (liberty_send(handle, libcmd) == 0)
	{
		printf ("Liberty_send(p) for disable handshake trouble %d\n", errno); 
		return -1;
	}

	/* Step.8 Close the drivers */
    usb_close(handle);
    
    free(stations);
    stations=NULL;
    free(libcmd);

	return 0;
}

int 
Liberty_Acq::count_bits(uint16_t v)
{
    int c;
    for (c = 0; v; c++)
    {
        v &= v - 1; // clear the least significant bit set
    }
    return c;
}

struct usb_device *
Liberty_Acq::find_device_by_id(uint16_t vendor, uint16_t product)
{
    struct usb_bus *bus;
	struct usb_device *devi;
	
    usb_find_busses();
    usb_find_devices();

    for (bus = usb_get_busses(); bus; bus = bus->next) 
    {
        for (devi = bus->devices; devi; devi = devi->next) 
        {
            if (devi->descriptor.idVendor == vendor
                && devi->descriptor.idProduct == product)
                return devi;
        }
    }
    return NULL;
}

int 
Liberty_Acq::request_num_of_stations(void) 
{    
    static char cmd[] = { control('u'), '0', '\r', '\0' };
    active_station_state_response_t resp;
    
    liberty_send(handle, cmd);
    liberty_receive(handle, &buffer, &resp, sizeof(resp));

    //printf("station number: 0x%0x\n",resp.head.station);
    //printf("Init cmd: 0x%0x\n",resp.head.init_cmd);
    //printf("Error: 0x%0x\n",resp.head.error);
    //printf("Reserved: 0x%0x\n",resp.head.reserved);
    if (resp.head.init_cmd==21) 
    	return count_bits(resp.detected & resp.active);
    else 
    	return 0;
}

void 
Liberty_Acq::set_hemisphere(int x, int y, int z)
{
    char cmd[32];
    
    snprintf(cmd, sizeof(cmd), "h*,%d,%d,%d\r", x, y, z);
    liberty_send(handle, cmd);
} // sets the zenith of the hemisphere in direction of vector (x, y, z)
