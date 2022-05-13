#ifndef STR_H
#define STR_H

#include <string.h>

struct strList
{
	struct strList *next;
	struct strList *head;
	char *data;
};

struct strBinList
{
	struct strBinList *right;
	struct strBinList *left;
	char *data;
};

struct strStack
{
	char *data;
	int used;
	int len;
};

/* we don't do the *x==*y trick, as it doesn't take kindly to functions */
#define strEq(x,y) (strcmp((x), (y)) == 0)
#define strnEq(x,y,z) (strncmp((x), (y), (z)) == 0)
#ifdef HAVE_STRCASECMP
#  define strCaseEq(x,y) (strcasecmp((x), (y)) == 0)
#  define strnCaseEq(x,y,z) (strncasecmp((x), (y), (z)) == 0)
#endif

extern  char *xstrdup (char *s);
extern  int strExchange (char *s, char c1, char c2);
extern  int strLower (char *s);
extern  int strUpper (char *s);
extern  int strSnip (char *s, int len, char *start, char *end, char *buf, int blen);
extern  char *strncasestr (char *s, char *find, int slen);
extern  int strStripLeftRight (char *s);
extern  int strStripEOL (char *s);
extern  int strnStripEOL (char *s, int n);
extern  int strMakeEOLn (char *s);
extern  int strMakeEOLrn (char *s);
extern  unsigned long strHash (unsigned long h, char *s);
extern  int strToi (char *s);
extern  int strKToi(char *s, int *i);
extern  char *conv (double n);
extern  int strToVec(char *p, char **cp, int cpnum);
extern  int hexToBin(char *in, char *out, int len);

#endif /* STR_H */