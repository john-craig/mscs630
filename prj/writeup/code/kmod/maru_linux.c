/*
 * $Id: maru_linux.c,v 1.12 2000/08/17 13:02:56 ralphcvs Exp $
 *
 * marutukku kernel module for linux (supporting both 2.0.x and 2.2.x kernels now (well... maybe))
 *
 * Written by Ralf-Philipp Weinmann, 1999/07/12
 *
 * Copyright 1997-1999 Julian Assange & Ralf-P. Weinmann
 *
 * $Log: maru_linux.c,v $
 * Revision 1.12  2000/08/17 13:02:56  ralphcvs
 * bugfixes, bugfixes.
 * added {de,}keyaspect again cause {un,}bindaspect was broken
 * changed ioctl command names:
 * MARUIOC{AT,DE}TACH -> MARUIOC{,UN}BIND
 * MARUIOCISATTACHED -> MARUIOCISBOUND
 *
 * Revision 1.11  2000/08/17 08:14:14  ralphcvs
 * fix: multiple aspects couldn't be attached simultaneously.
 *
 * Revision 1.10  2000/08/04 13:45:57  ralphcvs
 * fixed several bugs in maru_kue_ioctl:
 * invalid ioctl on kue dev and ioctl proxy resulted in an infinite loop
 * maru_kue_ioctl emergency detach was broken
 *
 * Revision 1.9  2000/05/19 03:51:42  ralphcvs
 * maru_hunt_device() added.
 * two-way communication between hose and hose daemon introduced.
 *
 * Revision 1.8  2000/04/25 23:11:11  proff
 * fix exports, warnings
 *
 * Revision 1.7  2000/04/14 09:32:13  ralphcvs
 * changes in maru kernel module to disallow opening of aspect devices
 * unless the flag MUF_PERMISSIVE is set.
 *
 * Revision 1.6  2000/04/12 00:51:20  ralf
 * Aspect handling in process_maru_message() was broken. fixed.
 *
 * Revision 1.5  2000/04/10 07:22:08  ralf
 * The hose daemon seems to work with encryption enabled.
 *
 * Revision 1.4  2000/04/09 01:19:39  ralf
 * introduced MARU_ERROR for signalling errors to the kernel module.
 *
 * Revision 1.3  2000/04/04 17:20:01  ralf
 * hosed daemon rewritten from scratch. not yet functional
 *
 * Revision 1.2  1999/10/22 00:48:29  proff
 * changes from ralf
 *
 * Revision 1.1  1999/09/09 08:04:32  proff
 * changes from ralf
 *
 * Revision 1.1.1.1  1999/09/08 17:32:22  rpw
 * Initial import of Marutukku 0.7
 *
 * Revision 1.1.1.1  1999/08/10 15:17:20  rpw
 * import
 *
 * Revision 1.1  1999/07/25 21:48:16  ralphcvs
 * new marutukku kernel module(s) for linux 2.2 and above.
 * we now have a userspace daemon doing all the cryptographically related
 * work. user/kernelspace message passing is done using the KUE scheme
 * (via a character device). Consult the sources for details.
 *
 *
 */


/* always include linux_compat.h first ! */
#include "linux_compat.h"

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/stat.h>
#include <linux/errno.h>
#include <linux/major.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/blk-mq.h>

#include <asm/uaccess.h>

#include "maru_config.h"
#include "maru_types.h"

#include "maru_linux.h"
#include "kue_linux.h"
#include "maru_bsd_ioctl.h"
#include "kue-api.h"
#include "mkern/mkern-api.h"
#include "queue.h"

#ifndef MARU_MAJOR
#define MARU_MAJOR	LOOP_MAJOR
#endif

#undef MARU_MAJOR
#define MARU_MAJOR	LOOP_MAJOR
#define MARU_MINOR	1

#define DEVICE_NAME		"marutukku"
#define DEVICE_REQUEST	        maru_request
#define DEVICE_NR(device)	(MINOR(device))
#define DEVICE_ON(device)
#define DEVICE_OFF(device)
#define DEVICE_NO_RANDOM
#define TIMEOUT_VALUE		(6 * HZ)
#define MAJOR_NR		MARU_MAJOR

#define NR_SECTORS 				1024
#define MARU_DEFAULT_BLOCKSIZE	1024

