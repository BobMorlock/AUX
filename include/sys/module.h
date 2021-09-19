/*	@(#)module.h	UniPlus VVV.2.1.2	*/
/*
 *	The following file contains information about how the autoconfig
 *		utility and the firmware communicate about which hardware
 *		is in the system.
 */

/*
 *	This is the data structure found in the MODULES section in a kernel
 *		code file
 */

struct mods {
	char 	name[20];		/* 0 */
	int  	major_num;		/* 20 */
	int	flag;			/* 24 */
	int 	board_id;		/* 28 */
	int	version_lo;		/* 32 */
	int	version_hi;		/* 36 */
	int	options;		/* 40 */
	char	prefix[20];		/* 44 */
	int	devices;		/* 64 */
	int	controllers;		/* 68 */
	long	addresses[16];		/* 72 */
					/* 136 */
	char dummy[256-136];	/* pad out to 256 bytes */
};

/*
 *	defines for flags 
 */

#define	MM_PRESENT	0x01	/* already in the kernel */
#define	MM_INCLUDED	0x02	/* to be included */
#define MM_EXCLUDED	0x04	/* to be excluded */
#define MM_DEVICE	0x08	/* related to a device in the system */
#define MM_UNKNOWN(x)	(((x)&0xf) == 0)

/*
 *	defines for options
 */

#define	MO_BLOCK	0x00000001
#define	MO_CHAR		0x00000002
#define	MO_STREAM	0x00000004
#define	MO_MODULE	0x00000008
#define	MO_TTY		0x00000010
#define	MO_ADDR		0x00000020
#define	MO_SLOT		0x00000040
#define	MO_SOFT		0x00000080
#define	MO_LINE		0x00000100
#define	MO_NETWORK	0x00000200
#define	MO_FORK		0x00000400
#define	MO_EXEC		0x00000800
#define	MO_EXIT		0x00001000

/*
 *	defines for inits
 */

#define	MI_NONE		0
#define	MI_FIRST	1
#define	MI_SECOND	2
#define	MI_NORMAL	3
#define	MI_0		4
#define	MI_LAST		5


/*
 *	The following data structure and constants are created in low core
 *	by the booter at address AUTO_ADDR (normally just above the vectors
 *	and before the kernel). It contains a summary of the devices in the
 *	system (board ids and versions for each slot) and any command from
 *	the booter to autoconfig. It is only valid if contains the magic number.
 */

#define AUTO_ADDR	((struct auto_data *)0x400)
#define AUTO_MAGIC	0x50696773

/*
 *	Run autoconfig
 */

#define	AUTO_OK		0x0		/* the kernel is ok, don't run
					   autoconfig */
#define	AUTO_RUN	0x1		/* run autoconfig to make a new
					   kernel */

struct auto_data {
	long		auto_magic;		/* magic number */
	int		auto_id[16];		/* per slot board ids */
	int		auto_version[16];	/* per slot board version */
	long		auto_command;		/* command to autoconfig */
};
