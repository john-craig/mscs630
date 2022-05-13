#ifndef CLIENT_COMMON_H
#define CLIENT_COMMON_H

#ifndef _PATH_RANDOM
#  define _PATH_RANDOM "/dev/random"
#endif
#ifndef _PATH_URANDOM
#  define _PATH_URANDOM "/dev/urandom"
#endif

typedef enum {RAND_PSEUDO, RAND_TRUE} maru_random;

#include "maru/maru_types.h"
#include "utils/utils.h"
#include "remap/remap.h"
//Removed for kernel module compilation
#include <stdio.h>

extern  int a_debug ;
extern  int a_force ;
extern  volatile char *stackP ;

extern void freeWipeList(bool nofree);
extern void *maruMalloc(int len);
extern void *maruCalloc(int len);
extern void maruFree(void *p);
extern void maruWipeFree(void *p);
extern int matoi(char *s);
extern int xmatoi(char *s);
extern m_u32 simpleSum(u_char *p, int len);
extern void maruRandom(void *buf, int n, maru_random type);
extern m_u32 maruRandom32();
extern void maruRandomf(void *data, int len, maru_random type);
extern void maruWipe(void *p, int n);
extern  u_char *loadFile(char *name, int *len);
extern  maruKeymap *loadKeymap(char *name, int *klenp);
extern  void makeKeymapSum(maruKeymap *h, int len);
extern  void saveKeymap(char *name, maruKeymap *h, int len);
extern  void syncInstance(maruInstance *i);
extern  void freeInstance(maruInstance *i);
extern  __attribute__ ((__noreturn__))  void wipeStackExit()  ;
extern  void freeAspect(maruAspect *a);
extern  maruAspect *buildAspect(maruInstance *i, maruKeymapAspect *h, int as, maruPass *key, int keylen);
extern  int  validKeymap(maruKeymap *h, int len);
extern  maruInstance *instanceNew(maruKeymap *h, int klen, maruRemapFlags remap_flags);
extern  maruAspect *getAspect(maruInstance *i, int as);
extern  int getKeymapAspectLen(maruInstance *i);
extern  maruKeymapAspect *getKeymapAspect(maruInstance *i, maruKeymap *keymap, int as);
extern  maruInstance *genInstance(char *fn, maruRemapFlags remap_flags);
extern  void listCiphers(FILE *fh);
extern  maruCipherDesc *xfindCipherTxt(char *txt);
extern  maruCipherDesc *xfindCipherType(maruCipher cipher);
extern  void nocore();
extern  void maruExitHandler();
extern  void nosignals();

extern  int  f_timestampHack ;
extern  int Iam1970(char *fn);

extern bool waitForEntropy;
extern bool f_wipeMem;

#endif
