#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <paths.h>
#include <signal.h>
#include <err.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "maru/maru.h"
#include "ipc/ipc_commands.h"
#include "common/encoding.h"
#include "common/client_common.h"
#include "common/common.h"
#include "ciphers/ciphers.h"
#include "remap/remap.h"
#include "utils/getpassphrase.h"
#include "utils/assert.h"
#include "utils/str.h"

#include "hose.h"

static int
new_pass(int as, maruPass *pass)
{
    int l1, l2;
    int tries;
    SECURE
	{
	    maruPass newpass;
	}
    END_SECURE(s);

    for (tries = 0;tries<20;tries++)
	{
	    char msg[80];
	    sprintf(msg, "Enter new aspect %d passphrase (%d significant characters): ", as, sizeof *pass);
	    bzero(pass, sizeof *pass);
	    l1 = asGetPassPhrase(as, msg, pass);
	    if (l1<MIN_PASSPHRASE)
		{
		    printf("Passphrase too short %d<%d.\n", l1, MIN_PASSPHRASE);
		    continue;
		}
	    l2 = asGetPassPhrase(as, "Confirm passphrase: ", &s->newpass);
	    if (l1 != l2 || strncmp(pass->data, s->newpass.data, l1) != 0)
		continue;
	    l2 = asGetPassPhrase(as, "Confirm passphrase (again): ", &s->newpass);
	    if (l1 != l2 || strncmp(pass->data, s->newpass.data, l1) != 0)
		continue;
    else
		break;
	}
    /* In case there is some kind of tty read loop */
    if (tries>=20)
	errx(1, "over twenty attempts at entering passphrase");
    maruWipeFree(s);
    return l1;
}

static int alrmCaught;

static void
sigalrm(int n)
{
    alrmCaught++;
}

static void
mprint_cipher(maruCipherDesc *c, char *pre)
{
    char msg[64] = "";
    if (!c)
	{
	    printf("%sunknown", pre);
	    return;
	}
    if (c->flags & MCD_PARITY)
	sprintf(msg, " (%d bits real)", EITHER(c->keylen, MAX_KEY)*7);
    printf("%sname %s\n" \
	   "%s  cipher_num\t%d\n" \
	   "%s  key_size  \t%d bits%s\n" \
	   "%s  block_size\t%d bits%s\n" \
	   "%s  state/ksch\t%d bytes\n",
	   pre, c->txt,
	   pre, (int)c->cipher,
	   pre, EITHER(c->keylen, MAX_KEY)*8, msg,
	   pre, c->blocksize*8, c->blocksize? "": " (stream cipher)",
	   pre, c->opaque_size);
}

static void
mprint_remap(maruRemapDesc *p, char *pre)
{
    if (!p)
	{
	    printf("%sunknown", pre);
	    return;
	}
    printf("%s%s\t%s\n", pre, p->name, p->txt);
}

static void
mprint_keymap(maruKeymap *h)
{
    maruCipherDesc *c1;
    maruRemapDesc *remapper;
    printf("\tMajor Version:\t%u\n", ntoh8(h->majVersion));
    printf("\tMinor Version:\t%u\n", ntoh8(h->minVersion));
    printf("\tKey Cipher:\n");
    c1 = findCipherType(ntoh8(h->keyCipherType));
    mprint_cipher(c1, "\t\t");
    printf("\tKey Iterations:\t%u\n", ntoh32(h->iterations));
    printf("\tBlocks:    \t%u\n", ntoh32(h->blocks));
    printf("\tBlock Size:\t%u\n", ntoh32(h->blockSize));
    printf("\tLattice Depth:\t%u (%quk addressable bytes)\n",
	   ntoh32(h->depth), (m_u64)1<<((ntoh32(h->depth)-10)));
    printf("\tAspects:  \t%u\n", ntoh32(h->aspects));
    remapper = remapLookupType(ntoh8(h->remapType));
    printf("\tRemap Type:\t%s\n", remapper? remapper->name: "unknown");
    printf("\tChecksum: \t0x%x\n", ntoh32(h->headSum));
}

static void
mdetail(char *dev, char *mir[], char *iv, int life, int idle, int msec)
{
    int mn;
    printf("\tMaru device:\t%s\n" \
	   "\tMaru extents:\t", dev);
    for (mn=0; mir[mn] && mn<MAX_MIRRORS; mn++)
	printf("%s%s", mn? ", ": "", mir[mn]);
    printf("\n" \
	   "\tMaru IV/SALT:\t%s\n" \
	   "\tLife time:\t%d (seconds)\n" \
	   "\tIdle time:\t%d (seconds)\n" \
	   "\tXOR cycle:\t%d (mili seconds)\n",
	   iv, life, idle, msec);
}

//This -- and I'm not kidding -- literally only initializes a bunch of stuff and
//gets the user's password for the aspect. All the sensitive information-- the password,
//metadata, etc, is salted and encrypted. There's like four layers of metadata in this thing.
m_u32
make_aspect(maruInstance *i, maruKeymapAspect *a, int as, maruCipherDesc *c2, maruCipherDesc *c3,
	    m_u32 start, m_u32 blocks, int itime, maruRemapFlags remap_flags)
{
    SECURE
	{
	    int passlen;
	    int maxpasslen;
	    maruPass pass;
	    maruKey remapKey;
	    maruIV remapIV;
	    maruCycle cycle;
	    maruAspectInfo info;
	}
    END_SECURE(s);
    maruOpaque *opaque; 	/* c1 context */
    maruOpaque *info_ctx;
    maruOpaque *remap_ctx;
    m_u64 *mk;
    maruCipherDesc *c1 = i->keyCipher;
    m_u32 iterations = i->iterations;

    assert(as < i->aspects);

    s = maruCalloc(sizeof *s);

	//Fill the maruKeymapAspect struct, 'a' with random data 
    maruRandomf(a, getKeymapAspectLen(i), RAND_PSEUDO);
	//Fill the passworld salt of 'a' with more random data
    maruRandomf(&a->passSalt, sizeof a->passSalt, RAND_PSEUDO);

	//Deteremine what the maximum password length could/should be
    s->maxpasslen = c1->keylen? c1->keylen: MAX_PASSPHRASE;	/* 0 = infinte key length */
    
	//Get a password from the user
	s->passlen = new_pass(as, &s->pass);
    
	//XOR the password recieved from the user with the salt
    xor(&s->pass, &a->passSalt, sizeof s->pass);
    
	//This allocates enough space for and calls the initialization of
	//'c1's cipher
	opaque = maruOpaqueInit(c1);

	//Sets 'c1' key as the password
    c1->setkey(opaque, s->pass.data, s->maxpasslen, MCD_ENCRYPT);
	//Now wipe out the password
    maruWipe(&s->pass, sizeof s->pass);
	//Fill the current cycle with random data
    maruRandomf((char*)&s->cycle, sizeof s->cycle, RAND_PSEUDO);
	//Now fill the current cycle's master key with true random data
    maruRandomf(s->cycle.masterKey.data, MAX_KEY, RAND_TRUE);

	//Optional debug -- skipping
    if (a_debug>1)
	printf("masterKey[0..8] = 0x%qx\n", *(m_u64*)&s->cycle.masterKey);

	//Fill the info key with random data
    maruRandomf(s->cycle.infoKey.data, MAX_KEY, RAND_TRUE);

    if (a_debug>1)
	printf("infoKey[0..8] = 0x%qx\n", *(m_u64*)&s->cycle.infoKey);

	//Create a new context for the information or metadata
    info_ctx = maruOpaqueInit(c1);
	//Set the key for the new context with the info key
    c1->setkey(info_ctx, s->cycle.infoKey.data, sizeof s->cycle.infoKey, MCD_ENCRYPT);

	//Now fill the remapMasyerKey with true random data
    maruRandomf(s->cycle.remapMasterKey.data, MAX_KEY, RAND_TRUE);

    if (a_debug>1)
	printf("remapMasterKey[0..8] = 0x%qx\n", *(m_u64*)&s->cycle.remapMasterKey);

	//Fill the cycleSalt with semi-random data
    maruRandomf((u_char*)&a->cycleSalt, sizeof a->cycleSalt, RAND_PSEUDO);

	//Create a new context this time with 'c3' as the cipher
    remap_ctx = maruOpaqueInit(c3);
	//Set the key for this context to the remap masterkey
    c3->setkey(remap_ctx, s->cycle.remapMasterKey.data, sizeof s->cycle.remapMasterKey.data, MCD_ENCRYPT);
	//Not sure what purpose this serves yet
    s->cycle.keySum[1] = s->cycle.keySum[0];
    
	
	//Apply the salt to this cycle... like the whole thing. Including
	//the keys. Huh.
    xor(&s->cycle, &a->cycleSalt, sizeof s->cycle);
    signal(SIGALRM, sigalrm);
    alrmCaught = 0;

    if (!iterations)
	{
	    signal(SIGALRM, sigalrm);
	    alrmCaught = 0;
	    if (a_debug>0)
		{
		    printf("\nAgitating %s key generator state for %d second%s...\n", c1->txt, itime, (itime == 1)? "": "s");
		    fflush(stdout);
		}
      
		//Cast the entire cycle value into an unsigned 64-bit integer array
	    mk = (m_u64*)&s->cycle;
	    alarm(itime);
		//For each iteration
	    for (iterations=0; !alrmCaught || iterations<2; iterations++)
		{
			//And this would do the XOR chaining necessary for block ciphers I think
		    if (c1->blocksize)	/* only chain block ciphers */
			mk[0] ^= mk[sizeof(s->cycle)/sizeof(m_u64) - 1];

			//Call 'c1' to encrypt... the cycle using the original context, that is,
			//the one using the user's password as the key
		    c1->crypt(opaque, NULL,
			      (u_char*)&s->cycle, (u_char*)&s->cycle, sizeof s->cycle, MCD_ENCRYPT);
		}
 	    if (a_debug>0)
		{
		    printf("%u %s agitations (%u per second)\n", iterations, c1->txt, iterations/itime);
		    fflush(stdout);
		}
	} 
    else
	{
	    if (a_debug>0)
		{
		    printf("\nAgitating %s key generator state for %d iteration%s...\n", c1->txt, iterations, (iterations == 1)? "": "s");
		    fflush(stdout);
		}
		//Same as above, just only done once
	    mk = (m_u64*)&s->cycle;
	    for (; iterations>0; iterations--)
		{
		    if (c1->blocksize)	/* only chain block ciphers */
			mk[0] ^= mk[sizeof(s->cycle)/sizeof(m_u64) - 1];
		    c1->crypt(opaque, NULL, (u_char*)&s->cycle, (u_char*)&s->cycle, 
			      sizeof s->cycle, MCD_ENCRYPT);
		}
	}
    maruOpaqueFree(opaque);
    memcpy(&a->cycle, &s->cycle, sizeof s->cycle);

	//Record the lattice cipher type
    s->info.latticeCipherType = hton8(c2->cipher);
	//Record the block cipher type
    s->info.blockCipherType = hton8(c3->cipher);
	//Record the start position -- which was passed in
    s->info.start = hton32(start);
	//Record the blocks -- which was passed in
    s->info.blocks = hton32(blocks);

	//Create a salt and apply it to the information
    maruRandomf(&a->infoSalt, sizeof &a->infoSalt, RAND_PSEUDO);
    xor(&s->info, &a->infoSalt, sizeof s->info);

	//Encrypt the salt information
    c1->crypt(info_ctx, NULL, (u_char*)&s->info, (u_char*)&a->info, sizeof a->info, MCD_ENCRYPT);
	//Free the context
    maruOpaqueFree(info_ctx);

	//Now fill the lattice key salt, the lattice salt, and the whitener
    maruRandomf((char*)&a->latticeKeySalt, sizeof a->latticeKeySalt, RAND_PSEUDO);
    maruRandomf(&a->latticeSalt, sizeof a->latticeSalt, RAND_PSEUDO);
    maruRandomf(a->whitener, sizeof a->whitener, RAND_PSEUDO);

//#warning fix remap initialisation
	//If applicable, initialize the remap and assign it to 'a's remap
    if (i->remapDesc->create)
	{
	    int len;
	    maruKeymapAspectRemap *hr = i->remapDesc->create(i, blocks, &len, remap_flags);
	    memcpy(&a->remap, hr, len);
	    maruFree(hr);
	}
//#if 0
	// //Fill the entire remap with random data
    // maruRandomf((char*)&a->remap, sizeof a->remap, RAND_PSEUDO);
    // /* use only a single 0 bit to represent an unused split, inorder to
    //    maximally reduce known plaintext */
    // for (int n=0; n<MAX_SPLITS; n++)
	// &a->remap.bmap.remap[n] &= ~REMAP_USED;

	// //Fill the remap key with truly random data and the remap IV with semi-random data
    // maruRandomf(&a->remap.bmap.remapKey, sizeof &a->remap.bmap.remapKey, RAND_TRUE);
    // maruRandomf(&a->remap.bmap.remapIV, sizeof &a->remap.bmap.remapIV, RAND_PSEUDO);

    // /* use a unique key to encrypt the remap, in order to (a) distance
    //    the known remap plaintext from the masterkey, and (b) prevent
    //    bithday attacks on the limited variation (2^64) of remapIV which
    //    could otherwise be used to discover active aspects by noticing fixed
    //    cipher:plaintext variations in the remap array */
    // c1->crypt(opaque, NULL, &a->remap.bmap.remapKey, s->remapKey, sizeof s->remapKey, MCD_ENCRYPT);
    // c1->crypt(opaque, NULL, &a->remap.bmap.remapIV, s->remapIV, sizeof s->remapIV, MCD_ENCRYPT);
    // c3->setkey(opaque, s->remapKey, sizeof s->remapKey, MCD_ENCRYPT);
    // c3->crypt(opaque, s->remapIV, a->remap, a->remap, sizeof a->remap, MCD_ENCRYPT);
 //#endif
    if (a_debug>0)
	{
	    printf("\nClearing key artifacts\n");
	    fflush(stdout);
	}
    maruFree(s);
    return iterations;
}


