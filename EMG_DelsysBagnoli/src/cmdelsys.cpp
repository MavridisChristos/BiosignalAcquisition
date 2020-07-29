/*********************************
 Christos N. Mavridis @ CSL, NTUA
 *********************************
 
 This code communicates with DAQ- PCI 6036E using comedi library.
 
***************************************

Writes to file (1st argument):
channels,timestamp

***************************************
 prerequisites:
	comedi installed
	LDFLAGS=-lcomedi -lm

 run:
	1. source initdelsys.sh
	2. as root run cmdelsys 		*/ 

#include <cstdlib>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>

#include <comedilib.h>

#define DEL_FREQ 1000
#define BUFSZ 10000
#define DEL_CHANS 16

using namespace std;

static int got_signal = 0;
FILE * emgfile;

class Comedi_Acq
{
private:
    comedi_t * dev;
    comedi_cmd cmd; 
    int n_scan ;// Number of samples per read
    char delbuf[BUFSZ]; 
    char * filename ;//= "/dev/comedi0";
    comedi_range * rang; //range_info[]
    int subdevice;
    unsigned int chanlist[DEL_CHANS];
    
    int prepare_cmd( void );
public:
	double * data;
    
    Comedi_Acq(int n_samples_);
	~Comedi_Acq();
    int Comedi_Acq_Init(void);
    int Comedi_Acq_Get(size_t count);   
};

static void 
sighandler(int signo)
{
	printf("Terminating by signal %d\n", signo);
	got_signal = 1;
}



/*******************************************************************/



int main(int argc, char **argv)
{

  	int delsamples = 100; // Number of samples per read
	Comedi_Acq delsys (delsamples);
	int delbytesRead=0;
	int StopTime=0;
  	
	double 	startTime;
	double	timeStamp; 
		
	struct 	sigaction sigact;
	struct 	timeval tim;

  	/*** User Input ***/
	if (argc<2)
	{
		printf("Usage: 1st argument is file to write\n");
		return -1;
	}
	else	
	{
		/* open and clear file */
		emgfile = fopen (argv[1],"w");
		printf("Output file: '%s'\n",argv[1]);
	}

	if (argc<3)
	{
		printf("Usage: 2nd argument is time to stop\n");
		return -1;
	}
	else	
	{
		StopTime = atoi(argv[2]);
		printf("Acquisition for: '%d' seconds..\n",StopTime);
	}
  	  	
	printf("Press Ctrl-C to end\n");

	/*** catch signals ***/
	sigact.sa_handler = sighandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);

  	if(delsys.Comedi_Acq_Init()<0)
  		return -1;

	/* Get startTime */  
    gettimeofday(&tim, NULL);  
    startTime = tim.tv_sec + (tim.tv_usec/1000000.0);

   	printf("*** Start.. *** \a\n"); // play beep sound

	while(!got_signal)
	{

		/* Get timeStamp */
		gettimeofday(&tim, NULL);  
    	timeStamp = tim.tv_sec+(tim.tv_usec/1000000.0) - startTime;

		if((delbytesRead = delsys.Comedi_Acq_Get(DEL_CHANS*delsamples*2))<0)
			return -1; // get 100 samples of 16 measurements of 2 bytes

		for(int i=0; i< delbytesRead/2; i++) // print all the bytes read
		{
			fprintf(emgfile,"%lf\t", delsys.data[i]);
			if((i+1)%DEL_CHANS==0)
			{	
			  fprintf(emgfile,"%.6lf\t",timeStamp);
			  fprintf(emgfile,"\n");
			}			
		}	

		//cout<<"SamplesRead: "<< delbytesRead/2 <<", @ "<< timeStamp<<endl; 

		if(timeStamp>StopTime)
			got_signal = 1;
	}

	fclose(emgfile);
	
	// All done
	printf("*** Stop.. *** \a\n"); // play beep sound	
	printf("*** cmDelsys: Done. ***\n");
	
	return 0;
}