static struct maru_softc *maru_softc;

static int	maru_sizes[NMARU];
static int	maru_blksizes[NMARU];

static struct	maru_token *maru_tokens = NULL;
static m_u32	maru_tokens_active = 0, maru_outstanding_tokens = 0;

static struct	semaphore maru_token_lock;



//Debug information about the request
static inline void maru_berror(struct maru_request *req){
    DB(DEVICE_NAME ": maru_berror(%p)\n", req);
    //spin_lock_irq(&io_request_lock);
    //CURRENT = req;
    //spin_unlock_irq(&io_request_lock);
    //blk_mq_end_request(req, BLK_STS_IOERR);
}

/* Returns a 'maru_token' structure from the 'maru_tokens' global variable based on an ID */
static struct maru_token *maru_find_token(m_u32 id){
	//Calculate the technical ID
    struct maru_token *tok;    
    m_u32 tid = id % NUM_TOKENS;

	//Debug output
    DB(DEVICE_NAME ": maru_find_token(%ld): %ld\n", id, tid);

    /* token buffer not yet allocated/initialized */
    if (!maru_tokens)
	{
	    ERR(": token buffer not yet initialized.\n");
	    return NULL;
	}

	//Select from the 'maru_tokens' structure based on the technical ID
    tok = &maru_tokens[tid];

	//Make sure the token is valid
    if (tok->qt_id != id || tok->qt_sc == NULL){
		ERR(DEVICE_NAME ": maru_find_token(%ld) invalid token\n", id);
		return NULL;
    }

    return tok;
}

/* Releases a token which has been passed to it while updating relevant global variables */
static void maru_release_token(struct maru_token *tok){
    DB(DEVICE_NAME ": maru_release_token(%p)\n", tok);
    
    if (!tok)
      return;

	kfree(tok->qt_req->buffer);
    memset(tok, 0, sizeof(struct maru_token));
    maru_tokens_active--;

    if (maru_outstanding_tokens > 0){
		maru_outstanding_tokens--;
		DB(DEVICE_NAME ": maru_outstanding_tokens=%ld\n",maru_outstanding_tokens);
		up(&maru_token_lock);
    }
}

//Finds an unused token in 'maru_tokens' and assigns it to 'sc' and 'req'
static int maru_acquire_token(struct maru_softc *sc, struct request *req){
    static m_u32 current_token;
    struct maru_token *tok;
    int n;
  
    DB(DEVICE_NAME ": maru_acquire_token(%p, %p)\n", sc, req);

	//First allocate the containing global struct, 'maru_tokens' and zero it out
    if (!maru_tokens){
		if ((maru_tokens = kmalloc(sizeof (struct maru_token) * NUM_TOKENS,GFP_KERNEL)) == NULL)
			return -ENOBUFS;

		memset(maru_tokens, 0, sizeof (struct maru_token) * NUM_TOKENS);
    }

	//Loop forever
    for (;;){
		//If 'maru_tokens_active' is greater than NUM_TOKENS
		if (maru_tokens_active >= NUM_TOKENS){
			//Increment the outstanding tokens
			maru_outstanding_tokens++;
	        DB(DEVICE_NAME ": maru_outstanding_tokens=%ld\n",maru_outstanding_tokens);
			//Take down the 'maru_token_lock'
			down(&maru_token_lock);
			DB(DEVICE_NAME ": woken up on token lock.\n");
	    }
	
		//Believe this iterates over all the tokens in 'maru_tokens' and checks
		//for one that has its 'qt_sc' member set to null...
		for (n = 0; n < NUM_TOKENS; n++)
	    	if (maru_tokens[++current_token%NUM_TOKENS].qt_sc == NULL)
				goto found;
		
		//This only happens if loop we're in repeats and the above condition is not
		//successful
		ERR(DEVICE_NAME ": maru_acquire_token() it's locking jim, but not as we know it\n");
		return -ENOBUFS;
    		
		//Jump to here from the condition of finding a token in 'maru_tokens' with 'qt_sc' set
		//to NULL
		found:
		//Assign the token to the request and other parameters
			//But first move all the data from the request
			//into a maru_request struct because Linux fucking sucks
			//and won't just give us a buffer in the request structure
			struct maru_request *mreq;
			struct bio_vec bvec;
			struct bvec_iter i;

			unsigned long total_len = 0;

			/* Do each segment independently. */
			bio_for_each_segment(bvec, req->bio, i) {
				total_len += (unsigned long)bvec.bv_len;
			}

			mreq->buffer = kmalloc(total_len, GFP_KERNEL);
			mreq->buflen = total_len;
			unsigned long last_len = 0;

			bio_for_each_segment(bvec, req->bio, i) {
				char *buffer = kmap_atomic(bvec.bv_page);
				unsigned long offset = bvec.bv_offset;
				size_t len = bvec.bv_len;
				
				memcpy(mreq->buffer+last_len,buffer+offset,len);
				kunmap_atomic(buffer);
			}

			maru_tokens_active++;
			tok = &maru_tokens[current_token%NUM_TOKENS];
			tok->qt_sc = sc;
			tok->qt_req = mreq;
			tok->qt_id = current_token;
			
			DB(DEVICE_NAME ": maru_acquire_token() = %ld/%ld\n", current_token%NUM_TOKENS,current_token);
      
			return current_token;
    }
    /* NOT REACHED */
}

