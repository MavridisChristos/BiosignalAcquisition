/**************************************
 ** Christos N. Mavridis @ CSL, NTUA **
 **************************************

This code virtually implements the procedure described at:
http://www.biosemi.com/faq/make_own_acquisition_software.htm 

prerequisites:
	bsif.h
	bsif_libusb.o
	labview_dll.ini
	labview_dll.h
	labview_dll shared library
	LDFLAGS=-L. -llabview_dll

run:
	1. as root source initbiosemi.sh 
	2. as root run cmbiosemi 			*/

#include <stdio.h> 
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>	
#include <errno.h>
#include <signal.h>
#include <iostream>

#include "labview_dll.h"
#include "bsif.h"

#define SPEED_MODE 7
#define NUM_CHANNELS 58
#define DATA_TO_WRITE 32 // 40 with 8 EX channels
#define PRINT_STATUS_FREQ 8

using namespace std;

static int got_signal = 0;
FILE * eegfile;

class Biosemi_Acq
{
private:
	PUCHAR	ringBuffer;
	CHAR 	controlBuffer[64];
	char	infoBuffer[120];
	INT_PTR	ringBufferSize; // 32Mbytes 
	INT_PTR bytes2Use;
	HANDLE	handle;
	float bytesPerMsec;
	int throughPut;
	int nbuffers; // buffer of 128Kbytes (= stride)
    int	nextSync;
	int lastSync;
	INT_PTR	seam;
    INT_PTR	orion;
	INT_PTR	lastSeam;
	
public:
    long int datum[4520][DATA_TO_WRITE]; // 565 samples in 128Kbyte
    // = round [stride / (NUM_CHANNELS * 4 bytes_per_channel)]
    
    Biosemi_Acq();
	~Biosemi_Acq();	
    int Biosemi_Acq_Init(void);
    int Biosemi_Acq_Start(void);
    int Biosemi_Acq_Get(void); 
    int Biosemi_Acq_Close(void); 
    int Biosemi_Acq_Status(void); 
};

static void 
sighandler(int signo)
{
	printf("Terminating by signal %d\n", signo);
	got_signal = 1;
}



/*********************************************************************/



int 
main (int argc, char **argv) 
{

	Biosemi_Acq biosemi;
	int bioSamplesRead;
	
	int		time_limit = 0; // Default: infinite
	double 	startTime;
	double	timeStamp; 
		
	struct 	sigaction sigact;
	struct 	timeval tim;
	
	/*** User Input ***/
	if (argc<2)
	{
		printf("Usage: 1st argument is file to write\n");
		return 1;
	}
	else	
	{
		/* open and clear file */
		eegfile = fopen (argv[1],"w");
		printf("Output file: '%s'\n",argv[1]);
	}
	
	if (argc<3)
	{
		printf("Unlimitted acquisition. Press Ctrl-C to end\n");
		printf("Usage: 2nd argument is time limit (optional)\n");
	}	
	else
	{
		time_limit = atoi(argv[2]);
		printf("Acquisition duration: %d sec\n", time_limit);
	}
		
	/*** catch signals ***/
	sigact.sa_handler = sighandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);

	if(biosemi.Biosemi_Acq_Init() < 0)
		return -1;	

	if(biosemi.Biosemi_Acq_Start() < 0)
		return -1;	
		
	/*** Past this point, the ring buffer is constantly filled ***/
	
	/* Get startTime */  
    gettimeofday(&tim, NULL);  
    startTime = tim.tv_sec + (tim.tv_usec/1000000.0);
	
	/* main sampling loop: for all buffers to come */	 
	while(!got_signal)
	{	
	  /* Get timeStamp */
		gettimeofday(&tim, NULL);  
    	timeStamp = tim.tv_sec+(tim.tv_usec/1000000.0);
    	
		// read 29 Kbyte = 128 samples
		bioSamplesRead = biosemi.Biosemi_Acq_Get(); 
		if(bioSamplesRead < 0)
			break;
		  
		/* cmcode */
		for(int i=0; i<bioSamplesRead; i++) 
		{	 
			for(int j=0; j<DATA_TO_WRITE; j++)
			{ 
				fprintf(eegfile,"%ld\t",biosemi.datum[i][j]);
			}
			fprintf(eegfile,"%.6lf\t",timeStamp);
			fprintf(eegfile,"\n");
		}		
		
		if(biosemi.Biosemi_Acq_Status())
			printf("@ %.6f\n", timeStamp);
		
		if ((time_limit > 0) && ((int)(timeStamp-startTime) >= time_limit))
		{
			printf("Time limit (%d seconds) reached.\n", time_limit);
			break; // exit main loop
		}
		
	} 

	biosemi.Biosemi_Acq_Close();
	fclose(eegfile);
	
	// All done
	printf("*** cmBiosemi: Done. ***\n");
	
	return 0; 
}



/******************************************************************/



Biosemi_Acq::Biosemi_Acq()
{
	ringBufferSize = 59392;
	bytes2Use = ((INT_PTR)ringBufferSize)*512;
	nbuffers = 0; 
    nextSync = 0;
	seam = 0;
    orion = 0;
	lastSeam = 0;
	bytesPerMsec = 0;
	throughPut = 0;
	
	/* allocate ring buffer */
	ringBuffer = (PUCHAR) malloc( (SIZE_T) bytes2Use);
	if (ringBuffer == 0)
	{
		printf(" Memory allocation error : %d\n", errno);
		abort();
	}
}