/*******************************************************************/



Comedi_Acq::Comedi_Acq(int n_samples_) 
{
	n_scan = n_samples_;
    filename = strdup("/dev/comedi0");
    subdevice = 0;
    /* allocate memory for buffer */
    data = (double *) malloc(DEL_CHANS * n_scan * sizeof(double));
	if (data == 0)
	{
		printf(" Memory allocation error : %d\n", errno);
		abort();
	}
}

Comedi_Acq::~Comedi_Acq() 
{
	free(data);
}

int Comedi_Acq::Comedi_Acq_Init( void )
{

   	int ret;
    int range = 0;
    int aref = AREF_GROUND;
	
    /* comedi_open () */
    dev = comedi_open(filename);
    if (!dev )
    {
    	printf("Error opening board... ABORTING\n");
    	return -1;
    }
    
    /* Set up channel list */
    for (int i = 0; i < DEL_CHANS ; i++ )
      chanlist[i] = CR_PACK(i, range, aref);
    
    /* find a command for the device */
    prepare_cmd();
    
    /* Test a command to see if the trigger sources and arguments 
     are valid for the subdevice.*/
    ret = comedi_command_test(dev, &cmd);
    if ( ret < 0 ) 
    {
        printf("Error while testing comedi_command...ABORTING\n");
        return -1;
    }
    
    /* if you can't get a valid command in two tests, the original
	 command wasn't specified very well. */
    ret = comedi_command_test(dev, &cmd);
    if ( ret < 0 ) {
        printf("Error while testing comedi_command...ABORTING\n");
        return -1;
    }
    
    printf("Second comedi_command test returned: %i\n", ret);
    if ( ret != 0 )
        return -1;

    printf("////////////////////////////////////////////////\n");
    printf("DAQ- PCI 6036E Initialized successfully for:\n" 
    		"\tFrequency: %d\n \tNumber of Channels: %i\n",DEL_FREQ, DEL_CHANS);
    printf("////////////////////////////////////////////////\n");
    
    return 0;
    
}

int Comedi_Acq::Comedi_Acq_Get(size_t count)
{

    int ret;
    size_t total = 0;   
    
	ret = comedi_command(dev, &cmd);    
    if ( ret < 0 ) 
	{
        printf("Error while strating comedi_command...ABORTING\n");
        return -1;
    }
	
    while(1)
    {
    	ret = read( comedi_fileno(dev), delbuf, count-total );
    	if ( ret < 0 ) 
    	{
    		printf("Error while reading delsys... ABORTING\n");
    		return -1;
    	}   
 	   	else // data = [ chan1 chan2 ... chan1 chan2 ... ] 
    	{
    		for( int i = 0; i < ret/2; i++ ) // for all samples read
    		{
    			// sampl_t = unsigned short = 2 bytes = [0,2*32767+1]
    		    data[total+i] =(double) ((((sampl_t *)delbuf)[i])-32767)*10/32767;
    		}
    		total += ret;
			if(total == count)
				break;
    	}
    }
    return total;
}

int Comedi_Acq::prepare_cmd( void )
{
    memset(&cmd, 0, sizeof(cmd));
    cmd.subdev = subdevice;
    cmd.flags = 0;
    
    cmd.start_src = TRIG_NOW;
    cmd.start_arg = 0;
    
    cmd.scan_begin_src = TRIG_TIMER;
    cmd.scan_begin_arg = 1e9/DEL_FREQ;
    
    cmd.convert_src = TRIG_TIMER;
    cmd.convert_arg = 1;
    
    cmd.scan_end_src = TRIG_COUNT;
    cmd.scan_end_arg = DEL_CHANS;
    
    cmd.stop_src = TRIG_COUNT;
    cmd.stop_arg = n_scan;
    
    cmd.chanlist = chanlist;
    cmd.chanlist_len = DEL_CHANS;
    return 0;
}