static void
mchangepass(char *fn, int as, maruRemapFlags remap_flags)
{
#if 0
    SECURE
	{
	    int passlen, oldpasslen;
	    maruPass pass;
	    maruPass oldpass; /* needed because buildAspect wipes the passphrase */
	}
    END_SECURE(s);
    maruKeymap *h;
    maruInstance *i;
    maruAspect *a = maruMalloc(sizeof *a);
    maruKeymapAspect *ahdr = maruMalloc(sizeof *ahdr);
    int klen;
    char prompt[64];
    int tries;

    if ((h = load_keymap(fn, &klen)) == NULL)
	err(1, "couldn't open keymap file");
    i = instanceNew(h, klen, remap_flags);
    sprintf(prompt, "Old passphrase for aspect %d: ", as);
    for(tries = 0; tries < 20; tries++)
	{
	    if (((s->passlen = asGetPassPhrase(as, prompt, &s->pass)) < MIN_PASSPHRASE))
		{
		    fprintf(stderr, "Passphrase too short %d<%d\n", s->passlen, MIN_PASSPHRASE);
		    continue;
		}
	    s->oldpass = s->pass;
	    s->oldpasslen = s->passlen;
	    if (!(a=buildAspect(i, &h->aspect[as] /* bogus */, as, &s->pass, s->passlen)))
		{
		    fprintf(stderr, "Aspect %d non-existant, or incorrect passphrase\n", as);
		    continue;
		}
	    else
	      goto skip;
	}
    fprintf(stderr, "\nBummer! Are we having problems remembering our passphrase ?\n"
	    "Call 1-800-HYPNOSIS for assistance\n\n");
    exit(1);
 skip:
    fprintf(stderr, "Changing passphrase...\n");
    s->passlen = new_pass(as, &s->pass);

    xor(&h->aspect[as].passSalt, &s->oldpass, s->oldpasslen);
    xor(&h->aspect[as].passSalt, &s->pass, s->passlen);
    saveKeymap(fn, h);
    maruFree(a);
    maruFree(ahdr);
    maruFree(s);
    free(h);
#endif
}

static void
print_aspect(maruAspect *a)
{
    printf("\tLattice Cipher:\t%s\n", a->latticeCipher->txt);
    printf("\tBlock Cipher:\t%s\n", a->blockCipher->txt);
    printf("\tStart:       \t%u\n", a->start);
    printf("\tBlocks:      \t%u\n", a->blocks);
}

static void
aspectinfo(char *fn, int as, maruRemapFlags remap_flags)
{
    maruInstance *i;
    maruAspect *a;
    i = genInstance(fn, remap_flags);
    a = getAspect(i, as);
    printf("Aspect %d:\n", as);
    print_aspect(a);
    freeInstance(i);
}

static void
remapinfo(char *fn, maruRemapFlags remap_flags)
{
    maruInstance *i;
    i = genInstance(fn, remap_flags);
    printf("Remap type: ");
    mprint_remap(i->remapDesc, "");
    if (i->remapDesc->info)
	i->remapDesc->info(i, 0, I_INSTANCE);
    freeInstance(i);
}

static void
mnew_aspect(int as, char *fn, maruCipherDesc *c2, maruCipherDesc *c3, m_u32 start, m_u32 blocks, int itime, maruRemapFlags remap_flags)
{
    m_u32 iterations;
    maruInstance *i;
    maruKeymap *h;
    int klen;
    h = loadKeymap(fn, &klen);
    i = instanceNew(h, klen, remap_flags);
    if (as >= i->aspects)
	errx(1, "invalid aspect %d (specified aspect should be in the range 0 - %d)", as, i->aspects);
    if (blocks == 0)
	{
	    blocks = ((m_u64)i->blocks * 2) / 3;
	    blocks += maruRandom32()%((i->blocks - blocks)/2);
	}
    if (blocks > i->blocks)
	errx(1, "Desired number of aspect blocks (%d) is greater than existing number of extent blocks (%d)",
	     blocks, i->blocks);
    if (blocks > i->aspect_blocks)
	errx(1, "Desired number of aspect blocks (%d) is greater than max number of aspect blocks (%d)",
	     blocks, i->aspect_blocks);
    if (blocks + start  > i->blocks)
	errx(1, "Specified aspect (blocks %d-%d) extends past the last extent block (%d)", start, start+blocks, i->blocks);
    if (itime < 1)
	{
	    h->iterations = hton32(16);
	    i->iterations = 16;
	}
    iterations = make_aspect(i, getKeymapAspect(i, h, as), as, c2, c3, start, blocks, itime, remap_flags);
    if (!i->iterations)
	h->iterations = hton32(iterations);
    saveKeymap(fn, h, klen);
}

