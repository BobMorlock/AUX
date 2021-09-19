/* NFSSRC @(#)types.h	2.2 86/05/15 */
/*	@(#)types.h	UniPlus VVV.2.1.1	*/

#ifndef _TYPES_
#define _TYPES_
/*
 * Machine specific system types
 */
typedef	struct{int r[1]} *physadr;

typedef unsigned short	iord_t;	/* enough to count the incore inodes */

typedef	int		label_t[13];
typedef	unsigned short	pgadr_t;

typedef char		swck_t;

typedef unsigned char	use_t;
#define MAXSUSE		255	/* Maximum share count on swap	*/

/*
 * Machine independent system parameters
 */
typedef	long		daddr_t;
typedef	char *		caddr_t;
typedef	unsigned char	uchar_t;
typedef unsigned char	u_char;
typedef	unsigned short	u_short;
typedef unsigned int	u_int;
typedef unsigned long	u_long;
typedef unsigned char	unchar;
typedef	unsigned int	uint;
typedef	unsigned short	ushort;
typedef	unsigned long	ulong;
typedef	ushort		ino_t;
typedef short		cnt_t;
typedef	long		time_t;
typedef	ushort		dev_t;
typedef	long		off_t;
typedef	long		paddr_t;
typedef	long		key_t;
typedef	long		ubadr_t;	/* physical unibus address */
typedef struct  	_quad { long val[2]; } quad;
typedef int     	size_t;
typedef struct  fd_set { long fds_bits[1]; } fd_set;
#endif	_TYPES_
