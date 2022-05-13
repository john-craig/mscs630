/* $Id: mSSLeay.h,v 1.1 2000/04/19 16:51:22 proff Exp $
 * $Copyright:$
 */
#ifndef MSSLEAY_H
#define MSSLEAY_H

//#include "mevp.h"
#include "openssl/evp.h"

typedef struct
{
    maruCipherDesc *cipher;
    EVP_CIPHER_CTX sslCtx;
} SSLEAYcontext;

//#include "mSSLeay.ext"
extern  void SSLEAYinit(maruCipherDesc *cipher, void *opaque, int flags);
extern  void SSLEAYsetkey(void *opaque, u_char *key, int len, int flags);
extern  void SSLEAYcrypt(void *opaque, u_char *iv, u_char *data, u_char *to, int len, int flags);
extern  void SSLEAYstir(void *opaque, int rounds);

#endif
