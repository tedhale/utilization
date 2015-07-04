/*
	Get the device I/O usage as a percentage value
	using the /sys filesystem
	
	19-Mar-2014   ted.b.hale@gmail.com  
	http://raspberrypihobbyist.blogspot.com/

	24-Nov-2014  prevent div by zero

/sys/block/DEVNAME/stat
Field  3 -- # of sectors read
Field  7 -- # of sectors written

This function tracks the change in the total of these values between 
calls to compute I/O usage.  i.e. it returns the I/O usage during the 
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
static ulong last_io_total = 0;
static ulong io_max = 1;

int io_usage(char *device)
{
    char buff[200];
	FILE* file;
	char *p, *ctx;
	ulong current, total, length;
	ulong sectors_read = 0;
	ulong sectors_written = 0;
	int i;
	
	// build the file name
	snprintf(buff,sizeof(buff),"/sys/block/%s/stat",device);
	buff[sizeof(buff)-1] = 0;  			// guarantee null termination
	file = fopen(buff, "r");
	if (file==NULL) return -1;
	length = fread(buff,sizeof(buff),1,file);	// read the file
	buff[length-1] = 0;   				// guarantee null termination
	fclose(file);

	p = strtok_r(buff," ",&ctx);		// get first token and ignore it
	for (i=1; i<7; i++)					// loop through rest of tokens
	{
		p = strtok_r(NULL," ",&ctx);    // get next token
		if (p==NULL) return -1;
		if (i==2)						// counting from zero so 3rd token
			sectors_read = atol(p);
		if (i==6)						// counting from zero so 7th token
			sectors_written = atol(p);
	}
	total = sectors_read + sectors_written;
	if (last_io_total==0) 
	{
		last_io_total = total;
		return 0;
	}
	current = total - last_io_total;
	last_io_total = total;
	if (current>io_max) io_max = current;
	if (io_max==0) return 0;
	return (int)(100*current/io_max);
}

void io_set_max(ulong newmax)
{
	io_max = newmax;
}

ulong io_get_max()
{
	return io_max;
}
