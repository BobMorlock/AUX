/*	@(#)sccio.c	UniPlus VVV.2.1.9	*/
#ifdef HOWFAR
extern int T_sccio;
#endif	HOWFAR
#define SCC_CONSOLE
/*
** These are the minor devices of the printer, and modem ports.
** This information is used to make sure that the correct flow control
** is always present by default on these lines.
*/
#define	MODEM_PORT	0
#define	PRINTER_PORT	1

/*
 *	SCC device driver
 *
 *	Copyright 1986 Unisoft Corporation of Berkeley CA
 *
 *
 *	UniPlus Source Code. This program is proprietary
 *	with Unisoft Corporation and is not to be reproduced
 *	or used in any manner except as authorized in
 *	writing by Unisoft.
 *
 */

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/param.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/seg.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/conf.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/reg.h"
#include <sys/debug.h>
#include "sys/proc.h"
#include "setjmp.h"
#endif lint
#include <sys/scc.h>
#include <sys/uconfig.h>
#include <sys/sysmacros.h>


#define NSCC 2

/*
 *	Local static routines
 */

static int scproc();
static int scparam();
static int scscan();
static int scw5();
static int schup();
static int scrintr();
static int scxintr();
static int scsintr();


/*
 * 	Note: 	support is provided below for other drivers which might wish to
 *		make use of the SCC chip. Two variables are declared:
 *
 *		sc_open		a character array (one per channel) which is
 *				used to mark a device in use so that two drivers
 *				do not access the same device at the same time
 *				(this driver checks this on open and clears it
 *				on close)
 *
 *		sc_intaddr	an array of adddresses of interrupt service
 *				routines (one per channel) which are used
 *				instead of the built in ISRs if they are non
 *				zero.
 *
 *		Between these two hooks it is possible to have multiple 
 *		drivers for the same device coexist without problems
 */

typedef int 		(*procedure_t)();
int 			sc_cnt = NSCC;	/* the number of scc channels */
static unsigned char	sc_modem[NSCC];	/* current modem control state */
static unsigned char	sc_dtr[NSCC];	/* current state of dtr flow control */
static unsigned char	sc_wdtr[NSCC];	/* we are waiting for dtr flow cntrl */
static unsigned char	sc_d5[NSCC];	/* saved register 5 contents */
static unsigned char	sc_brk[NSCC];	/* flag to mark input break in
					   progress */
procedure_t 		sc_intaddr[NSCC];/* indirect isr (hooks) */
char 			sc_open[NSCC];	/* open status (hooks) */
static int		sc_scancnt;	/* the number of devices that need
					   scanning */
struct tty		sc_tty[NSCC];	/* the per-device tty structures */
struct ttyptr		sc_ttptr[] = {	/* tty lookup structures */
		(SCC_ADDR + 2),	&sc_tty[0],
		(SCC_ADDR),	&sc_tty[1],
		0
};

#define	W5ON	(W5TXENABLE | W5RTS | W5DTR)	/* turn on to talk */


#define SCTIME (v.v_hz>>3)				/* scscan interval */

#define	isAside(a)  (((int)(a) & 0x2) == 2)	/* check address for A port */

/*
 * Note: to select baud rate
 *	k = chip_input_frequency/(2 * baud * factor) - 2
 *	put factor in register 9 and k in registers D & C
 *	NOTE:
 *		normally, factor = 16
 *		for this driver, chip_input_frequency = 3684600 Hz
 * scspeeds is a table of these numbers by UNIX baud rate
 */

#define	S(b) (((3686400 / 16) / (b * 2)) - 2)
static int scspeeds[] = {
	S(1),	S(50),	S(75),	S(110),	S(134),	S(150),	S(200),	S(300),
	S(600),	S(1200), S(1800), S(2400), S(4800), S(9600), S(19200), S(38400)
};

/*
 *	table to initialize a port to 9600 baud
 *	ports are initialized OFF, to avoid asserting appletalk type signals.
 */

static char scitable[] = {
	9, 0,
#define	SCCIRESET scitable[1]
	1, 0,
	15, 0x80,
	4, W4CLK16 | W42STOP,
	11, W11RBR | W11TBR,
	10, 0,
	12, S(9600) & 0xFF,		/* 12/13 are baud rate */
	13, (S(9600) >> 8) & 0xFF,	/* speed should be set to sspeed */
	14, W14BRGE,
	3, W38BIT | W3RXENABLE,
	5, W58BIT | W5TXENABLE,
	1, W1RXIALL | W1TXIEN | W1EXTIEN,
	2, 0,				/* auto vector */
	0, W0RXINT,
	9, W9MIE | W9DLC,
};

