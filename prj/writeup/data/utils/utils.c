#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <err.h>

#include "utils.h"

#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#else
#  include <time.h>
#endif



#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>

#ifndef HAVE_GETPAGESIZE
# include <sys/param.h>
# ifdef EXEC_PAGESIZE
#  define pagesize EXEC_PAGESIZE
# else
#  ifdef NBPG
#   define pagesize NBPG * CLSIZE
#   ifndef CLSIZE
#    define CLSIZE 1
#   endif
#  else
#   ifdef NBPC
#    define pagesize NBPC
#   else
#    ifdef PAGE_SIZE
#     define pagesize PAGE_SIZE
#    else
#     define pagesize PAGESIZE /* SVR4 */
#    endif
#   endif
#  endif
# endif

int getpagesize ()
{
    return pagesize;
}
#endif


void *xmalloc(int len)
{
    void *p;
    /* fix: under linux we need to align the length of the memory area we're trying to allocate
     *      to a multiple of the pagesize - otherwise can't lock those memory lock these areas.
     *  --rpw
     */
//#ifdef linux
    int psize = getpagesize();
    if (len % psize)
        len += psize - len % psize;
//#endif

    p = malloc(len );
    if (!p)
	err(1, "malloc");
    return p;
}

//#ifdef HAVE_MLOCK
bool f_lockMem = TRUE;
static bool f_lockedAllFutureMem = FALSE;

bool lockAllMem(void)
{
    if (!f_lockMem ||
	f_lockedAllFutureMem)
	return TRUE;
    if (mlockall(MCL_FUTURE != 0))
	{
	    warnx("couldn't mlockall(MCL_FUTURE) -- not root?");
	    return FALSE;
	}
    else
	{
	    f_lockedAllFutureMem = TRUE;
	    return TRUE;
	}
}

void xmlock(void *p, int len)
{
    static int errs;
    int psize;
    if (!f_lockMem || f_lockedAllFutureMem)
	return;
    psize = getpagesize();
    if (len % psize)
        len += psize - len % psize;

    if (mlock(p, len) != 0 && errs++ == 0)
	warn("couldn't lock %d of data in memory", len);
}

void xmunlock(void *p, int len)
{
    int psize;

    if (!f_lockMem || f_lockedAllFutureMem)
	return;

    psize = getpagesize();

    if (len % psize)
        len += psize - len % psize;

    /* important: don't call exit() in here, as we can be called from the atexit() handler */
    munlock(p, len);
}

//#endif /* HAVE_MLOCK */
