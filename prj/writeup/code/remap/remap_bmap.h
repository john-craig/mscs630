/* $Id: remap_bmap.h,v 1.1 2000/05/02 20:05:51 proff Exp $
 * $Copyright:$
 */

#ifndef REMAP_BMAP_H
#define REMAP_BMAP_H

#include "remappers.h"
//#include "remap_bmap.ext"

extern  m_u32 maruRemapBmapSize(m_u32 blocks);
extern  maruKeymapAspectRemap* maruRemapBmapCreate(maruInstance *i, m_u32 blocks, int *lenp, maruRemapFlags remap_flags);
extern  void* maruRemapBmapNew(maruInstance *i, maruRemapFlags remap_flags);
extern  void* maruRemapBmapAddAspect(maruAspect *a, maruKeymapAspectRemap *hr);
extern  void maruRemapBmapReleaseAspect(maruAspect *a);
extern  void maruRemapBmapFree(maruInstance *i);
extern  int  maruRemapBmapMapIO(maruReq *req);
extern  void maruRemapBmapInfo(maruInstance *i, int as, maruRemapInfoFlag flag);
extern  int  maruRemapBmapSync(maruInstance *i);

#endif /* REMAP_BMAP_H */