static void
mkeymap(maruKeymap *h, int len, maruCipherDesc *c1, int depth, int blocksize, m_u32 blocks, m_u32 aspect_blocks, maruRemapDesc *remap, int aspects)
{
    maruRandomf((char*)h, len, RAND_PSEUDO);
    
    h->majVersion = hton8(MH_MAJ_VERSION);
    h->minVersion = hton8(MH_MIN_VERSION);
    h->blockSize = hton32(blocksize);
    h->blocks = hton32(blocks);
    h->aspectBlocks = hton32(aspect_blocks);
    h->depth = hton32(depth);
    h->keyCipherType = hton8(c1->cipher);
    h->iterations = hton32(0);
    h->remapType = hton8(remap->remapType);
    h->aspects = hton32(aspects);
    
    if (a_debug>0)
	{
	    printf("Maru keymap generation complete.\n");
	    fflush(stdout);
	}
}

static void
mnew_keymap(char *name, maruCipherDesc *c1, int depth, int blocksize, m_u32 blocks, m_u32 aspect_blocks, maruRemapDesc *remap, int aspects)
{
    maruKeymap *h;
    struct stat st;
    int len = sizeof(*h) - sizeof (maruKeymapAspect) +
	(sizeof (maruKeymapAspect) +
	 (remap->size? remap->size(aspect_blocks) - sizeof(maruKeymapAspectRemap): 0))
	* aspects; /* ugh */

    if (a_force < 1 && stat(name, &st) == 0)
	errx(1, "%s already exists", name);

    h = maruCalloc(len);
    mkeymap(h, len, c1, depth, blocksize, blocks, aspect_blocks, remap, aspects);
    saveKeymap(name, h, len);
    if (a_debug>0)
	{
	    printf("Saving Maru Keymap as \"%s\"\n", name);
	    printf("* MAKE AT LEAST TWO BACKUPS of this file. If a single bit sells out to the dark\n");
	    printf("  forces of entropy, your entire maru ciphertext extent will follow suit!\n");
	    fflush(stdout);
	}
    maruFree(h);
}

/* we ned to wipe all mirrors in an identical manner to prevent detection of
 * known ciphertext
 */

static void
wipe_extent(char *mirror[], maruCipherDesc *c1, m_u32 blocks, int blocksize, int passno)
{
    void *opaque;
    char *buf;
    time_t ti;
    int fd[MAX_MIRRORS];
    int i;
    int max_fd;
    int keylen = c1->keylen? c1->keylen: MAX_KEY;
    m_u64 n, size=0;

    for (i=0; i<MAX_MIRRORS && mirror[i]; i++)
	{
	    fd[i] = open(mirror[i], O_WRONLY|(a_force? size? O_TRUNC: 0: (passno<2)? O_EXCL: 0)|O_CREAT, 0600);
	    if (fd[i]<0)
		err(1, mirror[i]);
	    if (!blocks)
		{
		    struct stat st;
		    if (fstat(fd[i], &st) != 0)
			err(1, mirror[i]);
		    size = st.st_size;
		}
	}
    max_fd = i;
    if (blocks)
	size = (off_t)blocksize * (off_t)blocks;
    if ((buf = maruMalloc(blocksize)) == NULL)
	err(1, "out of memory.");
    opaque = maruOpaqueInit(c1);
    ti = time(NULL);
    maruRandomf(buf, keylen, RAND_TRUE);
    if (a_debug>0)
	{
	    printf("\n");
	    fflush(stdout);
	}
    c1->setkey(opaque, buf, keylen, MCD_ENCRYPT);
    for (n=0; n<size;)
	{
	    int cc;
	    maruRandom(buf, blocksize, RAND_PSEUDO);
	    c1->crypt(opaque, &buf[sizeof(buf)-MAX_IV], buf, buf, blocksize, MCD_ENCRYPT);
	    cc = MIN(blocksize, size-n);
	    for (i=0; i<max_fd; i++)
		if (write(fd[i], buf, cc) != cc)
		    err(1, mirror[i]);
	    n += cc;
	    if (a_debug>0)
		{
		    time_t t;
		    t = time(NULL);
		    if (n == 0 || n>=size-1 || t-ti>0)
			{
			    printf("Erasing %s (and mirrors) with %s(/dev/random): pass %d %qd/%qd\r", mirror[0], c1->txt, passno, n, size);
			    fflush(stdout);
			    ti = t;
			}
		}
	}
    maruFree(buf);
    maruWipeFree(opaque);
    for (i=0; i<max_fd; i++)
	{
	    Iam1970(mirror[i]);
	    close(fd[i]);
	}
}


static void
create_extent(char *mirror[], maruCipherDesc *c1, int wipe, int blocksize, m_u32 blocks)
{
    m_u64 size;
    size = blocksize * blocks;
    if (wipe>0)
	{
	    int passno;
	    for (passno=1;passno<=wipe;passno++)
		wipe_extent(mirror, c1, blocks, blocksize, passno);
	    if (a_debug>0)
		printf("\n");
	}
    else
	{
	    int fd;
	    int i;
	    if (a_debug>0)
		warnx("Warning: creating extent using Unix file holes. Such extents are *not* crypto-deniable.");
	    for (i=0; i<MAX_MIRRORS && mirror[i]; i++)
		{
		    char *fn = mirror[i];
		    fd = open(fn, O_WRONLY|(a_force? O_TRUNC: O_EXCL)|O_CREAT, 0600);
		    if (fd<0)
			err(1, fn);
		    if (lseek(fd, size-1, SEEK_SET) != size-1)
			err(1, fn);
		    if (write(fd, "", 1) != 1)
			err(1, fn);
		    Iam1970(mirror[i]);
		    close(fd);
		}
	}
    if (a_debug>0)
	printf("Extent creation complete (%qd bytes)\n", size);    
}


static void
crypt_file(char *in, char *out, maruCipherDesc *c3, maruIV *iv, int flags)
{
    FILE *fin;
    FILE *fout;
    maruOpaque *ctx;
    int plen;
    SECURE
	{
	    char buf[MDEF_MARU_BLOCK_SIZE];
	    maruPass pass;
	}
    END_SECURE(s);

    if (!in)
	{
	    in = "<stdin>";
	    if (!(fin = fdopen(0, "r")))
		err(1, in);
	}
    else
	{
	    if (!(fin = fopen(in, "r")))
		err(1, in);
	}
    if (!out)
	{
	    out = "<stdout>";
	    if (!(fout = fdopen(1, "w")))
		err(1, out);
	}
    else
	{
	    if (!(fout = fopen(out, "w")))
		err(1, out);
	}
    
    memset(s->pass.data, 0, sizeof s->pass.data);
    
    if ((plen=getPassPhrase("Passphrase: ", &s->pass))<0)
	errx(1, "invalid passphrase");

    ctx = maruOpaqueInit(c3);
    c3->setkey(ctx, s->pass.data, plen, flags);
    
    for (;;)
	{
	    maruIV t;
	    int cc = fread(s->buf, 1, sizeof s->buf, fin);

	    if (cc == 0)
			break;

	    if (cc<0)
			err(1, in);

	    if (cc%c3->blocksize)
			//If we are encrypting and the blocksize is not
			//even, pad out the end
			if(flags&MCD_ENCRYPT){
				int i;
				for(i=0;i<(cc%c3->blocksize);i++){
					s->buf[cc+i] = 0;
				}
			}

			cc += (c3->blocksize - cc%c3->blocksize);

	    // if (flags&MCD_DECRYPT)
		// 	memcpy(&t, &s->buf[cc-c3->blocksize], c3->blocksize);
	    
		
		c3->crypt(ctx, NULL, s->buf, s->buf, cc, flags);
	    
		if (fwrite(s->buf, 1, cc, fout) != cc)
			err(1, out);

	    // if (flags&MCD_ENCRYPT)
		// 	memcpy(iv, &s->buf[cc-c3->blocksize], c3->blocksize);
	    // else
		// 	memcpy(iv, &t, c3->blocksize);
	}
    maruWipeFree(s);
    /* of course stdio and the kernel in general will keep disk/io buffers around */
    fclose(fout);
    if (!strcmp(out, "<stdout>"))
	Iam1970(out);
    fclose(fin);
}


