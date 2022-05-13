/* $Id: assert.h,v 1.2 2000/05/02 20:03:20 proff Exp $
 * $Copyright$
 */

#ifndef ASSERT_H
#define ASSERT_H

#include "maru/maru_types.h"
#include <stdint.h>

extern void maruFatal(char *fmt, ...);

#define assert(x) \
	if (!(x))\
        	maruFatal("%s:%s:%d failed assertion: %s\n", __FILE__, __FUNCTION__, __LINE__, #x)

#define NOTREACHED \
        	maruFatal("%s:%s:%d the twisting lane!\n", __FILE__, __FUNCTION__, __LINE__)

#define aligned(x, y) ((((int)(uintptr_t)(x)) % (y)) == 0)

#endif /* ASSERT_H */