//Release the device defined by the 'sc' parameter; force if necessary
static int maru_detach(struct maru_softc *sc, int force){
    struct maru_token *tok;
    int j;

    /* check whether there is an extent attached to the device */
    if (!(sc->sc_flags & MUF_INITED))
	{
	    ERR(DEVICE_NAME ": nothing to detach.\n");
	    return -EINVAL;
	}

    /* we cannot detach if there are still clients active on the
     * device unless force is activated.
     */
    if (!force && sc->sc_clients > 1)
		return -EBUSY;

    /* release all tokens for outstanding requests */
    if (maru_tokens)
		for(j = 0; j < NUM_TOKENS; j++){
			tok = &maru_tokens[j];
			if (tok->qt_req && tok->qt_sc == sc) {
		    	maru_berror(tok->qt_req);
		    	maru_release_token(tok);
			}
	    }

    if (--sc->sc_kapi->ka_refcount < 1) {
		/* release control of the kue API structure */
		kue_release_kapi(sc->sc_kapi);
    }

    sc->sc_flags &= ~MUF_INITED;
    //invalidate_buffers(sc->sc_marudev);

    return 0;
}

/* we have to sanitize the internal state upon closure of the kue file descriptor */
static int maru_shutdown(struct kue_kapi *ka){
    struct maru_kclnt *kclnt;
    int rval = 0;

    LIST_FOREACH(ka->ka_kclntdata, kclnt) {
	    if (rval < 0)
	    	maru_detach(kclnt->sc, 1);
	    else
		rval = maru_detach(kclnt->sc, 1);
    }
    //LIST_DESTROY((struct maru_kclnt *) ka->ka_kclntdata);
    struct maru_kclnt *ka_kclnt = (struct maru_kclnt *)ka->ka_kclntdata;
    do { 
        struct maru_kclnt *tmp;
		tmp = ka_kclnt->next;
		LIST_FREE(ka_kclnt);
		ka_kclnt = tmp;
	} while(0);

    return rval;
}

/* data going to userland daemon */
static int maru_read2_callback(void *data, size_t len, void *buf){
    struct maru_message *msg = data;
    struct maru_token *tok;
    struct maru_request *req;
    int err;
    
	//Debug
    DB("maru_read2_callback(%p, %ld, %p)\n", data, len, buf);

	//Use the msg (cast from the 'data' parameter) to find
	//a token
    tok = maru_find_token(msg->mm_id);
    
    /* no token found ? */
    if (!tok)
		goto done;

	//Assign the local buffer variable to the token's buffer pointer
    req = tok->qt_req;

	//Copy the contents of 'msg' to the 'buf'
    err = copy_to_user(buf, msg, sizeof *msg);
    
    if (err)
		goto done;
	
	//Condition for a write request
    if (msg->mm_flags&MARU_WRITE){
		//If the message was a write, also copy the buffer of the request
		//into the 'buf' parameter, past the 'msg' which we already copied
        err = copy_to_user(buf + sizeof *msg, req->buffer, msg->mm_len);
	        
        if (err)
			maru_berror(req);
	}

	//Condition for a read request
    if (msg->mm_flags&MARU_READ_REQ){
		//Guess we already handled this
	    DB("MARU_READ_REQ\n");
	    goto done;
	}

    maru_berror(req);
    DB("bad flags = %lx\n", msg->mm_flags);
    /* XXX pad */
 
 	done:
    	kfree(msg);
    
	return 0;
}

