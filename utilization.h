/*---------------------------------------------------------------------------
  utilization.h
  By Ted B. Hale
	CPU and IO utilization display daemon	
    
	This file is the shared include 

  2014-04-02  initial edits
  2014-12-12  add network usage support
  
---------------------------------------------------------------------------*/
// causes Global variables to be defined in the main
// and referenced as extern in all the other source files
#ifndef EXTERN
#define EXTERN extern
#endif

#define CPU 0
#define IO 1
#define NET 2

int cpu_usage();
int io_usage(char *device);
int net_usage(char *device);
void diag_mode();
void pinInit(int pin);
void ReadConfigFile(char *filename);

EXTERN int kicked;
EXTERN char *ioDevice;
EXTERN char *netDevice;
EXTERN int maxCpuPin;
EXTERN int maxIoPin;
EXTERN int maxNetPin;
EXTERN int cpuPin[10];
EXTERN int cpuThresh[10];
EXTERN int ioPin[10];
EXTERN int ioThresh[10];
EXTERN int netPin[10];
EXTERN int netThresh[10];