/*
 * we call this to preinitialize all the ports
 */

scinit()
{
	register struct device *addr;
	register unsigned i;
	register unsigned nsc;

#ifdef SCC_CONSOLE
	addr = (struct device *)sc_ttptr[CONSOLE].tt_addr;
	for (i = 100000; i; i--) {		/* wait for output to drain */
		addr->csr = 1;
	 	if (addr->csr & R1ALLSENT)
			break;
	}
#endif SCC_CONSOLE
	for (nsc = 0; nsc < sc_cnt; nsc++) {	/* init each channel */
		addr = (struct device *)sc_ttptr[nsc].tt_addr;
		if (isAside(addr)) {
			SCCIRESET = W9NV | W9ARESET;
		} else {
			SCCIRESET = W9NV | W9BRESET;
		}
		for (i = 0; i < sizeof(scitable); i++) {
			addr->csr = scitable[i];
		}
		sc_d5[nsc] = W58BIT | W5TXENABLE;
		sc_dtr[nsc] = 0;		/* -modem -dtrflow by default */
		sc_modem[nsc] = 0;
		addr->csr = W0REXT;		/* clear pending status */
	}
#ifdef SCC_CONSOLE
	addr = (struct device *)sc_ttptr[CONSOLE].tt_addr;
	for (i = 100000; i; i--) {		/* wait for channel to settle */
		addr->csr = 1;
	 	if (addr->csr & R1ALLSENT)
			break;
	}
#endif SCC_CONSOLE
}

/* ARGSUSED */
scopen(dev, flag)
register dev_t dev;
{
	register struct tty *tp;
	register struct device *addr;
	int	orig_dev;

	/*
	 *	Check the device's minor number for validity
	 */

	orig_dev = dev;
	dev = minor(dev);
	if (dev >= sc_cnt) {
		return(ENXIO);
	}
	tp = sc_ttptr[dev].tt_tty;

	/*
	 *	Disable interrupts while initialising shared data structures
	 *		and starting the chip.
	 */

	SPL6();

	/*
	 *	If the device is not already open then:
	 *		- initialise the device data structures
	 *		- setup the device parameters
	 *		- sense carrier (for modem control)
	 */

	if ((tp->t_state&(ISOPEN|WOPEN)) == 0) {
		if (sc_open[dev]) {	/* make sure the device is not being */
			SPL0();		/*	used for other purposes */
			return(ENXIO);
		}
		sc_open[dev] = 1;
		tp->t_index = dev;
		tp->t_proc = scproc;
		ttinit(tp);		
		/*
		** If we are the printer port, make sure that
		** the DTR flow control is enabled for Apple handshaking.
		*/
		if ( dev == PRINTER_PORT )
		{
			tp->t_oflag |= (TAB3|OPOST|ONLCR);
			scioctl(orig_dev,UIOCDTRFLOW,0,0);
		}
		/*
		** If we are the modem port, make sure that
		** the MODEM flow control is present.
		*/
		if ( dev == MODEM_PORT )
			scioctl(orig_dev,UIOCMODEM,0,0);
		sc_wdtr[dev] = 0;
		addr = (struct device *)sc_ttptr[dev].tt_addr;
		if (!sc_modem[dev] || !(addr->csr & R0CTS)) {
			tp->t_state = WOPEN | CARR_ON;
		} else {
			tp->t_state = WOPEN;
		}
#ifdef SCC_CONSOLE
		if (dev == CONSOLE) {
			tp->t_iflag = ICRNL | ISTRIP;
			tp->t_oflag = OPOST | ONLCR | TAB3;
			tp->t_lflag = ISIG | ICANON | ECHO | ECHOK;
			tp->t_cflag = sspeed | CS8 | CREAD;
		}
#endif SCC_CONSOLE
		scparam(dev);

		/*
		 *	If necessary start the scan routine to look for DCD 
		 *		(really CTS) changes
		 */

		if (sc_modem[dev] || sc_dtr[dev]) {
			sc_scancnt++;
			if (sc_scancnt == 1)
				scscan();
		}
	}

	/*
	 *	Wait until carrier is present before proceeding
	 */

	if (!(flag & FNDELAY)) {
		while (!(tp->t_state&CARR_ON)) {
			tp->t_state |= WOPEN;
			(void) sleep((caddr_t)&tp->t_rawq, TTOPRI);
		}
	}

	/*
	 *	Renable interrupts and call the line discipline open
	 *		routine to set things going
	 */

	SPL0();
	(*linesw[tp->t_line].l_open)(tp);
	return(0);
}

