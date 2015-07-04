/******************************************************************************
   Filename:   logfile_syslog.c

	 20-Mar-2014   Ted Hale  created
				compatible with my old Logfile, which is now logfile_file.c

******************************************************************************/

/* system includes */
#include <stdio.h>
#include <stdlib.h>  
#include <stdarg.h>
#include <syslog.h>
#include <string.h>

/* global data*/

static char *g_ident = NULL;    
static int pri_dbg = LOG_MAKEPRI(LOG_DAEMON, LOG_WARNING);
static int pri_err = LOG_MAKEPRI(LOG_DAEMON, LOG_ERR);

/*
  CloseLogFile

  Closes syslog if it is in use.
*/
void LogClose(void) 
{
	if (g_ident)
	{
		closelog();
		free(g_ident);
		g_ident = NULL;
	}
}     

/*
	OpenLogFile
	ident - string to pass into lopenlog as the identification string
	RETURNS: 0

*/
int LogOpen(char *ident) 
{
	g_ident = strdup(ident);
	setlogmask (LOG_UPTO (LOG_DEBUG));
	openlog (g_ident, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	return (0);
}

/*
   Log
   Writes a message to the previously opened log.
   Passed in a format string and variable number of parameters,
   in the format used by printf.
*/
void Log(char *format, ... ) 
{
	va_list arglist;

	if (!g_ident) return;
	va_start(arglist, format);
	vsyslog(pri_err, format, arglist);
}

/*
	LogDbg
	Same as Log, but used for debug level messages
*/
void LogDbg(char *format, ... ) 
{
	va_list arglist;

	if (!g_ident) return;
	va_start(arglist, format);
	vsyslog(pri_dbg, format, arglist);
}
