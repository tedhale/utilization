/*---------------------------------------------------------------------------
  diagmode.c
  By Ted B. Hale
	CPU and IO utilization display daemon	
    
	This file implements the diagnostic mode 

  2014-04-01  diagnostic mode moved to separate file
  2014-04-02  mod for version with config file
  
---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <wiringPi.h>

#include "utilization.h"

void Sleep(int ms);
void setUtLed(int ledType, int val);

//************************************************************************
void diag_blink(int pin)
{
	digitalWrite(pin, 0);
	Sleep(500);
	digitalWrite(pin, 1);
}

//************************************************************************
// diagnostic mode
void diag_mode()
{
	int i;

	while (1)
	{
		// all on
		setUtLed(CPU,100);
		setUtLed(IO,100);
		Sleep(1000);
		setUtLed(CPU,0);
		setUtLed(IO,0);
		if (kicked==2) return;
		Sleep(500);

		// step up cpu
		for (i=0; i<=maxCpuPin; i++)
		{
			setUtLed(CPU,cpuThresh[i]+1);
			Sleep(500);
		}
		Sleep(500);
		setUtLed(CPU,0);
		if (kicked==2) return;
		Sleep(500);
	
		// step up io
		for (i=0; i<maxIoPin; i++)
		{
			setUtLed(IO,ioThresh[i]+1);
			Sleep(500);
		}
		Sleep(500);
		setUtLed(IO,0);
		if (kicked==2) return;
		Sleep(500);

		// step up both
		for (i=0; ((i<=maxCpuPin)||(i<=maxIoPin)); i++)
		{
			setUtLed(CPU,cpuThresh[i]+1);
			setUtLed(IO,ioThresh[i]+1);
			Sleep(500);
		}
		Sleep(500);
		setUtLed(CPU,0);
		setUtLed(IO,0);
		if (kicked==2) return;
		Sleep(500);
		
		// run up cpu
		for (i=0; i<=maxCpuPin; i++)
		{
			diag_blink(cpuPin[i]);
		}
		if (kicked==2) return;
		Sleep(500);
		
		// run up io
		for (i=0; i<=maxIoPin; i++)
		{
			diag_blink(ioPin[i]);
		}
		if (kicked==2) return;
		Sleep(500);
	}

	return;
}