/* ARGSUSED */
scclose(dev, flag)
register dev_t dev;
{
	register struct tty *tp;

	/*
	 *	Call the line discipline routine to let output drain and to
	 *		shut things down gracefully. If required drop DTR
	 *		to hang up the line.
	 */

	dev = minor(dev);
	sc_open[dev] = 0;
	tp = sc_ttptr[dev].tt_tty;
	(*linesw[tp->t_line].l_close)(tp);
	if (tp->t_cflag & HUPCL) {			/* hang up */
		schup(dev, (struct device *)sc_ttptr[dev].tt_addr);
	}
	if (sc_modem[dev] || sc_dtr[dev]) {		/* remove timeout */
		sc_scancnt--;
		if (sc_scancnt == 0)
			untimeout(scscan, 0);
	}
	return(0);
}

/*
 *	Read simply calls the line discipline to do all the work
 */

scread(dev, uio)
dev_t dev;
struct uio *uio;
{
	register struct tty *tp;

	tp = sc_ttptr[minor(dev)].tt_tty;
	return((*linesw[tp->t_line].l_read)(tp, uio));
}

/*
 *	Write simply calls the line discipline to do all the work
 */

scwrite(dev, uio)
dev_t dev;
struct uio *uio;
{
	register struct tty *tp;

	tp = sc_ttptr[minor(dev)].tt_tty;
	return((*linesw[tp->t_line].l_write)(tp, uio));
}

/*
 *	The proc routine does all the work. It takes care of requests from
 *		the line discipline.
 */

static
scproc(tp, cmd)
register struct tty *tp;
int cmd;
{
	register struct ccblock *tbuf;
	register struct device *addr;
	register int dev;
	register int s;
	extern ttrstrt();

	/*
	 *	disable interrupts in order to synchronise with the device
	 */

	s = spl6();
	dev = tp->t_index;
	addr = (struct device *)sc_ttptr[dev].tt_addr;
	switch (cmd) {

	case T_TIME:
		scw5(dev, addr, 0);		/* clear break */
		tp->t_state &= ~TIMEOUT;
		goto start;

	case T_WFLUSH:
		tbuf = &tp->t_tbuf;		/* clear the output buffer */
		tbuf->c_size -= tbuf->c_count;
		tbuf->c_count = 0;
		if (sc_wdtr[dev]) {
			tp->t_state &= ~BUSY;
			sc_wdtr[dev] = 0;
		}
		/* fall through */

	case T_RESUME:				
		tp->t_state &= ~TTSTOP;		/* start output again */
		goto start;

	case T_OUTPUT:
start:						/* output data */
		if (tp->t_state & (TTSTOP|TIMEOUT|BUSY))
			break;
		if (tp->t_state & TTXOFF) {	/* send an XOFF */ 
			tp->t_state &= ~TTXOFF;
			tp->t_state |= BUSY;
			addr->data = CSTOP;
			break;
		}
		if (tp->t_state & TTXON) {	/* send an XON */
			tp->t_state &= ~TTXON;
			tp->t_state |= BUSY;
			addr->data = CSTART;
			break;
		}

		/*
		 *	If there is no data in the buffer, get some more. If no
		 *		more is available then return
		 */

		tbuf = &tp->t_tbuf;
		if ((tbuf->c_ptr == 0) || (tbuf->c_count == 0)) {
			if (tbuf->c_ptr)
				tbuf->c_ptr -= tbuf->c_size;
			if (!(CPRES & (*linesw[tp->t_line].l_output)(tp)))
				break;
		}

		/*
		 *	If DTR flow control is enabled and DTR is not asserted
		 *		then wait until it is
		 */

		if (sc_dtr[dev] && addr->csr&R0CTS) { 
			tp->t_state |= BUSY;
			sc_wdtr[dev] = 1;
			break;
		}

		/*
		 *	Output a character, set the busy bit until it is done
		 */

		tp->t_state |= BUSY;
		addr->data = *tbuf->c_ptr++;
		tbuf->c_count--;
		break;

	case T_SUSPEND:				/* stop all output */
		tp->t_state |= TTSTOP;
		break;

	case T_BLOCK:				/* send XOFF to block input*/
		tp->t_state &= ~TTXON;
		tp->t_state |= TBLOCK;
		tp->t_state |= TTXOFF;
		goto start;

	case T_RFLUSH:				/* flush pending input */
		if (!(tp->t_state&TBLOCK))
			break;
		/* fall through */

	case T_UNBLOCK:				/* send XON to restart input */
		tp->t_state &= ~(TTXOFF|TBLOCK);
		tp->t_state |= TTXON;
		goto start;

	case T_BREAK:				/* start transmitting a break */
		scw5(dev, addr, W5BREAK);
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, (caddr_t)tp, v.v_hz>>2);
		break;

	case T_PARM:				/* call the param routine */
		scparam((dev_t)dev);
		break;
	}
	splx(s);
}

