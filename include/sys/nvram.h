/*	@(#)nvram.h	UniPlus VVV.2.1.1	*/
/*
 *	Unix NVRAM defines
 *
 *		Byte	Bits		Contents
 *
 *		0x40	3-0		GMT BIAS
 *		0x41	7-0
 *	
 *		0x42	7-5		0-> Reboot Unix
 *					1-> Power off system
 *					2-> Start MacOS
 *
 *			4		Do not run Eschatology
 *
 *			3-0		Reserved
 *
 *		0x43	7-0		Eschatology Cluster Number
 *
 *		0x44	7-0		Reserved
 *		0x45	7-0		Reserved
 */

#define	NVRAM_GMTBIAS		0x40
#define	NVRAM_GMTMASK			0x0fff

#define	NVRAM_AUTO		0x42
#define	NVRAM_AUTO_REQ			0xe0
#define	NVRAM_REBOOT			0x00
#define	NVRAM_POWEROFF			0x20
#define	NVRAM_RUN_MAC			0x40
#define	NVRAM_NO_RUN_ESCH		0x10

#define	NVRAM_ESCHATOLOGY	0x43