/* for data coming back from userland (written to /dev/kue[0-9]) */
static int maru_write_callback(struct kue_kapi *kapi, size_t len, const void *buf){
    int err;
    struct maru_message msg;
    struct maru_token *tok;
    struct maru_request *req;
    
	//Debug
    DB("maru_write_callback(%p, %ld, %p)\n", kapi, len, buf);

    err = copy_from_user(&msg, buf, sizeof msg);

    if (err)
		return (err);
	
	//Find the token corresponding to the message
    tok = maru_find_token(msg.mm_id);

	//Bail out if token wasn't found
    if (!tok)
		return -EINVAL;

	//Get the request out of the token
    req = tok->qt_req;

	//Make sure the length of the request is equal to the length of the message
    if (msg.mm_len != req->buflen){
	    NOTICE(DEVICE_NAME ": invalid length of data (%u != %lu)", msg.mm_len,req->buflen);
	    maru_berror(req);
	    return -EINVAL;
	}

	//If the request was an error handle that
    if (msg.mm_flags & MARU_ERROR){
	  maru_berror(req);
	  return -EINVAL;
    }

	//If the 'msg' was a read, then copy from 'buf' at an offset of msg
	//into the request's buffer -- maybe we got the 'msg' from 'buf'?
    if (msg.mm_flags&MARU_READ){
        err = copy_from_user(req->buffer, buf + sizeof msg, msg.mm_len);
        
        if (err)
            maru_berror(req);
    }

	//Release the token we got earlier
    maru_release_token(tok);

	//Spinlocking stuff I don't get
    // spin_lock_irq(&io_request_lock);
    // saved_req = CURRENT;
    // CURRENT = req;
    // end_request(1);
    // CURRENT = saved_req;
    // spin_lock_irq(&io_request_lock);

    return 0;
}

//Finds a token associated with the 'data' parameter when
//cast to a 'maru_message'; it then releases that token
static int maru_free_callback(void *data, int len){
    struct maru_message *msg = data;
    struct maru_token *tok;
    
    DB("maru_free_callback(%p, %d)\n", data, len);
    
    tok = maru_find_token(msg->mm_id);
    kfree(msg);
    if (!tok)
		return -EINVAL;
    maru_release_token(tok);
    
    return 0;
}

/* entry point of ioctl proxy. useful for emergency detaches etc. */
static int maru_kue_ioctl(struct kue_kapi *ka, struct inode *i, struct file *f, unsigned int cmd, unsigned long arg){
	if (!i){
		NOTICE(DEVICE_NAME ": invalid inode (NULL pointer)\n");
    	return -EINVAL;
	}

	//If we recieved a 'cmd' to unbind, call the shutdown
	switch(cmd) {
  		case MARUIOCUNBIND:
    		return maru_shutdown(ka);
		default:
      		ERR(DEVICE_NAME "ioctl %d not implemented.", cmd);
      		return -1;
	}
}

