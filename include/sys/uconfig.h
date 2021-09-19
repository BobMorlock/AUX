/*
 * (C) 1985 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* I/O Map:
 *	Each address indicates a physical page (PPAGESIZE in length)
 *	which must be mapped through the MMU.
 *
 *		xxx_MAP is PHYSICAL address
 *		xxx_ADDR is VIRTUAL address
 */
#define	DEVICE_MAP	0x50000000	/* begin phsyical i/o space */
#define PHYSIOMAP	DEVICE_MAP
#define VIA1_MAP	0x50000000	/* VIA1 physical addr (0x2000 long) */
#define VIA2_MAP	0x50002000	/* VIA2 physical addr (0x2000 long) */
#define SCC_MAP		0x50004000	/* SCC physical addr (0x10 long) */
#define SCSI_MAP	0x50010000	/* SCSI physical addr (0x80 long) */
#define	SDMA_MAP	0x50014000	/* SCSI special DMA register */
#define IWM_MAP		0x5001C000	/* IWM physical addr (0x1000 long) */
#define NET_MAP		0x7E0000	/* network physical memory base */
/*#define BOOT_MAP	0xFFF00116	/* for reboot command */

#define	DEVICE_ADDR	0x50000000	/* begin virtual i/o space */
#define VIA1_ADDR	0x50000000	/* VIA1 virtual addr (0x2000 long) */
#define VIA2_ADDR	0x50002000	/* VIA2 virtual addr (0x2000 long) */
#define SCC_ADDR	0x50004000	/* SCC virtual addr (0x10 long) */
#define SCSI_ADDR	0x50010000	/* SCSI virtual addr (0x80 long) */
#define	SDMA_ADDR	sdma_addr	/* SCSI special DMA register */
#define	SDMA_ADDR_R7	0x50014000	/* SCSI special DMA register */
#define	SDMA_ADDR_R8	0x50012000	/* SCSI special DMA register */
#define IWM_ADDR	iwm_addr	/* IWM virtual addr (0x1000 long) */
#define IWM_ADDR_R7	0x5001C000	/* IWM virtual addr (0x1000 long) */
#define IWM_ADDR_R8	0x50016000	/* IWM virtual addr (0x1000 long) */
#define NET_ADDR	0x7E0000	/* network virtual memory base */
#define BOOT_ADDR	0xFFF00116	/* for reboot command */

#if	(defined(KERNEL) && !defined(LOCORE))
extern  long sdma_addr, iwm_addr;
#endif

/* 68020 Instruction Cache */
#define CACHEON		0x1		/* CACR enable cache bit */
#define CACHEOFF	0x0		/* CACR disable cache */
#define CACHEF		0x2		/* CACR freeze cache bit */
#define CACHECE		0x4		/* CACR clear entry bit */
#define CACHECLR	0x8		/* CACR clear cache bit */

/* 68020 Cache and Vector */
#ifdef MMB
#define VECBASE		0xFE000000
#else MMB
#define VECBASE		0x0
#endif MMB
#define KSTART		0x0
#define MMUADDR		0x24000
