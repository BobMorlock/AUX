/*
 *   Synertek SY6522 VIA Versatile Interface Adapter
 *
 *      Copyright 1985 UniSoft Corporation
 */

struct via {
	u_char	regb;		/* 0000 register b input and output */
	u_char	jnk1[0x3FF];	/* handshaked rega is here, do not use */
	u_char	ddrb;		/* 0400 data direction register B */
	u_char  jnk2[0x1FF];
	u_char	ddra;		/* 0600 data direction register A */
	u_char	jnk3[0x1FF];
	u_char  t1cl;		/* 0800 timer one low */
	u_char	jnk4[0x1FF];
	u_char	t1ch;		/* 0A00 timer one high */
	u_char	jnk5[0x1FF];
	u_char  t1ll;		/* 0C00 timer one latches low */
	u_char	jnk6[0x1FF];
	u_char	t1lh;		/* 0E00 timer one latches high */
	u_char	jnk7[0x1FF];
	u_char	t2cl;		/* 1000 timer 2 low */
	u_char	jnk8[0x1FF];
	u_char	t2ch;		/* 1200 timer two counter high */
	u_char	jnk9[0x1ff];
	u_char	sr;		/* 1400 shift register */
	u_char	jnka[0x1ff];
	u_char	acr;		/* 1600 auxilary control register */
	u_char	jnkb[0x1ff];
	u_char	pcr;		/* 1800 peripheral control register */
	u_char	jnkc[0x1ff];
	u_char	ifr;		/* 1A00 interrupt flag register */
	u_char	jnkd[0x1ff];
	u_char	ier;		/* 1C00 interrupt enable register */
	u_char	jnke[0x1ff];
	u_char	rega;		/* 1E00 register A, read and write */
};


/*	Register A contents */

#define	VRA_HEAD	0x20	/* disk head select */
#define	VRA_REV8	0x40	/* zero on via1 if this is a rev8 board */

/*	Auxillary control register contents */

#define	VAC_T2CTL	0x20	/* Timer two control */
#define VAC_T1CONT	0x40	/* Timer one continous couinting */
#define VAC_T1PB7	0x80	/* Timer one drives PB7 */

/*	Interrupt enable register contents */

#define	VIE_CA1		0x02	/* interrupt on CA1 */
#define	VIE_CA2		0x01	/* interrupt on CA2 */
#define	VIE_CB2		0x08	/* interrupt on CB2 */
#define	VIE_CB1		0x10	/* interrupt on CB1 */
#define	VIE_TIM2	0x20	/* timer 2 interrupt */
#define	VIE_TIM1	0x40	/* timer 1 interrupt */
#define	VIE_SET		0x80	/* Set interrupt bits if this is on */
#define	VIE_CLEAR	0x00	/* Clear bits if used */

#define	VIA_SDMA(on)	(((struct via *) VIA2_ADDR)->ier = \
	(on ? VIE_SET : VIE_CLEAR) | VIE_CA2)
#define	VIA_CLRDMA()	(((struct via *) VIA2_ADDR)->ifr = VIE_CA2)

/*
 *	These two variables define the smallest and largest slots
 *		in the system. Use these in order to make your code
 *		portable to systems that have more slots in the
 *		future.
 */

#define	SLOT_LO		0x09		/* the lowest slot */
#define	SLOT_HI		0x0e		/* the highest slot */