static void
crypt_aspect(char *name, char *name_keymap, int as, m_u32 maxblocks, int flags, char *io, maruRemapFlags remap_flags)
{
    maruInstance *i;
    maruAspect *a;
    int blockno;
    struct stat st_extent, st_io;
    char *data;
    maruReq req_data, *req = &req_data;
    int io_fd, extent_fd;
    m_u32 blocks;
    m_u32 extent_blocks;
    m_u32 io_blocks;
    bzero(req, sizeof *req);

    if ((extent_fd = open(name, (flags&MCD_ENCRYPT)? O_WRONLY: O_RDONLY)) <0 ||
        fstat(extent_fd, &st_extent)!=0)
	err(1, name);

    if ((flags&MCD_DECRYPT) && io == NULL)
	{
	    io_fd = 1;
	}
    else
	{
	    if ((io_fd = open(io, (flags&MCD_ENCRYPT)? O_RDONLY: O_WRONLY|O_CREAT|((a_force>0)?O_TRUNC:O_EXCL), 0600)) <0 ||
		fstat(io_fd, &st_io)!=0)
		err(1, io);
	}

    i = genInstance(name_keymap, remap_flags);
    assert(i);
    a = getAspect(i, as);
    assert(a);
    if (st_extent.st_size % i->blockSize != 0)
      errx(1, "%s size (%d) is not an integer multiple of maru block size (%d)!", name, (int)st_extent.st_size, i->blockSize);
    if ((flags&MCD_ENCRYPT) && st_io.st_size % i->blockSize != 0)
      errx(1, "%s size (%d) is not an integer multiple of maru block size (%d)!", io, (int)st_io.st_size, i->blockSize);
    data = maruMalloc(i->blockSize);
    i->extent_fd = extent_fd;
    req->aspect = a;
    req->data = data;
    req->blockSize = i->blockSize;
    req->op = (flags&MCD_ENCRYPT)? MR_WRITE: MR_READ;
    
    extent_blocks = MIN(i->blocks, st_extent.st_size / i->blockSize);
    if ((flags & MCD_ENCRYPT))
	io_blocks = st_io.st_size / i->blockSize;
    else
	io_blocks = a->blocks;

    blocks = MIN(maxblocks, MIN(a->blocks, MIN(extent_blocks, io_blocks)));
    
    for (blockno = 0; blockno < blocks; blockno++)
	{
	    req->block = blockno;
	    if (req->op == MR_WRITE)
		{
		    if (read(io_fd, req->data, req->blockSize) != req->blockSize)
			err(1, "read of %s failed", io);
		}
	    if (!i->remapDesc->mapIO(req))
		errx(1, "mapIO failed");
	    if (req->op == MR_READ)
		{
		    if (write(io_fd, req->data, req->blockSize) != req->blockSize)
			err(1, "write of %s failed", io);
		}
	}
    if (flags & MCD_ENCRYPT && a_debug > 0)
	fprintf(stderr, "encrypted %d blocks from %s to %s\n", blocks, io, name);
    else
	fprintf(stderr, "decrypted %d blocks from %s to %s\n", blocks, name, io);
	    
    maruWipeFree(data);	/* XXX actually useless due to VFS buffers */
    freeInstance(i);
    Iam1970(name);
    Iam1970(name_keymap);
    close(extent_fd);
    close(io_fd);
}

static void
minfo(char *dev, char *mir[], char *iv, int life, int idle, int msec)
{
    maruKeymap *h;
    int klen;
    h = loadKeymap(iv, &klen);
    mprint_keymap(h);
    mdetail(dev, mir, iv, life, idle, msec);
    free(h);
}

static void
list_ciphers(bool minimal)
{
    maruCipherDesc *p;
    for (p=m_ciphers; p->txt; p++)
	{
	    if (minimal)
		printf("%s\n", p->txt);
	    else
		mprint_cipher(p, "\t");
	}
}

static void
list_remaps(bool minimal)
{
    maruRemapDesc *p;
    if (!minimal)
	printf("Available remap types:\n");
    for (p=maruRemapTab; p->name; p++)
	{
	    if (minimal)
		printf("%s\n", p->name);
	    else
		mprint_remap(p, "\t");
	}
}

static volatile bool vcaught = FALSE;

/* create a listening AF_UNIX socket */
static int connect_unix_socket(char *sname)
{
    int sock;
    struct sockaddr_un uns;

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
	    warn("bind");
	    return -1;
	}
    
    memset(&uns, 0, sizeof(struct sockaddr_un));
    uns.sun_family = AF_UNIX;
    strncpy(uns.sun_path, sname, sizeof(uns.sun_path));
    if (connect(sock, (struct sockaddr *)&uns, sizeof(struct sockaddr_un)) < 0)
	{
	    warn("connect");
	    close(sock);
	    return -1;
	}
    return sock;
}


#define COMMAND_BUFFER_SIZE		4096

static int
mbindaspect(int sock, int aspect_num)
{
    char buf[COMMAND_BUFFER_SIZE];
    char *data;
    int maxlen;
    int cmd = MARUCMD_BIND_ASPECT;

    data = buf;
    maxlen = sizeof(buf);

    ENCODE(int, cmd);
    ENCODE(int, aspect_num);

    if ((data - buf) > 0)
	return write(sock, buf, data - buf);
    else
	return -1;
}

static int
mdekeyaspect(int sock, int aspect_num)
{
    char buf[COMMAND_BUFFER_SIZE];
    char *data;
    int maxlen;
    int cmd = MARUCMD_DEKEY_ASPECT;

    data = buf;
    maxlen = sizeof(buf);

    ENCODE(int, cmd);
    ENCODE(int, aspect_num);

    if ((data - buf) > 0)
	return write(sock, buf, data - buf);
    else
	return -1;
}

static int
mdetach_extent(int sock, int force)
{
    char buf[COMMAND_BUFFER_SIZE];
    char *data;
    int maxlen;
    int cmd = MARUCMD_DETACH_EXTENT;

    data = buf;
    maxlen = sizeof(buf);

    ENCODE(int, cmd);
    ENCODE(int, force);

    if ((data - buf) > 0)
	return write(sock, buf, data - buf);
    else
	return -1;
}

static int
mkeyaspect(int sock, int aspect_num)
{
    char *data;
    int maxlen;
    int passlen;
    int cmd = MARUCMD_KEY_ASPECT;
    char msg[80];

    SECURE
      {
	  char buf[COMMAND_BUFFER_SIZE];
	  maruPass pass;
      }
    END_SECURE(s);

    sprintf(msg, "Enter aspect %d passphrase (%d significant characters): ", aspect_num, sizeof s->pass);
    memset(&s->pass, 0, sizeof s->pass);
    passlen = asGetPassPhrase(aspect_num, msg, &s->pass);
    if (passlen < MIN_PASSPHRASE)
	{
	    printf("Passphrase too short %d<%d.\n", passlen, MIN_PASSPHRASE);
	    return -1;
	}

    data = s->buf;
    maxlen = sizeof(s->buf);

    ENCODE(int, cmd);
    ENCODE(int, aspect_num);
    ENCODE_RAW(&s->pass, sizeof(s->pass));

    if (data - s->buf > 0)
	return write(sock, s->buf, data - s->buf);
    else
	return -1;
}

static int
mterminate(int sock)
{
    char buf[COMMAND_BUFFER_SIZE];
    char *data;
    int maxlen;
    int cmd = MARUCMD_TERMINATE_DAEMON;

    data = buf;
    maxlen = sizeof(buf);

    ENCODE(int, cmd);

    if ((data - buf) > 0)
	return write(sock, buf, data - buf);
    else
	return -1;
}

static int
marusync(int sock)
{
    char buf[COMMAND_BUFFER_SIZE];
    char *data;
    int maxlen;
    int cmd = MARUCMD_SYNC;

    data = buf;
    maxlen = sizeof(buf);

    ENCODE(int, cmd);

    if ((data - buf) > 0)
	return write(sock, buf, data - buf);
    else
	return -1;
}

static int
munbindaspect(int sock, int aspect_num)
{
    char buf[COMMAND_BUFFER_SIZE];
    char *data;
    int maxlen;
    int cmd = MARUCMD_UNBIND_ASPECT;

    data = buf;
    maxlen = sizeof(buf);

    ENCODE(int, cmd);
    ENCODE(int, aspect_num);

    if ((data - buf) > 0)
	return write(sock, buf, data - buf);
    else
	return -1;
}

static char*
globalise_path(char *file)
{
    char cwd[1024]="";
    static char *s;
    if (s)
	maruFree(s);
    if (file[0] != '/')
	{
	    if (getcwd(cwd, sizeof cwd) == NULL)
		cwd[0] = '\0';
	}
    else
	file++;
    s = maruCalloc(strlen(cwd) + 1 + strlen(file) + 1);
    sprintf(s, "%s/%s", cwd, file);
    return s;
}

static int
mattach_extent(int sock, char *maru_device, char *extent_fname, char *iv_file, int aspect_bsize, maruRemapFlags remap_flags)
{
    char buf[COMMAND_BUFFER_SIZE];
    char *data;
    int maxlen;
    int cmd = MARUCMD_ATTACH_EXTENT;
    
    data = buf;
    maxlen = sizeof(buf);

    ENCODE(int, cmd);
    ENCODE(string, globalise_path(maru_device));
    ENCODE(string, globalise_path(extent_fname));
    ENCODE(string, globalise_path(iv_file));
    ENCODE(int, aspect_bsize);
    ENCODE(int, remap_flags);

    if ((data - buf) > 0)
	return write(sock, buf, data - buf);
    else
	return -1;
}

static int
decode_response(char *data, int len, int *rvalue, char **msg)
{
    DECODE(int, rvalue);
    DECODE(string, msg);
    return 0;
}

static char *
mget_response(int sock)
{
    static char buf[COMMAND_BUFFER_SIZE];
    char *msg;
    int len, result;

    if ((len = read(sock, buf, COMMAND_BUFFER_SIZE)) <= 0) {
	return "read from daemon<->client socket failed.\n";
    }

    decode_response(buf, len, &result, &msg);
    return msg;
}

struct maru_args
{
    char cmdbuf[MDEF_ARGV_MAX*2];
    char *cmdv[MDEF_ARGV_MAX];
};

static int
make_args(struct maru_args *s, char *argv0)
{
    int n;
    int argc;

    fprintf(stderr, "Hi There! What is your bidding? ");
    fflush(stderr);
    s->cmdv[0] = argv0;
    for (n=0; n<sizeof(s->cmdbuf) - 1; n++)
	{
	    if (read(0, &s->cmdbuf[n], 1) != 1)
		err(1, "read failed");
	    if (s->cmdbuf[n] == '\n' ||
		s->cmdbuf[n] == '\r')
		break;
	}
    s->cmdbuf[n] = '\0';
    argc = strToVec(s->cmdbuf, s->cmdv+1, MDEF_ARGV_MAX);
    return argc+1;
}

