#ifndef _MARU_LINUX_H
#define _MARU_LINUX_H

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/stat.h>
#include <linux/errno.h>
#include <linux/major.h>
#include <linux/init.h>
#include <linux/slab.h>

#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>

#include <asm/uaccess.h>
typedef enum
{
    MUF_INITED 		= 1<<0,		/* unit has been initialised */
    MUF_READONLY	= 1<<1,		/* unit is marked read-only */
    MUF_PERMISSIVE      = 1<<2          /* unit allows processes with different euid to read/write */
} maru_flags;

/* a token queue to associate block device
 * read requests and the (async) answer from userland
 */
#define NUM_TOKENS	(NMARU * 8)

struct maru_request {
    int     status;
    m_u64   buflen;
    void    *buffer;
};

struct maru_token
{
    m_u32 		qt_id;
    struct semaphore	qt_lock;
    struct maru_softc   *qt_sc;
    struct maru_request *qt_req;
    // int			qt_status;
    // m_u64		qt_buflen;
    // void	       *qt_buf;
};

/* XXX align me properly ! */
struct maru_softc
{
    spinlock_t lock;
    struct gendisk  *maru_disk;
    struct blk_mq_tag_set tag_set;
    struct request_queue *maru_queue;

    maru_flags	sc_flags;	/* flags */
    int			sc_writing;    	/* outstanding writes */
    int			sc_reading;    	/* outstanding reads */
    int			sc_clients;    	/* active opens() */
    struct kue_kapi    *sc_kapi;	/* pointer to kue kernel api */
  
    dev_t		sc_dev;		/* our kue device */
    dev_t		sc_marudev;	/* our maru device */
    m_u64		sc_size;       	/* size in bytes */
    struct semaphore	sc_lock;	/* semaphore for locking unit */    

    struct semaphore	sc_tlock;	/* semaphore for token queue */
    struct maru_token  *sc_tok;		/* pointer token queue */
    m_u32		sc_num_tokens;	/* maximal number of tokens in queue */
    m_u32		sc_active_tokens;	/* number of active tokens */
    m_u32		sc_current_token;	/* token counter */
    m_u32		sc_outstanding_token;	/* number of requests waiting for tokens */
    m_u32		sc_aspect;              /* aspect viewed through this device */
    m_u32       sc_euid;                /* effective user id of whoever opened this device */
};

/* linked list structure for marutukku kernel clients */
struct maru_kclnt {
   struct maru_kclnt *next;
   struct maru_softc *sc;
};

#define LIST_ALLOC(n)		kmalloc((n), GFP_KERNEL)
#define LIST_FREE(e)		kfree(e)

#define LIST_EMPTY(l)		((l) == NULL)
#define LIST_NEXT(e)		((e)->next)
#define LIST_NEW(t)		((typeof(t)*) LIST_ALLOC(sizeof (t)))
#define LIST_INSERT_HEAD(l, e)	do { (e)->next = (l); (l) = (e); } while(0)
#define LIST_FOREACH(l, e)	for ((e)=(l); (e) != NULL; (e) = (e)->next)
#define LIST_DESTROY(l)		do { typeof(l) tmp; \
				     tmp = (l)->next; \
				     LIST_FREE(l); \
				     (l) = tmp; \
				} while(0)

#define maruunit(x)		MINOR(x)


/* {ENTER,EXIT}SC macros should be MP-safe. not tested however */

#define ENTERSC(inode)\
    struct maru_softc *sc;\
    do\
    {\
    int eerr;\
    if (!inode)\
	return -EINVAL;\
    if (maruunit(inode->i_rdev) >= NMARU)\
        return -ENODEV;\
    sc = &maru_softc[maruunit(inode->i_rdev)];\
    eerr = down_interruptible(&sc->sc_lock);\
    if (eerr!=0)\
      return eerr;\
    } while(0)

#define EXITSC(err)\
    do\
    {\
        up(&sc->sc_lock);\
        return err;\
    } while(0)

/* for debugging purposes */
#define DB		printk


#define ERR		printk
#define NOTICE		printk

#define SECT2BYTES(x)	((x)<<9)

#endif /* _MARU_LINUX_H */
