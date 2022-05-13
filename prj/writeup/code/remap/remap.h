/* $Id: remap.h,v 1.2 2000/04/13 13:28:19 proff Exp $
 * $Copyright:$
 */

#ifndef REMAP_H
#define REMAP_H

#include "remappers.h"
//#include "remap.ext"

extern int  isZero(char *data, int len);
extern maruRemapDesc *remapLookupStr(char *str);
extern maruRemapDesc *remapLookupType(maruRemapType type);
extern int  maruRemapIo(maruReq *req);

#endif /* REMAP_H */