static int maru_ioctl(struct block_device * disk, fmode_t mode, unsigned int cmd, unsigned long arg){
    struct inode *i = disk->bd_inode;
    ENTERSC(i);
  
    DB(DEVICE_NAME ": maru_ioctl(%p, %p, %x, %lx)\n", i, disk, cmd, arg);

    switch(cmd) {
    	case MARUIOCISBOUND:
			/* check whether this maru device is already bound */
			if (sc->sc_flags & MUF_INITED)
	    		EXITSC(-EBUSY);
		break;
	
    	case MARUIOCPERMISSIVE:
			/* ioctl command for turning permissive mode on/off */
			/* XXX explain impacts of permissive mode */
			if (arg)
	    		sc->sc_flags |= MUF_PERMISSIVE;
			else
	    		sc->sc_flags &= ~MUF_PERMISSIVE;
		break;

    	case MARUIOCBIND:
            struct maru_ioc_attach ma;
            struct kue_kapi *kapi;
            struct maru_kclnt *kclnt;
            int fd;
            struct file *fp;

            if(copy_from_user(&ma, (void *) arg, sizeof(struct maru_ioc_attach)))
                EXITSC(-EFAULT);

            sc->sc_size = ma.ma_size;
            sc->sc_aspect = ma.ma_aspect;

            /* check whether maru device is already bound */
            if (sc->sc_flags & MUF_INITED)
                EXITSC(-EBUSY);

            fd = ma.ma_kue_fd;

            /* check whether file descriptor is valid */
            if (fd >= current->files->fdt->max_fds || (fp=current->files->fdt->fd[fd]) == NULL)
            //if (fd >= NR_OPEN || (fp=current->files->fd[fd]) == NULL)
                EXITSC(-EBADF);

            /* kue devices are character devices. sanity check. */
            if (!S_ISCHR(FINODE(fp)->i_mode)) {
                ERR(DEVICE_NAME ": kue file descriptor doesn't refer to a character device\n");
            }

            /* try to obtain pointer to kue API structure for installing hooks */
            if (!(kapi = kue_get_kapi(maruunit(FINODE(fp)->i_rdev), 1))) {
                ERR(DEVICE_NAME ": cannot install kue hooks\n");
                EXITSC(-EINVAL);
            }

	    	/* install kue API callback and ioctl functions */
	    	sc->sc_kapi = kapi;

	    	if (kapi->ka_refcount++ < 1) {
				kapi->ka_read2_hook = maru_read2_callback;
				kapi->ka_free_hook  = maru_free_callback;
				kapi->ka_write_hook = maru_write_callback;
				kapi->ka_ioctl_hook = maru_kue_ioctl;
				kapi->ka_shutdown   = maru_shutdown;
	    	}
		
	    	kclnt = LIST_NEW(struct maru_kclnt);
	    	kclnt->sc = sc;

	    	if (LIST_EMPTY(kapi->ka_kclntdata)) {
				kapi->ka_kclntdata = kclnt;
				LIST_NEXT((struct maru_kclnt *) kapi->ka_kclntdata) = NULL;
	    	} else {
                //LIST_INSERT_HEAD((struct maru_kclnt *)kapi->ka_kclntdata, kclnt);   
                struct maru_kclnt *ka_kclnt = (struct maru_kclnt *)kapi->ka_kclntdata;
                do { 
                    kclnt->next = ka_kclnt; 
                    ka_kclnt = kclnt; 
                } while(0);
            }

	    	sc->sc_flags |= MUF_INITED;

	    	maru_sizes[maruunit(i->i_rdev)] = sc->sc_size;
	    	/* invalidate buffers for this device. just a precaution */
	    	//invalidate_buffers(i->i_rdev);
	    break;

    	case MARUIOCUNBIND:
			/* XXX check whether passing options by value (instead of reference)
	 		*     is a problem for ioctls
	 		*/
			EXITSC(maru_detach(sc, arg));
		/* NOTREACHED */

    	case MARUIOCSETBLKSIZE:
			if(copy_from_user(&maru_blksizes[maruunit(i->i_rdev)], (void *) arg, sizeof(unsigned int)))
	       	    EXITSC(-EFAULT);
				//invalidate_buffers(i->i_rdev);
		break;

    	case BLKGETSIZE:
			put_user(sc->sc_size >> 9, (long *) arg);
    }

    EXITSC(0);
}