/*
 *	scw5 -- write register d5.
 *	   Used to send a break, the intent is to set or clear only those 
 *	bits permitted by the mask.
 */

#define	scw5MASK  W5BREAK

static
scw5(dev, addr, d)
register dev;
register struct device *addr;
int d;
{
	register int s;

	s = spl6();
	sc_d5[dev] &= ~scw5MASK;
	sc_d5[dev] |= d & scw5MASK;
	addr->csr = 5;
	addr->csr = sc_d5[dev];
	splx(s);
}

/*
 *	The ioctl routine mostly handles the MODEM ioctls, all others are
 *		passed to ttiocom to be done.
 */

scioctl(dev, cmd, arg, mode)
register dev_t dev;
register int cmd;
char *arg;
int mode;
{
	register struct tty *tp;
	register int i;

	dev = minor(dev);
	tp = sc_ttptr[dev].tt_tty;
	switch(cmd) {

		/*
		 *	UIOCMODEM: turns on modem control. If DTR flow control
		 *		   was on turn it off and start output.
		 */

	case UIOCMODEM:
		if (sc_dtr[dev]) {
			sc_dtr[dev] = 0;
			if (sc_wdtr[dev]) {
				sc_wdtr[dev] = 0;
				tp->t_state &= ~BUSY;
				(*tp->t_proc)(tp, T_OUTPUT);
			}
		} else 
		if (!sc_modem[dev]) {
			sc_scancnt++;
			if (sc_scancnt == 1)
				scscan();
		}
		sc_modem[dev] = 1;
		return(0);

		/*
		 *	UIOCNOMODEM: turns of modem control and DTR flow control
		 *		   If DTR flow control was on turn it off and
		 *		   start output. If modem control is on allow
		 *		   processes waiting to open to do so.
		 */

	case UIOCNOMODEM:
		if (sc_dtr[dev] || sc_modem[dev]) {
			sc_scancnt--;
			if (sc_scancnt == 0)
				untimeout(scscan, 0);
		}
		if (sc_dtr[dev]) {
			sc_dtr[dev] = 0;
			if (sc_wdtr[dev]) {
				sc_wdtr[dev] = 0;
				tp->t_state &= ~BUSY;
				(*tp->t_proc)(tp, T_OUTPUT);
			}
		}
		sc_modem[dev] = 0;
		if (tp->t_state&WOPEN) {
			tp->t_state |= CARR_ON;
			wakeup((caddr_t)&tp->t_rawq);
		}
		return(0);

		/*
		 *	UIOCDTRFLOW: turns on DTR flow control. If modem control
		 *		is on allow processes waiting to open to do so.
		 */

	case UIOCDTRFLOW:
		if (sc_modem[dev]) {
			sc_modem[dev] = 0;
			if (tp->t_state&WOPEN) {
				tp->t_state |= CARR_ON;
				wakeup((caddr_t)&tp->t_rawq);
			}
		} else 
		if (!sc_dtr[dev]) {
			sc_scancnt++;
			if (sc_scancnt == 1)
				scscan();
		}
		sc_dtr[dev] = 1;
		return(0);

		/*
		 *	UIOCTTSTAT: return the modem control/flow control state
		 */

	case UIOCTTSTAT:
		if (sc_modem[dev]) {
			*arg++ = 1;
		} else {
			*arg++ = 0;
		}
		if (sc_dtr[dev]) {
			*arg++ = 1;
		} else {
			*arg++ = 0;
		}
		*arg = 0;
		return(0);

	default:
		if (ttiocom(tp, cmd, arg, mode)) {
			return(scparam(dev));
		} else {
			return(u.u_error);
		}
	}
}

