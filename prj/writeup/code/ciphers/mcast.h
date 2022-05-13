/* $Id: mcast.h,v 1.1 2000/04/19 16:51:22 proff Exp $
 * $Copyright:$
 */
#ifndef MCAST_H
#define MCAST_H

#include "cast.h"

typedef cast_key CASTcontext;

//#include "mcast.ext"
extern  void CASTinit(maruCipherDesc *cipher, void *opaque, int flags);
extern  void CASTsetkey(void *opaque, u_char *key, int len, int flags);
extern  void CASTencryptCBCTo(void *opaque, u_char *iv, u_char *data, u_char *to, int len);
extern  void CASTcryptCBC(void *opaque, u_char *iv, u_char *data, u_char *to, int len, int flags);

#endif