static blk_status_t maru_process_request(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bd){
    struct request *creq = bd->rq;
    struct maru_softc *sc = maru_softc;

    blk_mq_start_request(creq);
	DB(DEVICE_NAME ": maru_process_request() entered.\n");

    if (blk_rq_is_passthrough(creq)) {
        printk (KERN_NOTICE "Skip non-fs request\n");
        blk_mq_end_request(creq, BLK_STS_IOERR);
        goto out;
    }

    /* do work */
	int unit;
    struct maru_message *msg;

    //while (creq){
	    if ((unit = maruunit(creq->part->bd_dev)) >= NMARU)
		{
		    DB(DEVICE_NAME ": minor number out of range\n");
		    goto bail_out;
		}

	    sc = &maru_softc[unit];
	    if (!(sc->sc_flags & MUF_INITED))
		{
		    DB(DEVICE_NAME ": no marutukku extent attached to device\n");
		    goto bail_out;
		}
	    //creq->errors = 0;
	    down(&sc->sc_lock);

	    switch(creq->cmd_flags) {
			case READ:
				msg = kmalloc(sizeof(struct maru_message), GFP_KERNEL);
				msg->mm_flags = MARU_READ_REQ;
				msg->mm_id = maru_acquire_token(sc, creq);
				//msg->mm_len = SECT2BYTES(creq->current_nr_sectors);
				//msg->mm_offset = SECT2BYTES(creq->sector);
				msg->mm_aspect = sc->sc_aspect;
			
				DB(DEVICE_NAME ": MARU_READ_REQ: kue_push(%p, %p) len: %d, offset: %d\n",
			sc->sc_kapi, msg, msg->mm_len, (int)msg->mm_offset);

			if (!(sc->sc_flags & MUF_INITED))
				goto error_out;
		
		if (kue_push(sc->sc_kapi, msg, sizeof(struct maru_message)))
		  {
		    ERR(DEVICE_NAME ": kue_push() failed\n");
		    //maru_berror(msg->req);
		    maru_free_callback((char *) msg, 0);
		    goto error_out;
		  }
		sc->sc_reading++;
		break;
	    case WRITE:
		if (sc->sc_flags & MUF_READONLY)
		    goto error_out;
		msg = kmalloc(sizeof *msg, GFP_KERNEL);
		msg->mm_flags = MARU_WRITE;
		msg->mm_id = maru_acquire_token(sc, creq);
		//msg->mm_len = SECT2BYTES(creq->current_nr_sectors);
		//msg->mm_offset = SECT2BYTES(creq->sector);
		msg->mm_aspect = sc->sc_aspect;
	    
		DB(DEVICE_NAME ": MARU_WRITE: kue_push(%p, %p) len: %d, offset: %d\n",
		   sc->sc_kapi, msg, msg->mm_len, (int)msg->mm_offset);
	    
		if (!(sc->sc_flags & MUF_INITED))
		    goto error_out;
		
		if (kue_push(sc->sc_kapi, msg, sizeof(struct maru_message) + msg->mm_len)) {
		    ERR(DEVICE_NAME ": out of memory while trying to push message onto kue stack\n");
		    //maru_berror(creq);
		    maru_free_callback((char *) msg, 0);
		    goto error_out;
		}
		sc->sc_writing++;
		break;
	    default:
		ERR(DEVICE_NAME ": unknown device command (%d)\n",
		    creq->cmd_flags);
		goto error_out;
	    }
	    up(&sc->sc_lock);
	    //spin_lock_irq(&io_request_lock);
	    //CURRENT = creq = creq->next;
	    //continue;
	
	//}

    DB(DEVICE_NAME ": exiting maru_request().\n");
    blk_mq_end_request(creq, BLK_STS_OK);

out:
    return BLK_STS_OK;
error_out:
	//spin_lock_irq(&io_request_lock);
	blk_mq_end_request(creq, BLK_STS_IOERR);
	return BLK_STS_IOERR;
bail_out:
	blk_mq_end_request(creq, BLK_STS_IOERR);
	return BLK_STS_IOERR;
	//creq->errors++;
	//end_request(0);
	//CURRENT = creq = creq->next;
		
}

static struct blk_mq_ops maru_queue_ops = {
	.queue_rq = maru_process_request,
};

static int maru_open(struct block_device *disk, fmode_t mode){
	struct inode *i = disk->bd_inode;
    ENTERSC(i);
    DB(DEVICE_NAME ": maruopen(%p, %p)\n", i, disk);

    if (!(sc->sc_flags & MUF_PERMISSIVE) && sc->sc_euid != current->tgid)
		EXITSC(-EINVAL);

    if (!sc->sc_clients) {
		/* store the current effective user id for future reference */
		sc->sc_euid   = current->tgid;
		sc->sc_flags &= ~MUF_PERMISSIVE; 
    }
    
    sc->sc_clients++;
  
    EXITSC(0);
	//return 0;
}

static void maru_close(struct gendisk *disk, fmode_t mode){
	struct maru_softc *sc = maru_softc;
	struct inode *i = disk->part0->bd_inode;
    //ENTERSC(i);
    DB(DEVICE_NAME ": maruclose(%p, %p)\n", i, disk);

    if (sc->sc_clients > 0)
	sc->sc_clients--;
  
    //EXITSC(0);
}