/* wash away those hated sgml stains, with one easy function.
 * caller is responsible for freeing result. note that this
 * routine isn't too bright, and doesn't handle quotations */
static char *
unsgml(char *sgml)
{
    int len = strlen(sgml);
    char *clean = xmalloc(len);
    int n;
    int m;
    int br;
    for (br=m=n=0; n<len; n++)
	{
	    char c = sgml[n];
	    switch (c)
		{
		case '<': br++; break;
		case '>': if (br>0) br--; break;
		default:
		    if (br < 1)
			clean[m++] = c;
		    break;
		}
	}
    clean[m] = '\0';
    return clean;
}

		    
#define GLOBAL_OPTS "d:EfLP:qQTW"

struct maru_help
{
    char *opt;
    char *arg;
    char *help;
} maru_help[] =
{
    {"1", "cipher", "Cipher for encryption/decryption of keys"},
    {"2", "cipher", "Cipher for generation of block keys"},
    {"3", "cipher", "Cipher for block encryption/decryption"},
    {"A", "aspects", "Max number of usable aspects"},
    {"a", "aspect", "Use aspect number 'aspect'"},
    {"b", "bytes", "Block size in bytes"},
    {"c", "blocks", "Largest aspect size in blocks"},
    {"B", NULL, "Disable pro-active block reallocation (bmap)"},
    {"d", "level", "Set debug level to 'level'"},
    {"D", "depth", "Depth of block key lattice"},
    {"e", NULL, "Use entire maru encryption path for speed calculations"},
    {"E", NULL, "Disable wait for entropy (useful for batch tests)"},
    {"f", NULL, "Force through errors where possible"},
    {"i", "file", "Take input from 'file'"},
    {"I", "seconds", "Autodetach after 'seconds' of idleness"},
    {"L", NULL, "Disable memory locking"},
    {"l", "seconds", "Autodetach after 'seconds' since attach"},
    {"m", NULL, "Minimal output"},
    {"o", "file", "Output operation to 'file'"},
    {"O", "block", "Start block range at offset 'block'"},
    {"P", "level", "Set self-psychoanalysis rigour to 'level'"},
    {"q", NULL, "Quiet"},
    {"Q", NULL, "Quick and quiet, enable <Option>-d0</Option>, <Option>-ELQTW</Option> and <Option>-P0</Option> options"},
    {"R", "path", "Rendezvous with hosed AF_UNIX socket at 'path'"},
    {"r", "remap", "Use remap type 'remap'"},
    {"s", "blocks", "Size in 'blocks'"},
    {"S", NULL, "SGML output"},
    {"t", "time", "Use 'time' seconds of key cycle agitation"},
    {"T", NULL, "Disable reseting file time stamps to epoch"},
    {"V", "iv", "Use 'iv' (in hex) as the initialisation vector"},
    {"w", "rounds", "Apply 'rounds' worth of wiping"},
    {"W", NULL, "Disable memory wiping (useful for batch tests)"},
    {"x", "msec", "Use 'msec' miliseconds between cipher state xors"},
    {}
};

typedef enum
{
    c_aspectinfo,
    c_attachextent,
    c_bindaspect,
    c_changepass,
    c_decryptaspect,
    c_decryptfile,  
    c_dekeyaspect,
    c_detachextent,
    c_encryptaspect,
    c_encryptfile,
    c_example,
    c_global,
    c_help,
    c_info,
    c_keyaspect,
    c_list,
    c_newaspect,
    c_newextent,
    c_newkeymap,
    c_psycho,
    c_remapinfo,
    c_speeds,
    c_terminate,
    c_sync,
    c_unbindaspect,
    c_wipe
} maruCommand;

typedef struct
{
    char *name;
    char *opts;
    char *help_args;
    char *sgml_args;
    char *help_short;
    char *help_exam;
    char *help_long;
    maruCommand cmd;
} maruCommandDesc;


#define c(x,opt,hargs,sargs,hshort,hexam,hlong) {#x, opt, hargs, sargs, hshort, hexam, hlong, c_##x}
#warning long documentation incomplete
maruCommandDesc commands[] = /* style: KISS (no. keep it sorted, stupid) */
{
    c(aspectinfo,
      /* opts */	"a:",
      /* help_args */	"[keymap]",
      /* sgml_args */	0,
      /* help_short */	"Dump informative info about aspect",
      /* help_exam */	"-a 0 maru.keymap",
      /* help_long */	0
     ),
    c(attachextent,
      /* opts */	"a:R:B",
      /* help_args */	"[keymap [extent [device]]]",
      /* sgml_args */	0,
      /* help_short */	"Attach extent",
      /* help_exam */	"-a 0 -R /tmp/rendezvous",
      /* help_long */	0
     ),
    c(bindaspect,
      /* opts */	"a:R",
      /* help_args */	0,
      /* sgml_args */	0,
      /* help_short */	"Binds aspect to device",
      /* help_exam */	"-a 0 -R /tmp/rendezvous",
      /* help_long */	0
     ),
    c(changepass,
      /* opts */	"a:",
      /* help_args */	"[keymap]",
      /* sgml_args */	0,
      /* help_short */	"Change keying for aspect",
      /* help_exam */	"-a 0 maru.keymap",
      /* help_long */	0
     ),
    c(decryptaspect,
      /* opts */	"a:o:s:",
      /* help_args */	"[keymap [extent [output]]]",
      /* sgml_args */	0,
      /* help_short */	"Decrypt from Aspect to output",
      /* help_exam */	"-a 0 -o maru.out",
      /* help_long */	0
     ),
    c(decryptfile,
      /* opts */	"3:i:o:V:",
      /* help_args */	"[input [output]]",
      /* sgml_args */	0,
      /* help_short */	"Conventional file decryption",
      /* help_exam */	"-3 idea-cbc -i maru.ciphertext -V 0xadeadfedbabecafe -o maru.out",
      /* help_long */	0
     ),
    c(dekeyaspect,
      /* opts */	"a:R:",
      /* help_args */	0,
      /* sgml_args */	0,
      /* help_short */	"Dekey aspect",
      /* help_exam */	"-a 0 -R /tmp/rendezvous",
      /* help_long */	0
     ),
    c(detachextent,
      /* opts */	"R:",
      /* help_args */	0,
      /* sgml_args */	0,
      /* help_short */	"Detach previously attached extent",
      /* help_exam */	"-R /tmp/rendezvous",
      /* help_long */	0
     ),
    c(encryptaspect,
      /* opts */	"a:i:s:B",
      /* help_args */	"[keymap [extent [input]]]",
      /* sgml_args */	0,
      /* help_short */	"Encrypt from input to Aspect",
      /* help_exam */	"-a 0 -i maru.plaintext maru.keymap",
      /* help_long */	0
     ),
    c(encryptfile,
      /* opts */	"3:i:o:V:",
      /* help_args */	"[input [output]]",
      /* sgml_args */	0,
      /* help_short */	"Conventional file encryption",
      /* help_exam */	"-3 idea-cbc -i maru.plaintext -o maru.ciphertext",
      /* help_long */	0
     ),
    c(example,
      /* opts */	"m",
      /* help_args */	"command",
      /* sgml_args */	"<Arg>command</Arg>",
      /* help_short */	"Show example usage for command",
      /* help_exam */	"newaspect",
      /* help_long */	0
     ),
    c(global, /* pseudo command to exercise global options */
      /* opts */	GLOBAL_OPTS,
      /* help_args */	0,
      /* sgml_args */	0,
      /* help_short */	0,
      /* help_exam */	0,
      /* help_long */	0
     ),
    c(help,
      /* opts */	"S",
      /* help_args */	"[\"commands\" | \"options\" | command]",
      /* sgml_args */	"<Group><Arg>commands</Arg><Arg>options</arg><Arg><Replaceable>command</Replaceable></Arg></Group>",
      /* help_short */	"General help or help on a particular command",
      /* help_exam */	"newkeymap",
      /* help_long */	0
     ),
    c(info,
      /* opts */	"l:I:x:",
      /* help_args */	"[keymap [extent [device]]]",
      /* sgml_args */	0,
      /* help_short */	"Display configuration",
      /* help_exam */	"maru.keymap",
      /* help_long */	0
     ),
    c(keyaspect,
      /* opts */	"a:R",
      /* help_args */	0,
      /* sgml_args */	0,
      /* help_short */	"Key aspect (needs an attached extent)",
      /* help_exam */	"-a 0 -R /tmp/rendezvous",
      /* help_long */	0
     ),
    c(list,
      /* opts */	"m",
      /* help_args */	"[\"ciphers\" | \"commands\" | \"remaps\"]",
      /* sgml_args */	"<Group><Arg>ciphers</Arg><Arg>commands</Arg><Arg>remaps</Arg></Group>",
      /* help_short */	"List available ciphers, commands or remaps",
      /* help_exam */	"ciphers",
      /* help_long */	0
     ),
    c(newaspect,
      /* opts */	"2:3:a:O:s:t:",
      /* help_args */	"[keymap]",
      /* sgml_args */	0,
      /* help_short */	"Create new aspect for keymap",
      /* help_exam */	"-2 cast-cbc -3 idea-cbc -a 0 -s 64 -t 1 maru.keymap",
      /* help_long */	0
     ),
    c(newextent,
      /* opts */	"1:w:s:b:",
      /* help_args */	"[keymap] [extent]",
      /* sgml_args */	0,
      /* help_short */	"Create new extent",
      /* help_exam */	"-1 cast-cbc -w 0 -s 128 -b 8192",
      /* help_long */	0
     ),
    c(newkeymap,
      /* opts */	"1:A:b:c:D:r:s:",
      /* help_args */	"[keymap]",
      /* sgml_args */	0,
      /* help_short */	"Create new keymap file",
      /* help_exam */	"-1 cast-cbc -A 6 -b 8192 -c 32 -r bmap -s 128 maru.keymap",
      /* help_long */	0
     ),
    c(psycho,
      /* opts */	0,
      /* help_args */	0,
      /* sgml_args */	0,
      /* help_short */	"Visit the psychiatrist",
      /* help_exam */	"",
      /* help_long */	0
     ),
    c(remapinfo,
      /* opts */	0,
      /* help_args */	"[keymap]",
      /* sgml_args */	0,
      /* help_short */	"Dump remap information",
      /* help_exam */	"maru.keymap",
      /* help_long */	0
     ),
    c(speeds,
      /* opts */	"a:eS",
      /* help_args */	"[keymap [extent]]",
      /* sgml_args */	0,
      /* help_short */	"Test cipher speeds",
      /* help_exam */	"",
      /* help_long */	0
     ),
    c(sync,
      /* opts */	"R:",
      /* help_args */	"",
      /* sgml_args */	0,
      /* help_short */	"Sync hose daemon pending writes to disk",
      /* help_exam */	"-R /tmp/rendezvous",
      /* help_long */	0
     ),
    c(terminate,
      /* opts */	"R:",
      /* help_args */	"",
      /* sgml_args */	0,
      /* help_short */	"Terminate hose daemon",
      /* help_exam */	"-R /tmp/rendezvous",
      /* help_long */	0
     ),
    c(unbindaspect,
      /* opts */	"a:R:",
      /* help_args */	0,
      /* sgml_args */	0,
      /* help_short */	"Unbind aspect from device",
      /* help_exam */	"-a 0 -R /tmp/rendezvous",
      /* help_long */	0
     ),
    c(wipe,
      /* opts */	"1:b:",
      /* help_args */	"[extent]",
      /* sgml_args */	0,
      /* help_short */	"Wipe file or extent",
      /* help_exam */	"-1 rc16 maru.extent",
      /* help_long */	0
     ),
    {/*END*/},
};
#undef c


