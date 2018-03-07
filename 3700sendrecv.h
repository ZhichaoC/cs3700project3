#ifndef __3700SENDRECV_H__
#define __3700SENDRECV_H__

#include <stdio.h>
#include <stdarg.h>

//handle packet
void dump_packet(unsigned char *data, int size);
//time stamp
char *timestamp();
//log event
void mylog(char *fmt, ...);

#endif