static const struct block_device_operations maru_file_ops =
{
	.owner = THIS_MODULE,
    //.read = maru_block_read,			/* read */
    //.write = maru_block_write,			/* write */
    /* FIXME: */
    .ioctl = maru_ioctl,	                /* ioctl */

    .open = maru_open,			/* open */
    .release = maru_close,	       	/* release */

};

static int maru_create_device(struct maru_softc *sc){
    int err, i;

	/* Initialize tag set. */
    sc->tag_set.ops = &maru_queue_ops;
    sc->tag_set.nr_hw_queues = 1;
    sc->tag_set.queue_depth = 128;
    sc->tag_set.numa_node = NUMA_NO_NODE;
    sc->tag_set.cmd_size = 0;
    sc->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
    err = blk_mq_alloc_tag_set(&sc->tag_set);
    if (err) {
        goto out_err;
    }

    /* Allocate queue. */
    sc->maru_queue = blk_mq_init_queue(&sc->tag_set);
    if (IS_ERR(sc->maru_queue)) {
        goto out_blk_init;
    }

    //blk_queue_logical_block_size(sc->maru_queue, KERNEL_SECTOR_SIZE);
    blk_queue_logical_block_size(sc->maru_queue, MARU_DEFAULT_BLOCKSIZE);
	sc->maru_queue->queuedata = sc;

	//Allocate and add the disk for the maru device
	sc->maru_disk = blk_alloc_disk(MARU_MINOR);

	sc->maru_disk->major = MARU_MAJOR;
	sc->maru_disk->first_minor = 0;
	sc->maru_disk->fops = &maru_file_ops;
	sc->maru_disk->queue = sc->maru_queue;
	sc->maru_disk->private_data = sc;
	snprintf( sc->maru_disk->disk_name, 32, "marutukku");
	set_capacity(sc->maru_disk, NR_SECTORS);
	
	add_disk(sc->maru_disk);

	for(i = 0; i < NMARU; i++) {
	/* FIXME */
		// sema_init(&maru_softc[i].sc_lock,0);
		// sema_init(&maru_softc[i].sc_tlock,0);
		maru_softc[i].sc_num_tokens = NUM_TOKENS;
		maru_softc[i].sc_marudev = MKDEV(MARU_MAJOR, i);
		maru_blksizes[i] = MARU_DEFAULT_BLOCKSIZE;
    }
    //block_size[MAJOR_NR] = maru_sizes;
    //blksize_bits[MAJOR_NR] = maru_blksizes;

	return 0;

	out_blk_init:
    	blk_mq_free_tag_set(&sc->tag_set);
	out_err:
    	return -ENOMEM;
}

static void maru_delete_device(struct maru_softc *sc){
	//Remove the disk for the maru device
	if(sc->maru_disk)
		del_gendisk(sc->maru_disk);

	blk_mq_free_tag_set(&sc->tag_set);
    blk_cleanup_queue(sc->maru_queue);
}

int __init maru_init(void) {
	if (register_blkdev(MAJOR_NR, DEVICE_NAME)) {
		ERR(DEVICE_NAME ": cannot register device (major: %d)",MAJOR_NR);
	 	return -EIO;	
	}

//     blk_dev[MAJOR_NR].request_fn = DEVICE_REQUEST;
//     memset(&maru_sizes, 0, sizeof(maru_sizes));
//     memset(&maru_blksizes, 0, sizeof(maru_blksizes));
    if ((maru_softc = kmalloc(sizeof(struct maru_softc) * NMARU, GFP_KERNEL)) == NULL) {
		ERR(DEVICE_NAME ": unable to allocate memory\n");
		cleanup_module();
		return -ENOBUFS;
    }

    memset(maru_softc, 0, sizeof(struct maru_softc) * NMARU);

	maru_create_device(maru_softc);
  
    return 0;
}

void __exit maru_exit(void) 
{
	maru_delete_device(maru_softc);
    NOTICE(DEVICE_NAME ": unloading driver\n");
    if (maru_softc)
		kfree(maru_softc);
    unregister_blkdev(MAJOR_NR, DEVICE_NAME);
}

module_init(maru_init);
module_exit(maru_exit);
MODULE_LICENSE ("GPL v2");