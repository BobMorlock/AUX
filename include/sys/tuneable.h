/*	@(#)tuneable.h	UniPlus VVV.2.1.1	*/

typedef struct tune {
	int	t_gpgslo;	/* If freemem < t_getpgslow, then start	*/
				/* to steal pages from processes.	*/
	int	t_gpgshi;	/* Once we start to steal pages, don't	*/
				/* stop until freemem > t_getpgshi.	*/
	int	t_gpgsmsk;	/* Mask used by getpages to determine	*/
				/* whether a page is stealable.  The	*/
				/* page is stealable if pte & t_gpgsmsk	*/
				/* is == 0.  Possible values for this	*/
				/* mask are:				*/
				/* 0		- steal any valid page.	*/
				/* PG_REF	- steal page if not	*/
				/*		  referenced in 	*/
				/*		  t_vhandr seconds.	*/
				/*		  mask = 0x00000001	*/
				/* PG_REF|PG_NDREF - steal page if not	*/
				/*		  referenced in 2 *	*/
				/*		  t_vhandr seconds.	*/
				/*		  mask = 0x08000001	*/
	int	t_vhandr;	/* Run vhand once every t_vhandr seconds*/
				/* if freemem < t_vhandl.		*/
	int	t_vhandl;	/* Run vhand once every t_vhandr seconds*/
				/* if freemem < t_vhandl.		*/
	int	t_maxsc;	/* The maximum number of pages which	*/
				/* will be swapped out in a single	*/
				/* operation.  Cannot be larger than	*/
				/* MAXSPGLST in getpages.h.		*/
	int	t_maxfc;	/* The maximum number of pages which	*/
				/* will be saved up and freed at once.	*/
				/* Cannot be larger than MAXFPGLST in	*/
				/* getpages.h.				*/
	int	t_maxumem;	/* The maximum size of a user's virtual	*/
				/* address space in pages.		*/
	int	t_ppmem;	/* Percentage of physical memory not    */
				/* used to avoid deadlock		*/
} tune_t;

extern tune_t	tune;

/*	The following is the default value for t_gpgsmsk.  It cannot be
 *	defined in /etc/master or /etc/system due to limitations of the
 *	config program.
 */

#define	GETPGSMSK	(PG_REF|PG_NDREF)
