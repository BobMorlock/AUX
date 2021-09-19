/*
 *
 *	Copyright 1986 Apple Computers.
 *
 *
 *
 */

#include	<sys/via6522.h>

typedef	unsigned char	u8;
typedef	unsigned short	u16;
typedef	unsigned int	u32;

#define	PHYS		0xf0000000

#define	AE6_BASE(unit)	((unsigned)PHYS+(unit << 24))

#define	RAM		0x000d0000
#define	CTRL		0x000e0000
#define	ROM		0x000f0000
#define	RAMLEN		0x4000
#define	ROMLEN		0x4000

#define	RECV_START	0x08
#define	RECV_STOP	0x3f
#define	HLEN		0x20
#define	LLEN		0x00
#define	XMIT_START	0x00

#define	AE6_PAGESIZE	0x100
#define	PKTLEN		((8 * PAGESIZE) / 2)

struct ae6_regs {
	union {
		char	ae6_w_imr[4]	;
		char	ae6_r_cntr2[4]	;
		char	ae6_rw_mar7[4]	;
	} rf;
	union {		
		char	ae6_w_dcr[4]	;
		char	ae6_r_cntr1[4]	;
		char	ae6_rw_mar6[4]	;
	} re;
	union {
		char	ae6_w_tcr[4]	;
		char	ae6_r_cntr0[4]	;
		char	ae6_rw_mar5[4]	;
	} rd;
	union {
		char	ae6_w_rcr[4]	;
		char	ae6_r_rsr[4]	;
		char	ae6_rw_mar4[4]  ;
	} rc;
	union {
		char	ae6_w_rbcr1[4]	;
		char	ae6_rw_mar3[4]	;
	} rb;
	union {
		char	ae6_w_rbcr0[4]  ;
		char	ae6_rw_mar2[4]	;
	} ra;
	union {
		char	ae6_w_rsar1[4]	;
		char	ae6_r_crda1[4]	;
		char	ae6_rw_mar1[4]  ;
	} r9;
	union {
		char	ae6_w_rsar0[4]	;
		char	ae6_r_crda0[4]	;
		char	ae6_rw_mar0[4]	;
	} r8;
	union {
		char	ae6_w_isr[4]	;
		char	ae6_rw_curr[4]	;
	} r7;
	union {
		char	ae6_w_tbcr1[4]	;
		char	ae6_r_fifo[4]	;
		char	ae6_rw_par5[4]	;
	} r6;
	union {
		char	ae6_w_tbcr0[4]	;
		char	ae6_r_ncr[4]	;
		char	ae6_rw_par4[4]	;
	} r5;
	union {
		char	ae6_w_tpsr[4]	;
		char	ae6_r_tsr[4]	;
		char	ae6_rw_par3[4]	;
	} r4;
	union {
		char	ae6_w_bnry[4]	;
		char	ae6_rw_par2[4]	;
	} r3;
	union {
		char	ae6_w_pstop[4]	;
		char	ae6_r_clda1[4]  ;
		char	ae6_rw_par1[4]	;
	} r2;
	union {
		char	ae6_w_pstart[4]	;
		char	ae6_r_clda0[4]	;
		char	ae6_rw_par0[4]	;
	} r1;
		char	ae6_rw_cr[4];
};

#define	ae6_imr		rf.ae6_w_imr[0]
#define	ae6_dcr		re.ae6_w_dcr[0]
#define	ae6_tcr		rd.ae6_w_tcr[0]
#define	ae6_rcr		rc.ae6_w_rcr[0]
#define	ae6_rbcr1	rb.ae6_w_rbcr1[0]
#define	ae6_rbcr0	ra.ae6_w_rbcr0[0]
#define	ae6_rsar1	r9.ae6_w_rsar1[0]
#define	ae6_rsar0	r8.ae6_w_rsar0[0]
#define	ae6_isr		r7.ae6_w_isr[0]
#define	ae6_tbcr1	r6.ae6_w_tbcr1[0]
#define	ae6_tbcr0	r5.ae6_w_tbcr0[0]
#define	ae6_tpsr	r4.ae6_w_tpsr[0]
#define	ae6_bnry	r3.ae6_w_bnry[0]
#define	ae6_pstop	r2.ae6_w_pstop[0]
#define	ae6_pstart	r1.ae6_w_pstart[0]

#define	ae6_csr		ae6_rw_cr[0]

#define	ae6_cntr2	rf.ae6_r_cntr2[0]
#define	ae6_cntr1	re.ae6_r_cntr1[0]
#define	ae6_cntr0	rd.ae6_r_cntr0[0]
#define	ae6_rsr		rc.ae6_r_rsr[0]
#define	ae6_crda1	r9.ae6_r_crda1[0]
#define	ae6_crda0	r8.ae6_r_crda0[0]
#define	ae6_fifo	r6.ae6_r_fifo[0]
#define	ae6_ncr		r5.ae6_r_ncr[0]
#define	ae6_tsr		r4.ae6_r_tsr[0]
#define	ae6_clda1	r2.ae6_r_clda1[0]
#define	ae6_clda0	r1.ae6_r_clda0[0]

#define	ae6_mar7	rf.ae6_rw_mar7[0]
#define	ae6_mar6	re.ae6_rw_mar6[0]
#define	ae6_mar5	rd.ae6_rw_mar5[0]
#define	ae6_mar4	rc.ae6_rw_mar4[0]
#define	ae6_mar3	rb.ae6_rw_mar3[0]
#define	ae6_mar2	ra.ae6_rw_mar2[0]
#define	ae6_mar1	r9.ae6_rw_mar1[0]
#define	ae6_mar0	r8.ae6_rw_mar0[0]
#define	ae6_curr	r7.ae6_rw_curr[0]
#define	ae6_par5	r6.ae6_rw_par5[0]
#define	ae6_par4	r5.ae6_rw_par4[0]
#define	ae6_par3	r4.ae6_rw_par3[0]
#define	ae6_par2	r3.ae6_rw_par2[0]
#define	ae6_par1	r2.ae6_rw_par1[0]
#define	ae6_par0	r1.ae6_rw_par0[0]


