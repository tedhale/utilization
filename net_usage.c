/*
	Get the network usage as a percentage value
	using the /proc filesystem  
	
	12-Dec-2014   ted.b.hale@gmail.com  
	http://raspberrypihobbyist.blogspot.com/


/proc/net/dev
find line with interface name as first field, then
Field  2 -- # of bytes recieved
Field  10 -- # of bytes sent

This function tracks the change in the total of these values between 
calls to compute network usage.  i.e. it returns the usage during the 
period since the last time it was called.

ISSUE: this only works for a single device.  Would need a context structure 
passed in that holds what is now in static variables to support multiple devices.

ALSO: there is no easy way to know what the max I/O value should be, so this
is tracked dynamically.  io_set_max can be used to set an initial value if wanted.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// static here means these variable are not visible from other modules
// these variables will hold their value between calls of the function
static ulong last_net_total = 0;
static ulong net_max = 1;

int net_usage(char *interface_name)
{
    char buff[200];
	FILE* file;
	char *p, *ctx;
	ulong current, total;
	ulong bytes_in = 0;
	ulong bytes_out = 0;
	int i;
	
	// build the file name
	file = fopen("/proc/net/dev", "r");
	if (file==NULL) return -1;
	// find line with interface name
	do
	{
		if (feof(file))
		{
			fclose(file);
			return -1;
		}
		fgets(buff,sizeof(buff)-1,file);
		if (strstr(buff,interface_name)!=0)
			break;
	} while (1);
	
	fclose(file);

	p = strtok_r(buff," ",&ctx);		// get first token and ignore it
	for (i=1; i<11; i++)				// loop through rest of tokens
	{
		p = strtok_r(NULL," ",&ctx);    // get next token
		if (p==NULL) return -1;
		if (i==1)						// counting from zero 
			bytes_in = atol(p);
		if (i==9)						// counting from zero
			bytes_out = atol(p);
	}
	total = bytes_in + bytes_out;
	if (last_net_total==0) 
	{
		last_net_total = total;
		return 0;
	}
	current = total - last_net_total;
	last_net_total = total;
	if (current>net_max) net_max = current;
	if (net_max==0) return 0;
	return (int)(100*current/net_max);
}

void net_set_max(ulong newmax)
{
	net_max = newmax;
}

ulong net_get_max()
{
	return net_max;
}