static void
usage_summary(char *opts, FILE *out, bool sgml)
{
    char *p;
    char obuf[128];
    int n;
    for (p=opts, n=0; *p; p++)
	if (*p != ':' && p[1] != ':')
	    obuf[n++] = *p;
    if (n>0)
	{
	    obuf[n] = '\0';
	    fprintf(out, sgml? "<Arg><Option>-%s</Option></Arg>\n": "[-%s]", obuf);
	}
    for (p=opts; *p; p++)
	{
	    struct maru_help *mh;
	    if (p[0] == ':' || p[1] != ':')
		continue;
	    for (mh=maru_help; mh->opt; mh++)
		{
		    if (!strchr(mh->opt, *p))
			continue;
		    if (sgml)
			{
			    fprintf(out, "<Arg><Option>-%c <Replaceable>%s</Replaceable></Option></Arg>\n",
				    *p, mh->arg);
			}
		    else
			{
			    char *s;
			    if (n)
				fprintf(out, " ");
			    else
				n=1;
			    s = unsgml(mh->arg);
			    fprintf(out, "[-%c %s]", *p, s);
			    free(s);
			}
		    break;
		}
	}
}


static void
usage_opt(struct maru_help *mh, FILE *out, bool sgml)
{
    if (sgml)
	{
	    fprintf(out, "\
<VarListEntry>\n\
<Term><Option>-%c", mh->opt[0]);
	    if (mh->arg)
		fprintf(out, " <Replaceable>%s</Replaceable>", mh->arg);
	    fprintf(out, "</Option></Term>\n");
	    fprintf(out, "<ListItem><Para>%s</Para></ListItem>\n", mh->help);
	    fprintf(out, "</VarListEntry>\n");
	}
    else
	{
	    char *help;
	    char *arg;
	    if (mh->arg)
		arg = unsgml(mh->arg);
	    else
		arg = "";
	    if (mh->help)
		help = unsgml(mh->help);
	    else
		help = "";
	    fprintf(out, "\t-%c %-16s %s\n", mh->opt[0], arg, help);
	    if (mh->arg)
		free(arg);
	    if (mh->help)
		free(help);
	}
}


static void
usage_opts(char *opts, FILE *out, bool sgml)
{
    struct maru_help *mh;
    if (sgml)
	fprintf(out, "<VariableList>\n");
    for (mh=maru_help; mh->opt; mh++)
	{
	    char c = mh->opt[0];
	    char *p;
	    if ((p=strchr(opts, c)) && p[1]!=':')
		usage_opt(mh, out, sgml);
	}
    for (mh=maru_help; mh->opt; mh++)
	{
	    char *p;
	    char c = mh->opt[0];
	    if ((p=strchr(opts, c)) && p[1]==':')
		usage_opt(mh, out, sgml);
	}
    if (sgml)
	fprintf(out, "</VariableList>\n");
}


static void
usage_summary_args(maruCommandDesc *cmd, FILE *out, bool sgml)
{
    if (sgml)
	{
	    if (cmd->sgml_args)
		{
		    fprintf(out, "%s", cmd->sgml_args);
		}
	    else
		{
		    if (cmd->help_args)
			{
			    char *p;
			    char c;
			    int br;
			    for (br=0,p=cmd->help_args; (c=*p); p++)
				{
				    switch (c)
					{
					case '[':
					    if (br>0)
						{
						    fprintf(out, "</Replaceable>");
						    br--;
						}
					    fprintf(out, "<Arg><Replaceable>");
					    br++;
					    break;
					case ']':
					    if (br>0)
						{
						    fprintf(out, "</Replaceable>");
						    br--;
						}
					    fprintf(out, "</Arg>");
					    break;
					default:
					    fprintf(out, "%c", c);
					}
				}
			    fprintf(out, "\n");
			}
		}
	}
    else
	{
	    if (cmd->help_args)
		fprintf(out, " %s", cmd->help_args);
	}
}


static void
example(char *cmdname, char *argv0, FILE *out, bool minimal)
{
    maruCommandDesc *cmd;
    for (cmd = commands; cmd->name; cmd++)
	if (strEq(cmd->name, cmdname))
	    break;
    if (!cmd->name)
	errx(1, "unknown command name '%s'", cmdname);
    if (!minimal)
	fprintf(out, "Example:\n\t%s ", argv0);
    fprintf(out, "%s %s\n",  cmd->name, EITHER(cmd->help_exam, ""));
}