/*
 *	setup device dependant physical parameters
 */

static
scparam(dev)
register dev_t dev;
{
	register struct tty *tp;
	register struct device *addr;
	register int flag;
	register int s;
	register int w4;
	register int w5;
	register int speed;

	dev = minor(dev);
	tp = sc_ttptr[dev].tt_tty;
	flag = tp->t_cflag;
	addr = (struct device *)sc_ttptr[dev].tt_addr;
	if (((flag&CBAUD) == B0)) {
		schup(dev, addr);
		return(0);
	}
	w4 = W4CLK16;
	if (flag & CSTOPB) {
		w4 |= W42STOP;
	} else {
		w4 |= W41STOP;
	}
	w5 = W5TXENABLE | W5RTS | W5DTR;
	switch(flag & CSIZE) {
	case CS5:
		w5 |= W55BIT;
		break;
	case CS6:
		w5 |= W56BIT;
		break;
	case CS7:
		w5 |= W57BIT;
		break;
	case CS8:
		w5 |= W58BIT;
		break;
	}
	if (flag & PARENB) {
		if (flag & PARODD) {
			w4 |= W4PARENABLE;
		} else {
			w4 |= W4PARENABLE | W4PAREVEN;
		}
	}
	speed = scspeeds[flag&CBAUD];
	s = spl6();
	addr->csr = 4;
	addr->csr = w4;
	addr->csr = 12;
	addr->csr = speed;
	addr->csr = 13;
	addr->csr = speed >> 8;
	addr->csr = 5;
	addr->csr = w5;
	sc_d5[dev] = w5;
	splx(s);
	return(0);
}

/*
 *	This routine hangs up a modem by removing DTR
 */

static
schup(dev, addr)
register dev_t dev;
register struct device *addr;
{
	register int s;

	dev = minor(dev);
	s = spl6();
	addr->csr = 5;
	sc_d5[dev] = addr->csr = W5TXENABLE | W58BIT;	/* turn off DTR/RTS */
	splx(s);
}

/*
 *	The interrupt service routine is called from ivec.s and determines which
 *	service routines should be called
 */

scintr(ap)
struct args *ap;
{
	register struct device *addr;
	register int csr;
	register int dev;

	for (dev = 0; dev < sc_cnt; dev++) {
		addr = (struct device *)sc_ttptr[dev].tt_addr;
		if (sc_intaddr[dev]) {			/* hook for other */
			(*sc_intaddr[dev])(dev, addr);	/*  protocols */
			continue;
		}
		for (;;) {
			csr = addr->csr;
			if (!(csr&(R0RXRDY|R0BREAK)))
				break;
			if (csr&R0BREAK) {
				if (sc_brk[dev]) 
					break;
				sc_brk[dev] = 1;
			} else
			if (sc_brk[dev]) {
				csr = addr->data;	/* read 0 char */
				sc_brk[dev] = 0;
				addr->csr = W0REXT;	/* reset external status
							   interrupts */
				continue;
			}
			addr->csr = 1;
			if ((addr->csr & (R1PARERR|R1OVRERR|R1FRMERR)) ||
			    (csr & R0BREAK)) {
				scsintr(dev, addr);
			} else {
				scrintr(dev, addr);
			}
		}
		if (addr->csr & R0TXRDY) {
			scxintr(dev, addr);
		}
	}
}

