/*---------------------------------------------------------------------------
  readconfig.c
  By Ted B. Hale
	CPU and IO utilization display daemon	
	 
	This file processes the configuration file 

  2014-04-01  initial edits
  2014-12-12  add network usage support  
  
---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <mcp23017.h>
#include <mcp23008.h>

#include "log.h"
#include "utilization.h"

//**************************************************************************
void ProcessConfigLine(char *line)
{
	char *var;
	char *val;
	char *p;
	int i, addr, pin;

	if (*line=='#') return;
	var = line;
	val = strchr(line,'=');
	if (val==NULL) return;
	*val = 0;
	val++;

	if (!strcmp(var,"ioDevice"))
	{
		ioDevice = strdup(val);
		ioDevice[strlen(ioDevice)-1] = 0;
		return;
	}
	
	if (!strcmp(var,"netDevice"))
	{
		netDevice = strdup(val);
		netDevice[strlen(netDevice)-1] = 0;
		return;
	}
	
	// handle init of MCP23017
	if (!strcmp(var,"InitMCP23017"))
	{
		p = strchr(val,',');
		if (p==NULL) return;
		*p=0; 
		p++;
		addr = atoi(val);
		pin = atoi(p);
		mcp23017Setup(pin,addr);
		return;
	}

	// handle init of MCP2308
	if (!strcmp(var,"InitMCP2308"))
	{
		p = strchr(val,',');
		if (p==NULL) return;
		*p=0; 
		p++;
		addr = atoi(val);
		pin = atoi(p);
		mcp23008Setup(pin,addr);
		return;
	}
	
	// handle CPU pins
	if (!strncmp(var,"cpuPin",6))
	{
		i = atoi(var+6);
		if (i>maxCpuPin) maxCpuPin = i;
		cpuPin[i] = atoi(val);
		p = strchr(val,',');
		if (p==NULL) return;
		cpuThresh[i] = atoi(p+1);
		pinInit(cpuPin[i]);
		return;
	}

	// handle I/O pins
	if (!strncmp(var,"ioPin",5))
	{
		i = atoi(var+5);
		if (i>maxIoPin) maxIoPin = i;
		ioPin[i] = atoi(val);
		p = strchr(val,',');
		if (p==NULL) return;
		ioThresh[i] = atoi(p+1);
		pinInit(ioPin[i]);
		return;
	}

	// handle Net pins
	if (!strncmp(var,"netPin",6))
	{
		i = atoi(var+6);
		if (i>maxNetPin) maxNetPin = i;
		netPin[i] = atoi(val);
		p = strchr(val,',');
		if (p==NULL) return;
		netThresh[i] = atoi(p+1);
		pinInit(netPin[i]);
		return;
	}

	return;
}	
	

//**************************************************************************
void ReadConfigFile(char *filename)
{

	FILE *f;
	char buff[100];

	maxNetPin = maxIoPin = maxCpuPin = -1;
	
	f = fopen (filename,"r");
	if (f == NULL) 
	{
		Log("error opening config file\n");
		return;
	}
	while (!feof(f) && (fgets(buff,sizeof(buff),f) != NULL))
	{
		ProcessConfigLine(buff);
	}
	fclose (f);
	return;
}
