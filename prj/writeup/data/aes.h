#ifndef AES_H
#define AES_H

#include "../src/maru/maru.h"

#define AES_BLCK   16                   /* Block length in bytes */
#define AES_KEY    16                   /* Key length in bytes */


/* Blowfish context */
typedef struct
{
    m_u8 K[4][44];    /* Key rounds matrix */
} aes_ctx;

// //#include "AES.ext"
extern  void AESinit(maruCipherDesc *cipher, void *opaque, int flags);
extern  void AESsetkey(void *opaque, u_char *key, int len, int flags);
extern  void AESencryptCBC(void *opaque, u_char *iv, u_char *data, int len);
//extern  void AESsetkey(m_u8 *opaque, u_char *key, int len, int flags);
//extern  void AESencryptCBC(m_u8 *opaque, u_char *iv, u_char *data, int len);
// extern  void AESencryptCBCTo(void *opaque, u_char *iv, u_char *data, u_char *to, int len);
extern  void AESdecryptCBC(void *opaque, u_char *iv, u_char *data, int len);
extern  void AESdecryptCBCTo(void *opaque, u_char *iv, u_char *data, u_char *to, int len);
extern  void AEScryptCBC(void *opaque, u_char *iv, u_char *data, u_char *to, int len, int flags);
//extern m_u8 *AESencrypt(m_u8 *keyHex, m_u8 *pTextHex, int keyLen, int pTextLen);
//extern m_u8 *AESdecrypt(m_u8 *keyHex, m_u8 *cTextHex, int keyLen, int cTextLen);


#endif /* AES_H */
