#include "maru/maru_types.h"

#ifndef UTILS_H
#define UTILS_H

extern int getpagesize();
extern void *xmalloc(int len);
extern bool lockAllMem(void);
extern void xmlock(void *p, int len);
extern void xmunlock(void *p, int len);

extern bool f_lockMem;

#endif /* UTILS_H */
