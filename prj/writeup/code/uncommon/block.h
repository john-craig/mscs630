/* $Id: block.h,v 1.2 2000/04/07 17:23:41 ralf Exp $
 */
#ifndef _BLOCK_H
#define _BLOCK_H

//#include "block.ext"
extern  int maru_set_blocksize(int fd, int blocksize);
extern  int maru_handle_chunk(char *data, m_u64 offset, m_u32 length, maruReqOp operation, maruAspect *aspect);

#endif /* _BLOCK_H */
