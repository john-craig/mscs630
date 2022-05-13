extern void XORinit(maruCipherDesc *cipher, void *opaque, int flags);
extern void XORsetkey(void *opaque, u_char *key, int len, int flags);
extern void XORcrypt(void *opaque, u_char *iv, u_char *data, u_char *to, int len, int flags);