/* 
 * Bit definitions for the command register.  
 */
#define	CR_STP		0x21
#define	CR_STA		0x22
#define	CR_TXP		0x26
#define	CR_DMA_NOP		0x00
#define	CR_DMA_RRD	0x08
#define	CR_DMA_RWR	0x10
#define	CR_DMA_SPK	0x18
#define	CR_DMA_ACO	0x20
#define	CR_PS0		0x00
#define	CR_PS1		0x40
#define	CR_PS2		0x80


/* 
 * Bit definitions for the interrupt status register (ISR).  The isr register
 * is available whether you are reading or writing the page zero registers. 
 */
#define	ISR_PRX		0x01
#define	ISR_PTX		0x02
#define	ISR_RXE		0x04
#define	ISR_TXE		0x08
#define	ISR_OVW		0x10
#define	ISR_CNT		0x20
#define	ISR_RDC		0x40
#define	ISR_RST		0x80
#define	ISR_ALL		0xFF

/* 
 * Bit definitions for the interrupt mask register (IMR).  This is a write only
 * register 
 */
#define	IMR_NONE	0x00
#define	IMR_PRXE	0x01
#define	IMR_PTXE	0x02
#define	IMR_RXEE	0x04
#define	IMR_TXEE	0x08
#define	IMR_OVWE	0x10
#define	IMR_CNTE	0x20
#define	IMR_RDCE	0x40

/* 
 * The data configuration register bit definitions.  Again, these are writable
 * bits.
 */
#define	DCR_WTS		0x01
#define	DCR_BOS		0x02
#define	DCR_LAS		0x04
#define	DCR_BMS		0x08
#define	DCR_AR		0x10
#define	DCR_FT_1	0x00
#define	DCR_FT_2	0x20
#define	DCR_FT_4	0x40
#define	DCR_FT_6	0x60

/* 
 * The transmit configuration register (TCR)  bit definitions.  This is a 
 * write only register.
 */

#define	TCR_CRC		0x00
#define TCR_NOCRC	0x01
#define	TCR_LOO_NON	0x00
#define	TCR_LOO_INT	0x02
#define	TCR_LOO_EXT	0x04
#define	TCR_LOO_XTR	0x06
#define	TCR_ATD		0x08
#define	TCR_OFST	0x10

/* 
 * The transmit status register (TSR) bit definitions.  This is a read 
 * only register.
 */

#define	TSR_PTX		0x01
#define	TSR_DFR		0x02
#define	TSR_COL		0x04
#define	TSR_ABT		0x08
#define	TSR_CRS		0x10
#define	TSR_FU		0x20
#define	TSR_CDH		0x40
#define	TSR_OWC		0x80


/* 
 * The receive configuration register (RCR) bit definitions.  This is a
 * read only register.
 */

#define	RCR_SEP		0x01
#define	RCR_AR		0x02
#define	RCR_AB		0x04
#define	RCR_AM		0x08
#define	RCR_PRO		0x10
#define	RCR_MON		0x20

/*
 * The receive status register (RSR) bit definitions.  This is a read
 * only register.
 */

#define	RSR_PRX		0x01
#define	RSR_CRC		0x02
#define	RSR_FAE		0x04
#define	RSR_FO		0x08
#define	RSR_MPA		0x10
#define	RSR_PHY		0x20
#define	RSR_DIS		0x40
#define	RSR_DFR		0x80

/* 
 * The mask to use to get all eleven bits of byte count from the transmit
 * byte count registers (TBCR0 and TBCR1.)  The comuptation to use is
 * (ae6_tbcr0 | (ae6_tbcr1 << 8)) & TBCR_MASK.  
 */

#define	TBCR_MASK	0x7ff

struct rcv_pkt_hdr {
	u8	rph_status	;
	u8	rph_next	;
	u8	rph_lbc		;
	u8	rph_hbc		;
	struct ether_header	rph_eh;
#define	rph_dest	rph_eh.ether_dhost
#define	rph_src		rph_eh.ether_shost
#define	rph_type	rph_eh.ether_type
};

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * ae6_if, which the routing code uses to locate the interface.  This
 * structure contains the control register and ram addresses, the
 * interface address and error counts for the interface.
 */

struct ae6 {
	struct arpcom	ae6_ac;		/* common ethernet structures */
#define	ae6_if		ae6_ac.ac_if	/* network-visible interface */
#define	ae6_enaddr	ae6_ac.ac_enaddr
					/* hardware ethernet address */
	struct ae6_regs	*ae6_cmd;	/* register page pointer */
	char		*ae6_base;	/* ram base pointer */
	char		*ae6_rom;	/* rom base pointer */
	short		ae6_oactive;	/* output active flag */
	int		ae6_flags;	/* flag bits */
	struct {			/* error type counts */
		int	rcrc_err;	/* receive crc errors */
		int	rfae_err;	/* receive framing errors */
		int	rfo_err;	/* fifo overrun errors */
		int	rmiss_err;	/* missed packet errors */
		int	scol_err;	/* send collisions */
		int	sabt_err;	/* send aborts */
		int	scrs_err;	/* send carrier send lost errors */
		int	sfu_err;	/* send fifo underruns */
		int	scdh_err;	/* CD heartbeat failures */
		int	sowc_err;	/* Out of window collisions */
	} ae6_stats;
};

/*
 * Flag bit definitions.
 */

#define	AE6_TOPENDING	1

