SRCS=cpu_usage.c io_usage.c main.c logfile_syslog.c diagmode.c readconfig.c net_usage.c

OBJS=$(SRCS:.c=.o)

CC=gcc
DEBUG  = -g -O0
#DEBUG   = -O3
INCLUDE = -I/usr/local/include
LDFLAGS = -L/usr/local/lib 
LDLIBS  = -lpthread -lm -lwiringPi -lwiringPiDev 
CFLAGS  = $(DEBUG) -Wall $(INCLUDE) -Winline -pipe

LD=gcc

.c.o:
	@echo [CC] $<
	@$(CC) -c $(CFLAGS) $< -o $@
	
all: utilization

utilization: $(OBJS)
	@$(CC) -o utilization $(OBJS) $(LDFLAGS) $(LDLIBS) 

diagmode.o: utilization.h

main.o: log.h utilization.h

readconfig.o: log.h utilization.h

clean:
	rm -f $(OBJS) utilization core 