Biosemi_Acq::~Biosemi_Acq()
{
	free(ringBuffer);
}

int 
Biosemi_Acq::Biosemi_Acq_Init(void)
{	
	/* Step.1 Open USB2 driver */
	if ((handle=OPEN_DRIVER()) == (HANDLE)NULL)
	{
		printf("Can't open device driver!\n");
		return -1;
	}

	/* Step.2 Initialize USB2 interface */
	for (int i=0; i<64; i++)
		controlBuffer[i] = 0; // controlBuffer = zeros(64)

	if (USB_WRITE(handle, &controlBuffer[0]) == FALSE)
	{
		printf ("usb_write for enable handshake trouble %d\n", errno); 
		return -1;
	}

	/* Step.3 Initialize the ring buffer */
	READ_MULTIPLE_SWEEPS(handle, (PCHAR)ringBuffer, bytes2Use);
	
	/* Tuning if no labview_dll.ini file*/
	// BSIF_SET_SYNC(true);
	// BSIF_SET_STRIDE_KB(256); // 131072 bytes

	return 0;
}

int 
Biosemi_Acq::Biosemi_Acq_Start(void)
{
	/* Step.4 Enable the handshake */

	controlBuffer[0] = (CHAR)(-1); // MSByte of controlBuffer = 0xff
	
	if (USB_WRITE(handle, &controlBuffer[0]) == FALSE)
	{
		printf ("usb_write for enable handshake trouble %d\n", errno); 
		return -1;
	}
	
	/*** Past this point, the ring buffer is constantly filled ***/

	return 0;
}

int // returns number of measurements (rows) in datum
Biosemi_Acq::Biosemi_Acq_Get(void)
{
	BOOL read_ptr;
	int row = 0;
	
	/* Step.5 Read 128Kbyte from Buffer */	
	if ((read_ptr=READ_POINTER(handle, &seam)) == FALSE)
	{
		printf("Acquisition not running!\n");
		return -1;
	}
	
	/* Check seam */
	if (seam == lastSeam) 
	{	
		printf("No new data measured..\n");
		return -1; // exit main loop
	}
	
	/* define orion (longs read) */
	orion = seam - lastSeam;
	if(orion<0)
		orion += bytes2Use;
	orion /=4;
	
	lastSeam = seam;

	/*** For every data measurement ***/		
	for (int ii=0; ii<=orion; ii+=NUM_CHANNELS) 
	{
		/* Handle nextSync */
		if((nextSync+NUM_CHANNELS-1)*4 >= bytes2Use)
			nextSync += NUM_CHANNELS;// word does not fit in buffer
		if(nextSync*4 >= bytes2Use)
			nextSync -= (INT)bytes2Use/4;//loop index
		if((seam >= nextSync*4) && ((nextSync+NUM_CHANNELS-1)*4 >= seam))
				break; //word has not been read entirely
		
		/* check missing sync */
		/* cmPattern: "sync - nosync - nosync" */
		if(!((((unsigned int *)ringBuffer)[nextSync] == 0xffffff00) &&
			 (((unsigned int *)ringBuffer)[nextSync+1] != 0xffffff00) &&
			 (((unsigned int *)ringBuffer)[nextSync+2] != 0xffffff00)))
		{
			printf("missing sync: #buffer %d, @ %d\n", nbuffers, nextSync);
			return -1;		
		}
		
		/* store every data measurement (in uV) to datum */		
		for(int i=2; i<DATA_TO_WRITE+2; i++) 
			datum[row][i-2] = ((int *)ringBuffer)[nextSync+i]/8192;  
		row++;		
		
		nextSync+=NUM_CHANNELS;	
	} 
	
	nbuffers++;
	if(nbuffers>1023)
		nbuffers = 0;
		
	return row;
}

int 
Biosemi_Acq::Biosemi_Acq_Close(void)
{
	/* Display Driver Information */
	GET_DRIVER_INFO(infoBuffer, sizeof(infoBuffer)); // actual stride 
	printf("%s\n", infoBuffer);	// determined after the end of 5th buffer	
	
	/* Step.7 Disable the Handshake */
	controlBuffer[0] = 0;

	if (USB_WRITE(handle, (PCHAR)&controlBuffer[0]) == FALSE)
	{
		printf ("USB_WRITE for disable handshake trouble %d\n", errno); 
		return -1;
	}
	
	/* Step.8 Close the drivers & free memory */
	CLOSE_DRIVER(handle);
	
	return 0;
}

int 
Biosemi_Acq::Biosemi_Acq_Status(void)
{
	/*** Status output every PRINT_STATUS_FREQ ***/
	if (nbuffers % PRINT_STATUS_FREQ == 0)
	{
		/* determine throughput in words per channel per second */	
		bytesPerMsec = BSIF_GET_BYTES_PER_MSEC();
		throughPut = (int)(bytesPerMsec*1000./(double)(NUM_CHANNELS*4));
		/* output message */
		printf("#buffer %d, seam %lf, lastSync %d, throughput %d\t", 
				nbuffers, (double)seam/232, nextSync/58, throughPut);
		return 1;
	}
	else
		return 0;
}
