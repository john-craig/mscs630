/* $Id: common.h,v 1.1 2000/04/19 16:51:22 proff Exp $
 * $Copyright:$
 */
#ifndef COMMON_H
#define COMMON_H

#ifdef linux
#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/config.h>
#include <linux/string.h>
#include <asm/byteorder.h>
#endif
#endif

//#include "common.ext"
extern  void *maruOpaqueInit(maruCipherDesc *cipher);
extern  void maruOpaqueFree(void *opaque);
extern  void xor(void *vp, void *vp1, int len);
extern  void int2char(m_u32 i, u_char *p);
extern  m_u32 char2int(u_char *p);
extern  maruCipherDesc *findCipherType(maruCipher cipher);
extern  maruCipherDesc *findCipherTxt(char *txt);
extern  void maruGenBlockKey(maruAspect *a, maruKey *key, int keylen, m_u32 blockno);
extern  void maruEncryptBlock(maruAspect *a, u_char *block, u_char *to, int len, m_u32 blockno, int flags);


#endif
