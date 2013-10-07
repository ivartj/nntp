#include "msg.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define MSGMAXLEN 256

char msg[MSGMAXLEN] = { 0 };

void msg_set(const char *fmt, ...)
{
	va_list ap;
	char tmp[MSGMAXLEN];

	va_start(ap, fmt);
	vsnprintf(tmp, MSGMAXLEN, fmt, ap);
	memcpy(msg, tmp, MSGMAXLEN);
	va_end(ap);
}

char *msg_get(void)
{
	return msg;
}

void vmsg_log(const char *fmt, va_list ap)
{
	vfprintf(stderr, fmt, ap);
	fputs(".\n", stderr);
}

void msg_log(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vmsg_log(fmt, ap);
	va_end(ap);
}

void msg_fail(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vmsg_log(fmt, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}
