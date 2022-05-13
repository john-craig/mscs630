/* $Id: pattern.h,v 1.2 1999/09/09 07:43:28 proff Exp $
 * $Copyright:$
 */
#ifndef PATTERN_H
#define PATTERN_H


//#include "pattern.ext"
extern  unsigned char match (char *p, char *text, int f_case, char eol);
extern  unsigned char ispattern (char *p);

#endif /* PATTERN_H */