static
scrintr(dev, addr)
register int dev;
register struct device *addr;
{
	register struct ccblock *cbp;
	register struct tty *tp;
	register int c;
	register int lcnt;
	register int flg;
	register char ctmp;
	char lbuf[3];

	addr->csr = W0RXINT;	/* reenable receiver interrupt */
	addr->csr = W0RIUS;	/* reset interrupt */
	c = addr->data & 0xFF;
#if defined(DEBUG) && defined(SCC_CONSOLE)
	if (dev == CONSOLE && ((c & 0x7F) == 1)) {
        	debug();
        	return;
	}
#endif	/* DEBUG && SCC_CONSOLE */
	sysinfo.rcvint++;
	tp = sc_ttptr[dev].tt_tty;
	flg = tp->t_iflag;
	if (flg&ISTRIP)
		c &= 0x7f;
	if (flg & IXON) {
		if (tp->t_state & TTSTOP) {
			if (c == CSTART || tp->t_iflag & IXANY)
				(*tp->t_proc)(tp, T_RESUME);
		} else {
			if (c == CSTOP)
				(*tp->t_proc)(tp, T_SUSPEND);
		}
		if (c == CSTART || c == CSTOP)
			return;
	}
	if (tp->t_rbuf.c_ptr == NULL) {
		return;
	}
	lcnt = 1;
	if ((flg&ISTRIP) == 0) {
		if (c == 0xff && flg&PARMRK) {
			lbuf[1] = 0xff;
			lcnt = 2;
		}
	}

	/*
	 * Stash character in r_buf
	 */

	cbp = &tp->t_rbuf;
	if (lcnt != 1) {
		lbuf[0] = c;
		while (lcnt) {
			*cbp->c_ptr++ = lbuf[--lcnt];
			if (--cbp->c_count == 0) {
				cbp->c_ptr -= cbp->c_size;
				(*linesw[tp->t_line].l_input)(tp, L_BUF);
			}
		}
		if (cbp->c_size != cbp->c_count) {
			cbp->c_ptr -= cbp->c_size - cbp->c_count;
			(*linesw[tp->t_line].l_input)(tp, L_BUF);
		}
	} else {
		*cbp->c_ptr = c;
		cbp->c_count--;
		(*linesw[tp->t_line].l_input)(tp, L_BUF);
	}
}

static
scxintr(dev, addr)
register int dev;
register struct device *addr;
{
	register struct tty *tp;

	sysinfo.xmtint++;
	addr->csr = W0RTXPND;	/* reset transmitter interrupt */
	addr->csr = W0RIUS;	/* reset interrupt */
	tp = sc_ttptr[dev].tt_tty;
	tp->t_state &= ~BUSY;
	scproc(tp, T_OUTPUT);
}

static
scsintr(dev, addr)
register int dev;
register struct device *addr;
{
	register struct ccblock *cbp;
	register struct tty *tp;
	register int c;
	register int lcnt;
	register int flg;
	register char ctmp;
	register int csr;
	register unsigned char stat;
	char lbuf[3];

	sysinfo.rcvint++;
	c = addr->data & 0xFF;	/* read data BEFORE reset error */
	csr = addr->csr;	/* read register zero */
	addr->csr = 0x1;	/* cmd to read register 1 */
	stat = addr->csr;	/* read the status */
	addr->csr = W0RERROR;	/* reset error condition */
	addr->csr = W0REXT;	/* reset external status interrupts */
	addr->csr = W0RXINT;	/* reinable receiver interrupt */
	addr->csr = W0RIUS;	/* reset interrupt under service */
	tp = sc_ttptr[dev].tt_tty;
	if (tp->t_rbuf.c_ptr == NULL)
		return;
	if (tp->t_iflag & IXON) {
		ctmp = c & 0x7f;
		if (tp->t_state & TTSTOP) {
			if (ctmp == CSTART || tp->t_iflag & IXANY)
				(*tp->t_proc)(tp, T_RESUME);
		} else {
			if (ctmp == CSTOP)
				(*tp->t_proc)(tp, T_SUSPEND);
		}
		if (ctmp == CSTART || ctmp == CSTOP)
			return;
	}
	/*
	 * Check for errors
	 */
	if (csr & R0BREAK) {
		c = FRERROR;
	}
	lcnt = 1;
	flg = tp->t_iflag;
	if (stat & (R1PARERR |R1OVRERR|R1FRMERR)) {
		if ((stat & R1PARERR ) && (flg & INPCK))
			c |= PERROR;
		if (stat & R1OVRERR)
			c |= OVERRUN;
		if (stat & R1FRMERR)
			c |= FRERROR;
	}
	if (c&(FRERROR|PERROR|OVERRUN)) {
		if ((c&0xff) == 0) {
			if (flg&IGNBRK)
				return;
			if (flg&BRKINT) {
				(*linesw[tp->t_line].l_input)(tp, L_BREAK);
				return;
			}
		} else {
			if (flg&IGNPAR)
				return;
		}
		if (flg&PARMRK) {
			lbuf[2] = 0xff;
			lbuf[1] = 0;
			lcnt = 3;
			sysinfo.rawch += 2;
		} else {
			c = 0;
		}
	} else {
		if (flg&ISTRIP) {
			c &= 0x7f;
		} else {
			if (c == 0xff && flg&PARMRK) {
				lbuf[1] = 0xff;
				lcnt = 2;
			}
		}
	}

	/*
	 * Stash character in r_buf
	 */

	cbp = &tp->t_rbuf;
	if (lcnt != 1) {
		lbuf[0] = c;
		while (lcnt) {
			*cbp->c_ptr++ = lbuf[--lcnt];
			if (--cbp->c_count == 0) {
				cbp->c_ptr -= cbp->c_size;
				(*linesw[tp->t_line].l_input)(tp, L_BUF);
			}
		}
		if (cbp->c_size != cbp->c_count) {
			cbp->c_ptr -= cbp->c_size - cbp->c_count;
			(*linesw[tp->t_line].l_input)(tp, L_BUF);
		}
	} else {
		*cbp->c_ptr = c;
		cbp->c_count--;
		(*linesw[tp->t_line].l_input)(tp, L_BUF);
	}
}

