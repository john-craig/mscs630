#include <stdarg.h>
#include <stdio.h>
#include "assert.h"

extern void maruFatal(char *fmt, ...)
{
    int retval;
    va_list ap;
    char buf[2048];
    va_start(ap, fmt);
    retval = vprintf(fmt, ap);
    buf[retval] = '\0';
    va_end(ap);
    //panic(buf);
}