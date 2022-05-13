/* $Id: maru_types.h,v 1.1 2000/04/19 16:51:22 proff Exp $
 * $Copyright:$
 */
#ifndef MARU_TYPES_H
#define MARU_TYPES_H

#ifdef MARU_MOD
#ifndef linux
#  include <sys/cdefs.h>
#  include <sys/param.h>
#  include <sys/systm.h>
#  include <sys/kernel.h>
#endif
//Needs to be 'linux' for kernel module compilation
// 'sys' otherwise; e.g. <linux/types.h> or <sys/types.h>
#else
#  include <sys/types.h>
#  ifdef HAVE_SYS_PARAM_H
#    include <sys/param.h>
#  endif
#endif

//#include "maru_config.h"
#define bool	int
#define TRUE	1
#define FALSE	0
#define ABORT	-1

typedef char m_8;

typedef unsigned char m_u8;

typedef short m_16;

typedef unsigned short m_u16;

typedef long m_32;

typedef unsigned long m_u32;

typedef long long m_64;

typedef unsigned long long m_u64;

#ifndef MIN
#  define MIN(x,y) (((x)<(y))? (x): (y))
#endif
#ifndef MAX
#  define MAX(x,y) (((x)>(y))? (x): (y))
#endif

#define EITHER(x,y) ((x)? (x): (y))
#if 1
#ifndef NULL
#  define NULL ((void *)0)
#endif

#define STR(x) #x

#define PACKED __attribute__ ((__packed__))
#define NORETURN __attribute__ ((__noreturn__))

static inline m_u8 hton8(m_u8 x) {return x;}
#define ntoh8 hton8

static inline m_u16 hton16(m_u16 x) {return 
#ifdef WORDS_BIGENDIAN
				       x
#else
				       (m_u16)hton8(x>>8) | ((m_u16)hton8(x) << 8)
#endif
				       ;}
#define ntoh16 hton16

static inline m_u32 hton32(m_u32 x) {return 
#ifdef WORDS_BIGENDIAN
				       x
#else
				       (m_u32)hton16(x>>16) | ((m_u32)hton16(x) << 16)
#endif
				       ;}
#define ntoh32 hton32

static inline m_u64 hton64(m_u64 x) {return 
#ifdef WORDS_BIGENDIAN
				       x
#else
				       (m_u64)hton32(x>>32) | ((m_u64)hton32(x) << 32)
#endif
				       ;}
#define ntoh64 hton64

#define SECURE			struct
#define END_SECURE(label)	*label = maruMalloc(sizeof *label)
#endif
#endif /* MARU_TYPES_H */
