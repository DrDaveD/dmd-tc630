/* */
/*									*/
/*	Copyright (c) 1987,1988,1989,1990,1991,1992   AT&T		*/
/*			All Rights Reserved				*/
/*									*/
/*	  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T.		*/
/*	    The copyright notice above does not evidence any		*/
/*	   actual or intended publication of such source code.		*/
/*									*/
/* */
#define TH_PORT		((char *)0x300003)

#define TH_NOPAPER	0x1
#define TH_ERRBAR	0x2
#define TH_ACKBAR	0x4
#define TH_BUSY		0x8
#define TH_STATUS	0xf
#define TH_NORMAL	(TH_ACKBAR|TH_ERRBAR)

#define BYTESIZE	8
#define THBITS		640
#define THBYTES		((THBITS+BYTESIZE-1)/BYTESIZE)
#define THWORDS		((THBITS+WORDSIZE-1)/WORDSIZE)

typedef struct Tbuf {
	struct Tbuf *next;
	int i, n, size;
	unsigned char *data;
} Tbuf;

Tbuf *gettbuf();

#ifndef	THINKCHAR
extern int (*thquitf)();
#endif