static
scscan()
{
	register int i;
	register struct tty *tp;
	register struct device *addr;

	timeout(scscan, (caddr_t)0, SCTIME);
	for (i = 0; i < sc_cnt; i++) {
		addr = (struct device *)sc_ttptr[i].tt_addr;
		addr->csr = W0REXT;	/* update CTS */
		tp = sc_ttptr[i].tt_tty;
		if (!(addr->csr & R0CTS)) {
			if (sc_dtr[i]) {
				if (sc_wdtr[i]) {
					sc_wdtr[i] = 0;
					tp->t_state &= ~BUSY;
					(*tp->t_proc)(tp, T_OUTPUT);
				}
			} else
			if (sc_modem[i]) {
				if ((tp->t_state&CARR_ON) == 0) {
					tp->t_state |= CARR_ON;
					if (tp->t_state&WOPEN) {
						wakeup((caddr_t)&tp->t_rawq);
					}
				}
			}
		} else {
			if (sc_modem[i]) {
				if (tp->t_state&CARR_ON) {
					tp->t_state &= ~CARR_ON;
					if (tp->t_state&ISOPEN) {
						ttyflush(tp, FREAD|FWRITE);
						signal(tp->t_pgrp, SIGHUP);
					}
				}
			}
		}
	}
}

#ifdef SCC_CONSOLE
scputchar(c)
register int c;
{
	register struct device *addr ;
	register int s;
	register int i;
#ifndef PMMU
	extern int mmu_on;

	if (!mmu_on) {
		addr = (struct device *)0x50F04002;
	} else {
		addr = (struct device *)sc_ttptr[CONSOLE].tt_addr;
	}
#else PMMU
	addr = (struct device *)sc_ttptr[CONSOLE].tt_addr;
#endif PMMU
	s = spl7();
	if ((sc_d5[CONSOLE] & W5ON) != W5ON) {
		sc_d5[CONSOLE] |= W58BIT | W5ON;
		addr->csr = 5;
		addr->csr = sc_d5[CONSOLE];
	}
	if (c == '\n')
		scputchar('\r');
	i = 100000;
	while ((addr->csr & R0TXRDY) == 0 && --i)
			;
	addr->data = c;
	splx(s);
}
#ifdef DEBUG
scgetchar()
{
	register struct device *addr ;
	register int c;
	register int s;
	register int i;
#ifndef PMMU
	extern int mmu_on;

	if (!mmu_on) {
		addr = (struct device *)0x50F04002;
	} else {
		addr = (struct device *)sc_ttptr[CONSOLE].tt_addr;
	}
#else PMMU
	addr = (struct device *)sc_ttptr[CONSOLE].tt_addr;
#endif PMMU
	s = spl7();
	while ((addr->csr & R0RXRDY) == 0)
		;
	c = addr->data & 0x7F;	/* read data BEFORE reset error */
	addr->csr = W0RERROR;	/* reset error condition */
	addr->csr = W0RXINT;	/* reinable receiver interrupt */
	addr->csr = W0REXT;	/* reset external status interrupts */
	addr->csr = W0RIUS;	/* reset interrupt under service */
	splx(s);
	return(c);
}
#endif DEBUG
#endif SCC_CONSOLE
