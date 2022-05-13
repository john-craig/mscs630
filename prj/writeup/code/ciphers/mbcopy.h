/* $Id: mbcopy.h,v 1.1 2000/04/19 16:51:22 proff Exp $
 * $Copyright:$
 */
#ifndef MBCOPY_H
#define MBCOPY_H

//#include "mbcopy.ext"
extern  void bcopyinit(maruCipherDesc *cipher, void *opaque, int flags);
extern  void bcopysetkey(void *opaque, u_char *key, int len, int flags);
extern  void bcopycrypt(void *opaque, u_char *iv, u_char *data, u_char *to, int len, int flags);

#endif
