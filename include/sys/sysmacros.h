/*	@(#)sysmacros.h	UniPlus VVV.2.1.1	*/
/*
 * Invalidate ATB entries
 */

#define SYSATB		0x00	/* System atb.			*/
#define	USRRATB		0x01	/* Flag for user read atb.	*/
#define	USRWATB		0x02	/* Flag for user write atb.	*/
#define	URDATB		6	/* User atb for reading.	*/
#define	UWRTATB		7	/* Useratb for writing.		*/
#define	USRATB		(USRRATB|USRWATB)

#define	usermode(ps)	(((ps) & PS_SUP) == 0)		/* user mode psw */
/* BOBJ: priority level 0, not a good enough check for multiprocessors,
 * this will have to change
 */
#define critical(ps)	BASEPRI(ps)


/* Unpack filesystem block into long
 */

#define bnunpk(a, b)	*a++ = 0; *a++ = *b++; *a++ = *b++; *a++ = *b++;

/* Pack long into filesystem block
 */

#define bnpack(a, b)	if (*b++ != 0) printf("iaddress > 2^24\n");\
			*a++ = *b++; *a++ = *b++; *a++ = *b++;
/* Calculate user priority
 */

#define calcppri(p)	((p->p_cpu) >> 1) +  p->p_nice + (PUSER - NZERO)

/*
 *	Macro Definitions
 */


#define dtob(X)   ((X) << BPDSHFT)		/* disk blocks to bytes */
#define btod(X)   (((X) + DOFFMASK) >> BPDSHFT)	/* bytes to disk blocks */
#define ptob(X)   ((X) << PNUMSHFT)		/* pages to bytes */
#define btop(X)   (((X) + POFFMASK) >> PNUMSHFT) /* bytes to pages */
#define btotp(X)  ((unsigned)(X) >> BPPSHFT)	/* bytes to truncated pgs */
#define ptod(X)   ((X) << DPPSHFT)		/* pages to disk blocks */
#define dtop(X)   (((X) + (NDPP-1)) >> DPPSHFT)	/* disk blocks to pages */
#define ptos(X)   (((X) + PNUMMASK) >> PPSSHFT)	/* pages to segments */
#define	ptots(X)  ((X) >> PPSSHFT)		/* pages to truncated segs */
#define	stopg(X)   ((X) << PPSSHFT)		/* segments to pages */
#define stob(X)   ((X) << SNUMSHFT)		/* segments to bytes */
#define btos(X)	  (((X) + SOFFMASK) >> SNUMSHFT) /* bytes to segments */
#define btots(X)  (((unsigned)(X)) >> SNUMSHFT)	/* bytes to truncated segs */

#define	pttopgs(X) (((X) + NPTPP - 1) >> NPTPPSHFT) /* Page tables to pages. */

#define major(x)  (int)((unsigned)(x)>>8)	/* major part of a device */

#define minor(x)  (int)((x)&0377)		/* minor part of a device */
#define makedev(x,y)	(dev_t)(((x)<<8) | (y))	/* make a device number */

/* Some macros for units conversion
 */

/*	Inumber to disk address.  */

#ifdef LATER
#define	itod(x)	(daddr_t)(((unsigned)(x)+(2*INOPB-1))>>INOSHIFT)
#else
#define	itod(x)	(daddr_t)(((unsigned)(x)+(2*INOPB-1))/INOPB)
#endif

/*	Inumber to disk offset.  */

#ifdef LATER
#define	itoo(x)	(int)(((unsigned)(x)+(2*INOPB-1))&(INOPB-1))
#else
#define	itoo(x)	(int)(((unsigned)(x)+(2*INOPB-1))%INOPB)
#endif

#define bzeroba(addr, size)	bzero(addr, size)
#define initlock(lckp, val)	*(lckp) = (val)
