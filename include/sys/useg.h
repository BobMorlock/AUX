/*
 * (C) 1985 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#ifndef _USEG_SYSHEAD_
#define _USEG_SYSHEAD_

/* Kernel segment assignments
 */

#ifdef	MMB

#define MAXSEG		128	/* number of segments kernel address space */
#define NBRIOSEG	3	/* number of io segment table entries */
/* Window segments */
#define NETSEG		(MAXSEG-7)	/* network segment */
#define USEG		(MAXSEG-3)	/* segment used for u_area */
#define COPYSEG		(MAXSEG-4)	/* segment used for data copy */
#define LOCSEG 		(MAXSEG-5)	/* segment used for local data area */
#define MEMSEG		(MAXSEG-5)	/* segment used for memory driver */
#define PHYSIOSEG	(MAXSEG-5)	/* segment used for physio */
#define UMOVESEG	(MAXSEG-6)	/* segment used for umove */
#define IOSEG		(MAXSEG-2)	/* first of segments used for io */

#else	MMB

#define MAXSEG		1024	/* number of segments kernel address space */
#define TOPSEG          256      /* number of segments below 1-to-1 map */
 
 /* Window segments */
#define USEG            (TOPSEG-1)      /* segment used for u_area */
#define COPYSEG         (TOPSEG-2)      /* segment used for data copy */
#define LOCSEG          (TOPSEG-3)      /* segment used for local data area */
#define MEMSEG          (TOPSEG-3)      /* segment used for memory driver */
#define PHYSIOSEG       (TOPSEG-3)      /* segment used for physio */
#define UMOVESEG        (TOPSEG-4)      /* segment used for umove */

#endif	MMB

/* User segment assignments
 */

#define	USRSTACK	(stob(NSEGP))

#endif _USEG_SYSHEAD_