static void
usage_cmd(maruCommandDesc *cmd, char *argv0, FILE *out, bool sgml)
{
    if (sgml)
	{
#if 0
	    fprintf(out, "\
<VarListEntry>
<RefEntry id=\"manpage.%s\">\n\
<DocInfo><Date>" __DATE__ "</Date></DocInfo>\n\
<RefMeta>\n\
<RefEntryTitle>%s</RefEntryTitle>\n\
<ManVolNum>1</ManVolNum>\n\
<RefMiscInfo>SP</RefMiscInfo>\n\
</RefMeta>\n\
<RefNameDiv>\n\
<RefName>%s</RefName>\n\
<RefPurpose>%s</RefPurpose>\n\
<RefClass>\n\
A command within hose\n\
</RefClass>\n\
</RefNameDiv>\n",
#endif
#if 0
	    fprintf(out, "\
<VarListEntry>\n\
<Term id=\"hose.%s\">\n\
<CmdSynopsis>\n\
<Command>%s</Command>\n",
#endif
	    fprintf(out, "<RefSect2 id=\"hose.%s\">\n<Title>%s</Title><CmdSynopsis>\n<Command>%s</Command>\n",
		    cmd->name, cmd->name, cmd->name);
	}
    else
	{
	    fprintf(out, "Usage: %s ", argv0);
	    usage_summary(GLOBAL_OPTS, out, sgml);
	    fprintf(out, " %s", cmd->name);
	}
    if (cmd->opts)
	{
	    if (!sgml)
		fprintf(out, " ");
	    usage_summary(cmd->opts, out, sgml);
	}
    usage_summary_args(cmd, out, sgml);
    if (sgml)
	{
#if 0
	    fprintf(out, "</CmdSynopsis>\n</Term>\n<ListItem>\n");
#endif
	    fprintf(out, "</CmdSynopsis>\n");
	}
    else
	{
	    char *p;
	    p = unsgml(cmd->help_short);
	    fprintf(out, "\nDescription:\n\t%s\n", p);
	    free(p);
	    if (cmd->opts)
		{
		    fprintf(out, "Local options:\n");
		    usage_opts(cmd->opts, out, sgml);
		}
	    fprintf(out, "Global options:\n");
	    usage_opts(GLOBAL_OPTS, out, sgml);
	}
    if (sgml)
	{
	    if (cmd->opts)
		usage_opts(cmd->opts, out, sgml);
	    fprintf(out, "<synopsis>%s</synopsis>", cmd->help_short);
	}
    else
	{
	    example(cmd->name, argv0, out, FALSE);
	    if (cmd->help_long)
		{
		    char *p = unsgml(cmd->help_long);
		    fprintf(out, "%s", p);
		    free(p);
		}
	}
    if (sgml)
#if 0
	fprintf(out, "</ListItem>\n</VarListEntry>\n");
#endif
	fprintf(out, "</RefSect2>\n");

}


static void
list_commands(FILE *out, bool sgml, bool minimal)
{
    maruCommandDesc *cmd;
    if (!minimal)
	{
	    if (sgml)
		;
	    else
		fprintf(out, "Hose commands are:\n");
	}
    for (cmd = commands; cmd->name; cmd++)
	if (cmd->cmd != c_global)
	    {
		if (minimal)
		    {
			fprintf(out, "%s\n", cmd->name);
		    }
		else
		    {
			if (sgml)
			    {
				fprintf(out,
					"<VarListEntry>\n<Term><Link linkend=\"hose.%s\"<Command>%s</Command></Link></Term>\n<ListItem><Para>%s</Para></ListItem></VarListEntry>\n",
					cmd->name, cmd->name, cmd->help_short);
			    }
			else
			    {
				char *p = unsgml(cmd->help_short);
				fprintf(out, "\t%-16s%s\n", cmd->name, p);
				free(p);
			    }
		    }
	    }
}


static void
help(char *what, char *argv0, FILE *out, bool sgml)
{
    maruCommandDesc *cmd;
    if (what && !strEq(what, "commands"))
	{
	    if (strEq(what, "all"))
		{
		    for (cmd = commands; cmd->name; cmd++)
			if (cmd->cmd != c_global)
			    usage_cmd(cmd, argv0, out, sgml);
		    return;
		}
	    if (strEq(what, "options"))
		{
		    usage_opts(GLOBAL_OPTS, out, sgml);
		    return;
		}
	    for (cmd = commands; cmd->name; cmd++)
		if (strEq(what, cmd->name))
		    break;
	    if (!cmd->name)
		errx(1, "help doesn't know about command '%s'", what);
	    usage_cmd(cmd, argv0, out, sgml);
	}
    else
	{
	    list_commands(out, sgml, FALSE);
	}
}

static void
usage(char *av0)
{
    errx(1, "Usage: %s [global-options..] command [local-options..] [arguments...]\n\nFor more help try:\n\t%s help\n\t%s list commands\n\t%s help command\n\tman hose\n\n%s with no options or arguments activates invokation-stealth mode.\nNew options and arguments are then requested. As shells and other\nmechanims outside of marutukku's control may keep argument histories,\nthis is the preferred method of marutukku invokation.\n\nFor additional information see the distribution documentation or\nvisit http://www.rubberhose.org/",av0, av0, av0, av0, av0, av0);
}

static int
psycho_options(char ***argvp)
{
    int fails = 0;
    maruCommandDesc *cmd;
    struct maru_help *mh;

    if (a_debug>1)
	warnx("psychoanalysis: checking that all command options have help...");
    for (cmd = commands; cmd->name; cmd++)
	{
	    char *p;
	    if (!cmd->opts)
		continue;
	    for (p=cmd->opts; *p; p++)
		{
		    if (*p == ':')
			continue;
		    for (mh = maru_help; mh->opt; mh++)
			if (strchr(mh->opt, *p))
			    break;
		    if (!mh->opt)
			{
			    warnx("psychoanalysis: missing help for %s -%c", cmd->name, *p);
			    fails++;
			}
		}
	}
    if (a_debug>1 )
	warnx("psychoanalysis: checking that all options have commands that use them...");
    for (mh = maru_help; mh->opt; mh++)
	{
	    for (cmd = commands; cmd->name; cmd++)
		{
		    if (!cmd->opts)
			continue;
		    if (strchr(cmd->opts, mh->opt[0]))
			break;
		}
	    if (!cmd->name)
		{
		    warnx("psychoanalysis: missing command for help of option -%c", mh->opt[0]);
		    fails++;
		}
	}
#if 0 /* can't actually call GNU getopt more than once without SIGSEVGing !@#!@$ crap */
    if (a_debug>1 )
	warnx("psychoanalysis: checking that all examples are parseable...");
    for (cmd = commands; cmd->name; cmd++)
	{
	    if (cmd->help_exam && cmd->opts)
		{
		    int c;
		    int argc;
		    struct maru_args maru_args;
		    char optbuf[strlen(cmd->opts) + 1];
		    memset(&maru_args, 0 , sizeof maru_args);
		    maru_args.cmdv[0] = "hose";
		    strcpy(maru_args.cmdbuf, cmd->help_exam);
		    argc = strToVec(maru_args.cmdbuf, &maru_args.cmdv[1], MDEF_ARGV_MAX);
		    argc++;
		    strcpy(optbuf, "+"); /* @!#$@#$@ GNU getopt */
		    strcat(optbuf, cmd->opts);
		    *argvp = maru_args.cmdv;
		    while ((c=getopt(argc, maru_args.cmdv, optbuf)) != -1)
			if (c == '?')
			    {
				warnx("psychoanalysis: unparseable example '%s %s'", cmd->name, cmd->help_exam);
				fails++;
				break;
			    }
		    *argvp = argv_orig;
		}
	}
#endif
    return fails;
}


int
main(int argc, char **argv)
{
    volatile char marker = 0; 	/* pointer to start of stack */
    int c;
	    char *a_keyCipher = MDEF_KEY_CIPHER;
    char *a_latticeCipher = MDEF_LATTICE_CIPHER;
    char *a_blockCipher = MDEF_BLOCK_CIPHER;
    char *a_mdev = MDEF_MDEV;
    char *a_ext = MDEF_EXT;
    char *a_keymap = MDEF_KEYMAP;
    char *a_socketpath = MDEF_HOSED_SOCKET;
    char *arg1 = NULL;
    char *arg2 = NULL;
    char *arg3 = NULL;
    char *mirrors[MAX_MIRRORS];
    maruCommandDesc *cmd;
    int a_wipe = 1;	/* passes */
    m_u32 a_size = MDEF_MARU_BLOCKS;	/* size in blocks */
    m_u32 a_aspect_blocks = MDEF_MARU_BLOCKS;
    int a_itime = 1;	/* seconds */
    int a_testlevel = 0;
    int a_idle = 60*30;
    int a_life = 60*60*8;
    int a_depth = MAX_LATTICE_DEPTH;
    int a_blockSize = MDEF_MARU_BLOCK_SIZE;
    int a_msec = 500; /* mili seconds */
    int a_aspect = 0;
    int a_aspects = 0;
    char *a_command;
    int hosed_sock = 0;
    char *a_remap = "none";
    char *a_output = NULL;
    char *a_input = NULL;
    bool a_start = 0;
    bool f_clearopt = FALSE;
    bool f_instance = FALSE;
    bool f_help = FALSE;
    bool f_sgml = FALSE;
    bool f_minimal = FALSE;
    bool f_quiet = FALSE;
    maruRemapFlags remap_flags = 0;
    maruCipherDesc *c1, *c2, *c3;
    maruRemapDesc *remapper;
    maruIV iv;
    int fails= 0;
    struct maru_args *maru_args = NULL;

	umask(022);

    memset(mirrors, 0, sizeof mirrors);
    memset(iv.data, 0, sizeof iv.data);

    if (argc < 2)
	{
	    maru_args = maruCalloc(sizeof *maru_args);
	    argc = make_args(maru_args, argv[0]);
	    if (argc < 2)
		usage(argv[0]);
	    argv = maru_args->cmdv;
	}


    /* global options */

    /* the "+" below is to prevent evil GNU getopt argv re-writing */
    while ((c=getopt(argc, argv, "+" GLOBAL_OPTS))!=-1) /* style: keep sorted */
	switch (c)
	    {
	    case 'd':
		a_debug = xmatoi(optarg);
		break;
	    case 'E':
		waitForEntropy = FALSE;
		break;
	    case 'f':
		a_force++;
		break;
	    case 'L':
		f_lockMem = FALSE;
		break;
	    case 'P':
		a_testlevel = xmatoi(optarg);
		break;
	    case 'q':
		f_quiet = TRUE;
		break;
	    case 'Q':
		a_debug = 0;
		waitForEntropy = FALSE;
		f_lockMem = FALSE;
		a_testlevel = 0;
		f_quiet = TRUE;
		f_timestampHack = FALSE;
		break;
	    case 'T':
		f_timestampHack = FALSE;
		break;
	    case 'W':
		f_wipeMem = FALSE;
		break;
	    default:
		usage(argv[0]);
	    }
    
    a_command = argv[optind++];


    if (!a_command)
	usage(argv[0]);

    for (cmd = commands; cmd->name; cmd++)
	if (strEq(cmd->name, a_command))
	    break;

    if (!cmd->name)
	usage(argv[0]);

    if (f_lockMem)
	lockAllMem();

    setuid(getuid());

    if (cmd->opts)
	{
	    while ((c=getopt(argc, argv, cmd->opts))!=-1) /* style: keep sorted */
		switch (c)
		    {
		    case '1':
			a_keyCipher = optarg;
			break;
		    case '2':
			a_latticeCipher = optarg;
			break;	
		    case '3':
			a_blockCipher = optarg;
			break;
		    case 'A': /* nb. when changing this, also change strchr(cmd->opts, 'A')
			       * a few pages below */
			a_aspects = xmatoi(optarg);
			break;
		    case 'a':
			a_aspect = xmatoi(optarg);
			break;
		    case 'b':
			a_blockSize = xmatoi(optarg);
			if (a_blockSize > MAX_MARU_BLOCK)
			    errx(1, "blockSize (%d) > MAX_MARU_BLOCK (%d)", a_blockSize, MAX_MARU_BLOCK);
			break;
		    case 'B':
			remap_flags |= RF_DISABLE_REALLOC;
			break;
		    case 'c':
			a_aspect_blocks = xmatoi(optarg);
			break;
		    case 'D':
			a_depth = xmatoi(optarg);
			break;
		    case 'e':
			f_instance = TRUE;
			break;
		    case 'i':
			a_input = optarg;
			break;
		    case 'I':
			a_idle = xmatoi(optarg);
			break;
		    case 'l':
			a_life = xmatoi(optarg);
			break;
		    case 'm':
			f_minimal = TRUE;
			break;
		    case 'n':
			f_clearopt = TRUE;
			break;
		    case 'o':
			a_output = optarg;
			break;
		    case 'O':
			a_start = xmatoi(optarg);
			break;
		    case 'r':
			a_remap = optarg;
			break;
		    case 'R': /* nb. when changing this, also change strchr(cmd->opts, 'R')
				 a few pages below */
			a_socketpath = optarg;
			break;
		    case 's':
			a_size = xmatoi(optarg);
			break;
		    case 'S':
			f_sgml = TRUE;
			break;
		    case 't':
			a_itime = xmatoi(optarg);
			break;
		    case 'V':
			if (hexToBin(optarg, iv.data, sizeof iv.data)<1)
			    errx(1, "invalid iv (should be of the form 0x001122334455667788): %s", optarg);
			break;
		    case 'w':
			a_wipe = xmatoi(optarg);
			break;
		    case 'x':
			a_msec = xmatoi(optarg);
			break;
		    default:
			usage_cmd(cmd, argv[0], stderr, f_sgml);
			exit(1);
		    }
	}


    if (f_help)
	{
	    usage_cmd(cmd, argv[0], stdout, f_sgml);
	    exit(0);
	}

    if (a_aspects < 1 && cmd->opts && strchr(cmd->opts, 'A'))
	a_aspects = MDEF_MARU_ASPECTS/2 + maruRandom32()%(MDEF_MARU_ASPECTS*2) + 1;
	
    if (a_testlevel>0)
	{
	    fails = psycho_options(&argv);
	    if (fails>0)
		errx(1, "Failed the maru DSM");

	    //fails += psychoanalyse(a_testlevel);
	    if (fails)
		errx(1, "flunked %d components of the maru DSM\n", fails); 
	}
    if (cmd->opts && strchr(cmd->opts, 'R'))
	{
	    hosed_sock = connect_unix_socket(a_socketpath);
	    if (hosed_sock < 0)
		err(1, "cannot connect to hosed's AF_UNIX socket");
	}


     remapper = remapLookupStr(a_remap);
     if (!remapper)
	 errx(1, "invalid remap type '%s'", a_remap);
     c1 = xfindCipherTxt(a_keyCipher);
     c2 = xfindCipherTxt(a_latticeCipher);
     c3 = xfindCipherTxt(a_blockCipher);
     if (a_blockSize<c3->blocksize)
	 errx(1, "your specified block size (%d) is smaller than the %s cryptographic block size (%d) (size matters in the new cryptographic Jerusalem)", a_blockSize, c3->txt, c3->blocksize);
     switch (argc-optind)
	 {
	 case 3:
	     arg3 = argv[optind+2];
	 case 2:
	     arg2 = argv[optind+1];
	 case 1:
	     arg1 = argv[optind+0];
	 case 0:
	     break;
	 default:
	     usage_cmd(cmd, argv[0], stderr, f_sgml);
	     exit(1);
	 }


     if (a_debug<2)
	 {
	     nocore();
	     nosignals();
	     atexit(maruExitHandler);
	 }

    //  if (!f_quiet)
	//  {
	//      fprintf(stderr, "hose (%s) (c) 1997-2000 Julian Assange <proff@iq.org>\n" \
	// 	             "               1999-2000 Ralf-P. Weinmann <ralph@iq.org>\n", MARU_VERSION);
	//      fprintf(stderr, "%s\n", mquote());
	//      fflush(stderr);
	//  } 

     stackP = &marker;


     switch(cmd->cmd)
	 {
	 case c_aspectinfo:
	     aspectinfo(EITHER(arg1, a_keymap), a_aspect, remap_flags);
	     break;

	 case c_attachextent:
	     mattach_extent(hosed_sock, EITHER(arg3, a_mdev), EITHER(arg2, a_ext), EITHER(arg1, a_keymap), a_blockSize, remap_flags);
	     fprintf(stderr, "%s\n", mget_response(hosed_sock));
	     break;

	 case c_bindaspect:
	     mbindaspect(hosed_sock, a_aspect);
	     fprintf(stderr, "%s\n", mget_response(hosed_sock));
	     break;

	 case c_changepass:
	     mchangepass(EITHER(arg1, a_keymap), a_aspect, remap_flags);
	     break;

	 case c_dekeyaspect:
	     mdekeyaspect(hosed_sock, a_aspect);
	     fprintf(stderr, "%s\n", mget_response(hosed_sock));
	     break;

	 case c_detachextent:
	     mdetach_extent(hosed_sock, a_force);
	     fprintf(stderr, "%s\n", mget_response(hosed_sock));
	     break;

	 case c_encryptaspect:
	     crypt_aspect(EITHER(arg2, a_ext), EITHER(arg1, a_keymap), a_aspect, a_size, MCD_ENCRYPT, EITHER(arg3, a_input), remap_flags);
	     break;

	 case c_encryptfile:
	     crypt_file(EITHER(arg1, a_input), EITHER(arg2, a_output), c3, &iv, MCD_ENCRYPT);
	     break;

	 case c_example:
	     if (!arg1)
		 {
		     usage_cmd(cmd, argv[0], stderr, f_sgml);
		     exit(1);
		 }
	     example(arg1, argv[0], stdout, f_minimal);
	     break;

	 case c_decryptaspect:
	     crypt_aspect(EITHER(arg2, a_ext), EITHER(arg1, a_keymap), a_aspect, a_size, MCD_DECRYPT, EITHER(arg3, a_output), remap_flags);
	     break;

	 case c_decryptfile:
	     crypt_file(EITHER(arg1, a_input), EITHER(arg2, a_output), c3, &iv, MCD_DECRYPT);
	     break;

	 case c_global:
	     NOTREACHED;

	 case c_help:
	     help(arg1, argv[0], stdout, f_sgml);
	     break;

	 case c_keyaspect:
	     mkeyaspect(hosed_sock, a_aspect);
	     fprintf(stderr, "%s\n", mget_response(hosed_sock));
	     break;

	 case c_info:
	     mirrors[0] = EITHER(arg2, a_ext);
	     minfo(EITHER(arg3, a_mdev), mirrors, EITHER(arg1, a_keymap), a_life, a_idle, a_msec);
	     break;

	 case c_list:
	     if (arg1)
		 {
		     if (strEq(arg1, "ciphers"))
			 {
			     list_ciphers(f_minimal);
			     break;
			 }
		     if (strEq(arg1, "commands"))
			 {
			     list_commands(stdout, f_sgml, f_minimal);
			     break;
			 }
		     if (strEq(arg1, "remaps"))
			 {
			     list_remaps(f_minimal);
			     break;
			 }
		 }
	     usage_cmd(cmd, argv[0], stderr, f_sgml);
	     exit(1);
	     break;

	 case c_newextent:
	     mirrors[0] = EITHER(arg2, a_ext);
	     create_extent(mirrors, c1, a_wipe, a_blockSize, a_size);
	     break;

	 case c_newkeymap:
	     mnew_keymap(EITHER(arg1, a_keymap), c1, a_depth, a_blockSize, a_size, MIN(a_size, a_aspect_blocks), remapper, a_aspects);
	     break;

	 case c_newaspect:
	     mnew_aspect(a_aspect, EITHER(arg1, a_keymap), c2, c3, a_start, a_size, a_itime, remap_flags);
	     break;

	 case c_psycho:
	     printf("Passed the maru DSM, level %d\n", a_testlevel);
	     break;

	 case c_remapinfo:
	     remapinfo(EITHER(arg1, a_keymap), remap_flags);
	     break;

	//  case c_speeds:
	//      speeds(EITHER(arg2, a_ext), EITHER(arg1, a_keymap), 1, a_aspect, f_instance, f_sgml, remap_flags);
	//      break;

#if 0
	 case c_stats:
	     stackP = NULL;
	     mstats(EITHER(arg1, a_mdev));
	     break;
#endif

	 case c_terminate:
	     mterminate(hosed_sock);
	     break;

	 case c_sync:
	     marusync(hosed_sock);
	     break;

	 case c_unbindaspect:
	     munbindaspect(hosed_sock, a_aspect);
	     fprintf(stderr, "%s\n", mget_response(hosed_sock));
	     break;

	 case c_wipe:
	     mirrors[0] = EITHER(arg1, a_ext);
	     wipe_extent(mirrors, c1, 0, a_blockSize, a_wipe? a_wipe: 1);
	     break;
	}
     if (maru_args)
	 maruWipeFree(maru_args);
    stackP = NULL;
    exit(0);
}