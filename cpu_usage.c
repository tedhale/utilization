/*
	Get the current CPU usage as a percentage value
	using the /proc filesystem
	
	18-Mar-2014   ted.b.hale@gmail.com  
	http://raspberrypihobbyist.blogspot.com/

Notes:
filename: /proc/stat
example:
cpu  55366 271 17283 75381807 22953 13468 94542 0
meaning:
1 - user: cpu time executing in user mode, normal priority
2 - nice: cpu time executing in user mode, "niced" priority
3 - system: cpu executing in kernel mode
4 - idle: spinning wheels doing nothing

This function tracks the change in these values between calls 
to compute CPU usage.  i.e. it returns the CPU usage during the 
period since the last time it was called.

Only gives total CPU time.  Can be modified for multiprocessor systems.
Assumes the CPU line is first in /proc/stat, but I don't expect that to change.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// static here means these variable are not visible from other modules
// these variables will hold their value between calls of the function
static ulong ux = 0;					 
static ulong nx = 0;					
static ulong kx = 0;
static ulong ix = 0;
	
int cpu_usage()
{ 
    char buff[100];
	FILE* file;
	char *p, *ctx;
	ulong usage, total;
	ulong u, n, k, i;

	file = fopen("/proc/stat", "r");
	if (file==NULL) return -1;
	fread(buff,sizeof(buff),1,file);	// read the first line and more
	buff[sizeof(buff)-1] = 0;   		// guarantee null termination
	fclose(file);

	p = strtok_r(buff," ",&ctx);		// get first token and ignore it
	if (p==NULL) return -1;
	p = strtok_r(NULL," ",&ctx);		// get user mode usage
	u = atol(p);
	
	p = strtok_r(NULL," ",&ctx);		// get nice mode usage
	if (p==NULL) return -1;
	n = atol(p);
	
	p = strtok_r(NULL," ",&ctx);		// get kernel mode usage
	if (p==NULL) return -1;
	k = atol(p);
	
	p = strtok_r(NULL," ",&ctx);		// get idle usage
	if (p==NULL) return -1;
	i = atol(p);

	usage = (u-ux) + (n-nx) + (k-kx);
	total = usage + (i-ix);
	if (total==0) return 0;
	
	ux = u;								// save values for next call
	nx = n;
	kx = k;
	ix = i;
	
    return (int)(100*usage/total);

}
