/*---------------------------------------------------------------------------
  main.c
  By Ted B. Hale
	CPU and IO utilization display daemon	
    
	This file implements the main 

  2014-03-03  initial edits
  2014-03-21  diagnostic mode added
  2014-04-01  use configuration file
  2014-12-12  add network usage support
  
---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>
#include <mcp23017.h>

#include "log.h"
#define EXTERN
#include "utilization.h"

#define PIDFILE "/var/run/utilization.pid"
#define CONFIGFILE "/etc/default/utilization.cfg"

static char* IDENTIFIER = "UtilizationLEDs";

//************************************************************************
// handles signals to restart or shutdown
void sig_handler(int signo)
{
	switch (signo)
	{
		case SIGPWR:
			break;

	case SIGHUP:
		// do a restart
		Log("SIG restart\n");
		kicked = 1;
		break;

	case SIGINT:
	case SIGTERM:
		// do a clean exit
		Log("SIG exit\n");
		kicked = 2;
		break;
		
	case SIGFPE:
		Log("SIGFPE ******* restart program\n");
		kicked = 1;
		break;
	
	}
}

//***************************************************************************
// sleep with more precision
int Sleep(int millisecs)
{
	return usleep(1000*millisecs);
}

//**************************************************************************
// set the GPIO outputs for LED bargraph display
void setUtLed(int ledType, int val)
{
	int i;
	piLock(0);
	if (ledType==CPU)
	{
		for (i=0; i<=maxCpuPin; i++)
		{
			if (val>cpuThresh[i])
				digitalWrite(cpuPin[i], 0);
			else
				digitalWrite(cpuPin[i], 1);
		}
	}
	else if (ledType==IO)
	{
		for (i=0; i<=maxIoPin; i++)
		{
			if (val>ioThresh[i])
				digitalWrite(ioPin[i], 0);
			else
				digitalWrite(ioPin[i], 1);
		}
	}
	else if (ledType==NET)
	{
		for (i=0; i<=maxNetPin; i++)
		{
			if (val>netThresh[i])
				digitalWrite(netPin[i], 0);
			else
				digitalWrite(netPin[i], 1);
		}
	}
	piUnlock(0);
}

//**************************************************************************
// Thread entry point, param is not used
void *cputhread(void *param)
{
	if (maxCpuPin<0)
		return 0;
	do
	{
		setUtLed(CPU,cpu_usage());
		Sleep(200);
	} while (kicked==0);  // exit loop if flag set	
	return 0;
}

//**************************************************************************
// Thread entry point, param is not used
void *iothread(void *param)
{
	if (maxIoPin<0)
		return 0;
	do
	{
		setUtLed(IO,io_usage(ioDevice));
		Sleep(200);
	} while (kicked==0);  // exit loop if flag set	
	return 0;
}

//**************************************************************************
// Thread entry point, param is not used
void *netthread(void *param)
{
	if (maxNetPin<0)
		return 0;
	do
	{
		setUtLed(NET,net_usage(netDevice));
		Sleep(200);
	} while (kicked==0);  // exit loop if flag set	
	return 0;
}

//************************************************************************
void pinInit(int pin)
{
	pinMode (pin, OUTPUT);
	digitalWrite(pin, 1);
}

//************************************************************************
// and finally, the main program
// any cmd line parameter will cause it to run in the foreground 
// instead of as a daemon 
// will run diagnostic mode is parameter is "d"
int main(int argc, char *argv[])
{
    pid_t		pid;
	pthread_t	tid1,tid2,tid3;		// thread IDs
	FILE		*f;
	int 		x;

	LogOpen(IDENTIFIER);

	// check cmd line params and run as daemon if none
	if (argc==1)
	{
		LogDbg("Starting\n");
		// Spawn off a child, then kill the parent.
		// child will then have no controlling terminals, 
		// and will become adopted by the init proccess.
		if ((pid = fork()) < 0) {
			perror("Error forking process ");
			Log("Error forking process ");
			exit (-1);
		}
		else if (pid != 0) {
			exit (0);  // parent process goes bye bye
		}

		// The child process continues from here
		setsid();  // Become session leader;
	}

	// trap some signals 
	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGPWR, sig_handler);
	signal(SIGHUP, sig_handler);

	// save the pid in a file
	pid = getpid();
	f = fopen(PIDFILE,"w");
	if (f) {
		fprintf(f,"%d",pid);
		fclose(f);
	}

	// initialize the WireingPi interface
	LogDbg("Initialize WiringPi interface\n");
	x = wiringPiSetup ();
	Log("wiringPiSetup returned %d\n",x);
	if (x == -1)
	{
		Log("wiringPiSetup returned %d\n",x);
		Log("Error on wiringPiSetup.  Program exitting\n");
		return 0;
	}

	// read the configuration file
	ReadConfigFile(CONFIGFILE);

	// run diagnostic mode
	if ((argc>1) && !strncmp(argv[1],"d",1))
	{
		printf("Running Diagnostic Mode\npress ctrl-C to stop\n\n");
		diag_mode();
		return 0;
	}
	
	// blink LEDs to announce our arrival
	for (x=0; x<5; x++)
	{
		setUtLed(CPU,100);
		setUtLed(IO,100);
		setUtLed(NET,100);
		Sleep(333);
		setUtLed(CPU,0);
		setUtLed(IO,0);
		setUtLed(NET,0);
	}
	
	// start the main loop
	do
	{
		// start the various threads
		LogDbg("Main> start threads\n");
		tid1 = tid2 = tid3 = 0;
		pthread_create(&tid1, NULL, cputhread, NULL);
		pthread_create(&tid2, NULL, iothread, NULL);
		pthread_create(&tid3, NULL, netthread, NULL);
		sleep(5);
		
		// wait for signal to restart or exit
		do
		{
			sleep(1);
		} while (!kicked);
		
		// wait for running threads to stop
		if (tid1!=0) pthread_join(tid1, NULL);
		if (tid2!=0) pthread_join(tid2, NULL);
		if (tid3!=0) pthread_join(tid3, NULL);

		// exit?
		if (kicked==2) break;
		// else restart, set flag back to 0
		kicked = 0;

	} while (1); // forever

	// blink LEDs to announce our exit
	for (x=0; x<5; x++)
	{
		setUtLed(CPU,100);
		setUtLed(IO,100);
		setUtLed(NET,100);
		Sleep(333);
		setUtLed(CPU,0);
		setUtLed(IO,0);
		setUtLed(NET,0);
	}

	
	// delete the PID file
	unlink(PIDFILE);

	LogDbg("Program Completed");
	return 0;
}
