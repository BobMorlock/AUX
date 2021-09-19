#ifdef HOWFAR
int	T_scsi = 0;
#endif HOWFAR
/*
 * NCR 5380 SCSI interface.
 *
 * (C) 1986 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#ifndef	STANDALONE
#include "sys/param.h"
#include "sys/uconfig.h"
#include "sys/var.h"
#endif	STANDALONE
#include "sys/debug.h"
#endif lint
#include "sys/ncr5380.h"
#include "sys/via6522.h"

/*	DELAY -- A generic delay.  Not tuned to SCSI delay values */

#ifdef	lint
#define DELAY
#else	lint
#define	DELAY { int i, j;  i = 1; j = i; }
#endif	lint

static	scsiabt(), scsiio(), scsiin(), scsiout();
static	scsigo(), watchdog(), finishreset(), scsisense();
static	void	scsireset(), startdma();
#if defined(DEBUG) || defined(HOWFAR)
static	printscsi();
#endif defined(DEBUG) || defined(HOWFAR)

#ifdef	SCX_MEASURE
int	scx_c_stat;
int	scx_c_msg;
int	scx_in_phase;
int	scx_out_phase;
int	scx_out_exit;

#define	scx(n)	(++(n))
#else	SCX_MEASURE

#define	scx(n)	
#endif	SCX_MEASURE

#define	NSCSI 7

/*	SCSI global state.
 *	     This reflects the state of the current activity of the ncr
 *	chip.
 */

#define	SG_IDLE		0	/* no activity */
#define	SG_ACTIVE	1	/* some request is being processed */
#define	SG_WAIT		2	/* a request is pending */
#define	SG_RESET	3	/* The bus is being reset */
#define	SG_CLEAR	4	/* A request cleared after a reset */

/*	SCSI task states.
 *	     These states refect the activity currently in progress for
 *	each SCSI ID.
 */

#define	ST_IDLE		0	/* no activity */
#define	ST_ACTIVE	1	/* active and not waiting */
#define	ST_CHIP		2	/* waiting for the ncr chip */
#define	ST_READ		3	/* currently waiting for a DMA read interrupt */
#define	ST_WRITE	4	/* currently waiting for DMA write interrupt */
#define	ST_POST		5	/* waiting for stat phase */
#define	ST_WRAP		6	/* This request is complete with no error */

#ifndef	STANDALONE
#define	splcopy()	spl2()	/* copy operations at interrupt priority */
#define	splintr()	spl2()	/* priority when we are interrupted */
#define	SPLCOPY()	SPL2()
#define SPLINTR()	SPL7()
#else	STANDALONE
#define	splcopy()	0x2700	/* one priority in standalone */
#define	splintr()	0x2700
#define	splhi()		0x2700
#define	SPLCOPY()
#define SPLINTR()
#define	splx(s)
#undef	VIA_SDMA
#define	VIA_SDMA(x)		/* no DMA changes for standalone */
#undef	VIA_CLRDMA
#define	VIA_CLRDMA()		/* no interrupts, so not needed */
#define	viaclrius()		/* no interrupts for standalone */
#define	panic(s)	printf(s) ; while(1) ;
#define bzero(cp, n)	memset(cp, 0, n)
#ifdef	HOWFAR
#define	T_traceoff 0
#define T_tracelvl 0
#endif	HOWFAR
#endif	STANDALONE

static  short needed;	/* flag that a request is waiting */
static 	short sg_state;	/* SCSI global state variable */
static	short curtask;	/* index of current task */
static	short reset_savestate;	/* state saved over a reset */

struct	scsitask {
	struct scsireq *req;
	short	state;
	long	devchar;	/* device characteristics */
	} scsitask[NSCSI];

/*	scsistrings -- error message strings.
 *	This global data structure is indexed by scsi return code.  It is
 *	available to any driver which may want to print an error message.
 */

char	*scsi_strings[] = {
	"SCSI driver implementation error",	/* code 0, there is no error */
	"SCSI bus busy",			/* 1 */
	"Error during SCSI command",		/* 2 */
	"Error during SCSI status",		/* 3 */
	"Error during SCSI sense command",	/* 4 */
	"No SCSI unit",				/* 5 */
	"SCSI timeout",				/* 6 */
	"Driver(s) for SCSI device placed multiple conncurent requests", /* 7 */
	"More data than SCSI device requested",	/* 8 */
	"Less data than SCSI device requested", /* 9 */
	"SCSI extended status returned"		/* 10 */
	};

#define	TOUTTIME	5
static	short tout;		/* flag that time out daemon running */
static	short unjam;		/* flag for timeouts of transactions */
static	short isinit;		/* flag that driver has initialized */
	
/*	finishreset -- clean up following reset.
 *	    After pulsing the RST line, we give devices 2 seconds, and then
 *	we complete the offending request and go back to business as usual.
 */
 
static
finishreset()

{
	register struct scsitask *stp;
	register struct scsireq *rqp;
	register i;
	
	(void) spl2();
	TRACE(T_scsi, ("finish reset called\n"));
	if(sg_state != SG_RESET) {
		panic("software error in SCSI finish reset");
	}
	for(i = 0, stp = scsitask; i < NSCSI; ++i, ++stp) {
		switch(stp->state) {
		case ST_READ:
		case ST_WRITE:
		case ST_POST:
			rqp = stp->req;
			rqp->ret = SST_TIMEOUT;
			stp->state = ST_IDLE;
			if(rqp->faddr)
				(*rqp->faddr)(rqp);
			break;
		}
	}
	if(sg_state == SG_CLEAR) {
		reset_savestate = SG_IDLE;
		TRACE(T_scsi, ("reset cleared\n"));
	}
	switch(reset_savestate) {
	case SG_WAIT:
		sg_state = SG_IDLE;
		break;
	default:
		sg_state = reset_savestate;
		break;
	}
	if(sg_state == SG_IDLE) {
		TRACE(T_scsi, ("scsigo from finshreset\n"));
		scsigo();
	}
}

/*	scsiabt -- abort this transaction.
 *	     This routine is intended to stop the current device activity,
 *	and to return the bus to the free condition.
 *	     It seems necessary to assume that the target is still responding
 *	to SCSI protocol.
 */

static
scsiabt(stat, msg)

u_char	*stat, *msg;	/* status byte and message in byte */
{
	struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	int	phase;
	int	i;
	char	jnk[50];

	TRACE(T_scsi, ("scsiabt called\n"));
	if(sg_state == SG_RESET)
		return;
	VIA_SDMA(NO);
	unjam = 0;
	bzero(jnk, sizeof(jnk));
	while(ncr->curr_stat & SCS_BSY) {
		TRACE(T_scsi, ("scsiabt: phase is %d\n", 
					(ncr->curr_stat >> 2) & 0x7));
		switch((phase = ((ncr->curr_stat >> 2) & 0x7))) {
		case SPH_STAT:
			TRACE(T_scsi, ("abt stat\n"));
			if(ncr->init_comm & SIC_DB) {
				ncr->mode = 0;	/* cancel dma mode */
			}
			ncr->init_comm = 0;
			if(scsicomplete(stat, msg) == 0) {
				i = 0;
				while(ncr->curr_stat & SCS_BSY) {
					if(++i > 2000)
						break;
					DELAY;
				}
				if(i <= 2000)
					return;
			}
			break;
		case SPH_MIN:
			TRACE(T_scsi, ("abt message in\n"));
			ncr->mode = 0;
			(void)scsiin((caddr_t)msg, 1, SPH_MIN, 0);	
			break;
		case SPH_MOUT:
			TRACE(T_scsi, ("abt message out\n"));
			ncr->mode = 0;
			ncr->init_comm &= ~SIC_ATN;
			jnk[0] = SMG_ABT;
			(void)scsiout(jnk, 1, SPH_MOUT, 0);
			break;
		default:
			ncr->mode = 0;
			ncr->init_comm = SIC_ATN;
			for(i = 0; i < 500; ++i) {
				DELAY;
			}
			if(((ncr->curr_stat >> 2) & 0x7) != phase)
				break;
			if(phase & 1) 
				(void)scsiin(jnk, sizeof(jnk), phase, 0);
			else {
				(void)scsiout(jnk, sizeof(jnk), phase, 0);
			}
			break;
		}
		ncr->mode = 0;
	}
	ncr->init_comm = 0;	/* Make certain ATN is down */
	TRACE(T_scsi, ("scsiabt() complete\n"));
}

	
/*	scsicomplete -- finish command and get status.
 *	After a data in or data out phase, this routine may be called
 *	to get status from the target.
 *	This must be called to allow the bus to return to the free state.
 */

int
scsicomplete(stat, msg)

u_char *stat;
u_char *msg;
{
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	
	TRACE(T_scsi, ("scsicomplete\n"));
	if(ncr->mode & SMD_DMA) {
		while((ncr->bus_status & (SBS_PHASE | SBS_DMA)) !=
					(SBS_PHASE | SBS_DMA)) {
			if(!(ncr->bus_status & SBS_PHASE)) {
				TRACE(T_scsi, ("no phase match in complete\n"));
				ncr->mode = 0;
				ncr->init_comm = 0;
				return(-1);
			}
		}
		*stat = *((char *)SDMA_ADDR);
		DELAY;
		ncr->mode = 0;
	}
	else {
		ncr->targ_comm = SPH_STAT;
		DELAY;
		while(!(ncr->curr_stat & SCS_REQ)) {
			if(unjam > 1 || !(ncr->curr_stat & SCS_BSY)) {
				return(-1);
			}
			scx(scx_c_stat);
		}
		if(!(ncr->bus_status & SBS_PHASE)) {
			TRACE(T_scsi, ("scsicomplete: phase mismatch\n"));
			return(-1);
		}
		*stat = ncr->curr_data;
		ncr->init_comm = SIC_ACK;
		while((ncr->curr_stat & SCS_REQ)) {
			if(unjam > 1 || !(ncr->curr_stat & SCS_BSY)) {
				return(-1);
			}
		}
	}
	ncr->init_comm = 0;

	ncr->targ_comm = SPH_MIN;
	while(!(ncr->curr_stat & SCS_REQ)) {
		if(unjam > 1 || !(ncr->curr_stat & SCS_BSY)) {
			return(-1);
		}
		scx(scx_c_msg);
	}
	*msg = ncr->curr_data;
	ncr->init_comm = SIC_ACK;
	while((ncr->curr_stat & SCS_REQ)) {
		if(unjam > 1 || !(ncr->curr_stat & SCS_BSY)) {
			return(-1);
		}
	}
	ncr->init_comm = 0x0;
	return(0);
}


/*	scsidevchar -- set scsi device characteristics.
 *	     Set the bit mapped word of device characteristics for this
 *	device id.
 */

scsidevchar(id, stuff)

long	stuff;
{
	if(id < 0 || id >= NSCSI)
		return(-1);
	scsitask[id].devchar = stuff;
	return(0);
}


/*	scsidintr -- take dma interrupt.
 *	    The DRQ line does not depend on phase match.  So, this interrupt
 *	happens almost instantly on writes.
 */

scsidintr() 

{ 
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register struct scsitask *stp;
	register struct scsireq *req;
	int	s;

	VIA_SDMA(NO);
	viaclrius();
	TRACE(T_scsi, ("dmaintr\n"));
	if(sg_state != SG_WAIT) {
		panic("bad scsi driver interrupt handling"); /* can't happen */
	}
	sg_state = SG_ACTIVE;
	stp = &scsitask[curtask];
	req = stp->req;
	/* For writes, we come in here right away. XXX */
	if(!(ncr->bus_status & SBS_PHASE) 
#ifndef	STANDALONE
	    && !(ncr->init_comm & SIC_DB)
#endif	STANDALONE
								) {
		TRACE(T_scsi, ("lost phase match\n"));
		if(stp->state != ST_POST)
			req->ret = SST_MORE;
		else if((ncr->curr_stat & (7 << 2)) <= (SPH_DIN << 2))
			req->ret = SST_LESS;
		else
			req->ret = SST_COMP;
		ncr->init_comm = 0;
		ncr->mode = 0;
		scsiwrap(curtask, stp);
		return;
	}
	switch(stp->state) {
	case ST_READ:
		s = splcopy();
		if((req->datasent = scsiin(req->databuf, (int)req->datalen, 
		    SPH_DIN, (int)(stp->devchar & SDC_532))) == 
		    (int)req->datalen) {
			stp->state = ST_WRAP;
		}
		else if(unjam > 1)
			req->ret = SST_TIMEOUT;
		else
			req->ret = SST_MORE;
		splx(s);
		scsiwrap(curtask, stp);
		break;
	case ST_WRITE:
		s = splcopy();
		req->datasent = scsiout(req->databuf, (int)req->datalen, 
			SPH_DOUT, (int)(stp->devchar & SDC_532));
		splx(s);
		if(req->datasent != req->datalen) {
			if(unjam > 1)
				req->ret = SST_TIMEOUT;
			else
				req->ret = SST_MORE;
			scsiwrap(curtask, stp);
		}
		else {
			sg_state = SG_WAIT;
			stp->state = ST_POST;
			startdma(SPH_STAT, YES);
			VIA_SDMA(YES);
		}
		break;
	case ST_POST:
		stp->state = ST_WRAP;
		scsiwrap(curtask, stp);
		break;
	default:
		printf("bad state = 0x%x\n", stp->state);
		panic("bad state in scsidintr()\n");	/* can't happen */
		break;
	}
	TRACE(T_scsi, ("dmaintr done\n"));
}


/*	scsigo -- place a request.
 *	     This routine arbitrates for the device, and sends each
 *	part of the request out to the device.
 *	     If multiple requests are queued, this routine selects
 *	based on SCSI ID.
 *	     Always called at interrupt priority.
 *	     This routine assumes it is called only when it should be called,
 *	ie the global state that was previously in effect is over, and it's
 *	time to move on.
 */

static
scsigo()

{
	register id;
	register struct	scsitask *stp;
	register struct scsireq *req;
	int	s;


	/*	locate task to service */

	for(id = NSCSI-1, stp = &scsitask[NSCSI-1]; id >= 0; --id, --stp) {
		if(stp->state == ST_CHIP)
			break;
	}
	if(id < 0) {
		TRACE(T_scsi, ("scsigo() idle\n"));
		sg_state = SG_IDLE;
		needed = NO;
		return;
	}
	TRACE(T_scsi, ("scsigo for id %d\n", id));
	unjam = 0;
	curtask = id;
	req = stp->req;
	stp->state = ST_ACTIVE;
	sg_state = SG_ACTIVE;
	s = splhi();
	if(!tout) {
#ifndef	STANDALONE
		timeout(watchdog, 0, TOUTTIME * v.v_hz);
#endif	STANDALONE
		tout = 1;
	}
	SPLCOPY();

	/* Do the arbitration */
	if(scsiget()) {
		req->ret = SST_BSY;
		scsiwrap(id, stp);
		goto done;
	}

	/* Select the device */
	if(scsiselect(id)) {
		req->ret = SST_SEL;
		scsiwrap(id, stp);
		goto done;
	}

	/* Send the command */
	TRACE(T_scsi, ("command = 0x%x\n", req->cmdbuf[0]));
	if(scsiout(req->cmdbuf, (int)req->cmdlen, SPH_CMD, 0) != 
			(int)req->cmdlen) {
		TRACE(T_scsi, ("command length error\n"));
		if(unjam > 1)
			req->ret = SST_TIMEOUT;
		else
			req->ret = SST_CMD;
		scsiwrap(id, stp);
		goto done;
	}
	/* Send or recieve data asynchronously */
	if(req->databuf == 0) 
		req->datalen = 0;
	scsiio(stp);
done:
	splx(s);
}


/*	scsig0cmd -- make group 0 command block.
 *	Fill in the blanks of a six byte command.  A buffer with space
 *	for the command block is initialized and linked into the general
 *	scsi request format being prepared.
 */

scsig0cmd(req, op, lun, addr, len, ctl)

struct scsireq *req;	/* request being assembled */
int	op;		/* the SCSI op code */
int	lun;		/* The logical unit number for the comand (1..8) */
register addr;		/* The address field of the SCSI command block */
int	len;		/* The length field of the command */
int	ctl;		/* contents of byte 5 */
{
	struct scsig0cmd *cmd;

#ifdef lint	/* possible pointer alignment problem */
	cmd = (struct scsig0cmd *)NULL;
#else lint
	cmd = (struct scsig0cmd *)req->cmdbuf;
#endif lint
	cmd->op = op;
	cmd->addrH = ((lun & 0x7) << 5) | (((int)addr >> 16) & 0x1F);
	cmd->addrM = (int)addr >> 8;
	cmd->addrL = (int)addr;
	cmd->len = len;
	cmd->ctl = ctl;
	req->cmdbuf = (caddr_t) cmd;
	req->cmdlen = 6;
}


/*	scsiget -- perform SCSI bus arbitration.
 *	XXX Not implemented.  Must be implemented later.
 */

int
scsiget()

{
	register int i;
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	u_char	jnk;

	for(i = 0; ncr->curr_stat & SCS_BSY; ++i) {
		if(i >= 10000) {		/* TUNE */
#ifdef	DEBUG
			printscsi();
			printf("SCSI error:  Busy asserted\n");
#endif	DEBUG
			scsiabt(&jnk, &jnk);
			return(-1);
		}
	}
	return(0);
}


/*	scsiin -- input bytes from the scsi bus.
 *	The "expected" bus phase is set to the given phase, and bytes
 *	are read.  The routine returns the number of bytes actually
 *	transferred.
 */

static int
scsiin(buf, len, phase, extended)

register caddr_t buf;
register int len;
int	phase;
int	extended;		/* use 532 byte sectoring if true */
{
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register bstat;
	register len2;
	int	start;

	start = len;
	ncr->targ_comm = phase;
	while(!(ncr->bus_status & SBS_PHASE)) {
		if(unjam > 1) {
			return(0);
		}
		scx(scx_in_phase);
	}
	if(!(ncr->mode & SMD_DMA)) {
		ncr->mode = SMD_DMA;
		ncr->start_Ircv = 0;
	}
	TRACE(T_scsi, ("about to scsiin\n"));
	while(len > 0) {
		while((bstat = (ncr->bus_status & (SBS_PHASE | SBS_DMA))) != 
						(SBS_PHASE | SBS_DMA)) {
			if(!(bstat & SBS_PHASE)) {
errout:
#ifdef	DEBUG
				printf
		("scsiin: phase changed from 0x%x to 0x%x with %d bytes left\n",
					phase, (ncr->curr_stat>>2) & 0x7, len);
				printscsi();
#else	DEBUG
				TRACE(T_scsi,
		("scsiin: phase changed from 0x%x to 0x%x with %d bytes left\n",
					phase, (ncr->curr_stat>>2) & 0x7, len));
#endif	DEBUG
				ncr->mode = 0;
				return(start - len);	/* bytes transferred */
			}
			if(unjam > 1) {
				goto errout;
			}
		}
		*buf++ = *((char *)SDMA_ADDR);
		if(!(--len & (512-1))) {
		    unjam = 0;
		    if(extended && !(start & (512-1))) {
			for(len2 = 20; len2 > 0; --len2) {
			    while((bstat = 
			    (ncr->bus_status & (SBS_PHASE | SBS_DMA))) != 
				    (SBS_PHASE | SBS_DMA)) {
				if(!(bstat & SBS_PHASE) || unjam > 1)
				    goto errout;
			    }
			    bstat = *((char *)SDMA_ADDR);	/* junk */
			}
		    }
		}
	}
	ncr->mode = 0;
	TRACE(T_scsi, ("scsiin complete\n"));
	return(start);
}


/*	scsiinit -- do initialization.
 *	     Reset the bus.  Called at boot up.
 */

scsiinit()

{
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;

	if(ncr->curr_stat & SCS_BSY)
		scsireset();
	isinit = 1;
}


/*	scsiio -- do data in or data out phase.
 *	    Following an scsiget(), scsiselect(), and command phase, data
 *	is read or written from the device.
 */

static
scsiio(stp)

struct scsitask *stp;
{
#ifdef	STANDALONE
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
#endif  STANDALONE
	register struct scsireq *req;

	req = stp->req;
	SPLINTR();
	if(req->datalen == 0) {
		TRACE(T_scsi, ("status setup\n"));
		stp->state = ST_POST;
		startdma(SPH_STAT, YES);
	}
	else if(req->flags & SRQ_READ) {
		TRACE(T_scsi, ("read setup\n"));
		stp->state = ST_READ;
		startdma(SPH_DIN, YES);
	}
	else {
		/* Measurements with the ST225N drive showed that on writes
		 * the delay was just as likely to occur between the command
		 * and the acceptance of data, as between the completion of data
		 * and the arrival of the stat byte.
		 */
		TRACE(T_scsi, ("write setup\n"));
		stp->state = ST_WRITE;
		startdma(SPH_DOUT, NO);
	}
	sg_state = SG_WAIT;
	VIA_SDMA(YES);
#ifdef	STANDALONE
	do {
		int spincount;
		
		spincount = 0;
		while((ncr->bus_status & (SBS_PHASE | SBS_DMA)) !=
						(SBS_PHASE | SBS_DMA)) {
			if((++spincount % 10000) == 0) {
				if((ncr->curr_stat & (0x7<<2)) >= (SPH_STAT<<2)) {
					break;			
				}
			}
			if(spincount > 500000
			     && (ncr->curr_stat & (0x7 << 2)) <= (SPH_DIN << 2))
				break;
		}
		scsidintr();
	} while(sg_state == SG_WAIT);
#endif	STANDALONE
	return;
}


/*	scsiout -- output bytes on the scsi bus.
 *	The "expected" bus phase is set to the given phase, and bytes 
 *	are written.  The routine returns the number of bytes actually
 *	transferred.
 */

static int
scsiout(buf, len, phase, extended)

register caddr_t buf;
register len;
int	phase;
int	extended;		/* true if 532 byte disk sectors */
{
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register bstat;
	register len2;
	int start;

	TRACE(T_scsi, ("scsiout phase 0x%x\n", phase));
	start = len;
	ncr->init_comm |= SIC_DB;
	ncr->targ_comm = phase;
	while(!(ncr->bus_status & SBS_PHASE)) {
		if(unjam > 1) {
			return(0);
		}
		scx(scx_out_phase);
	}
	if(!(ncr->mode & SMD_DMA)) {
		ncr->mode = SMD_DMA;
		ncr->start_xmt = 0;
	}
	DELAY;
	while(len > 0) {
		while((bstat = (ncr->bus_status & (SBS_PHASE | SBS_DMA))) !=
						(SBS_PHASE | SBS_DMA)) {
			if(!(bstat & SBS_PHASE)) {
errout:
#ifdef	DEBUG
				printf
		("scsiout: phase changed from 0x%x to 0x%x with %d bytes left\n",
					phase, (ncr->curr_stat>>2) & 0x7, len);
				printscsi();
#else	DEBUG
				TRACE(T_scsi,
		("scsiout: phase changed from 0x%x to 0x%x with %d bytes left\n",
					phase, (ncr->curr_stat>>2) & 0x7, len));
#endif	DEBUG
				ncr->init_comm &= ~SIC_DB;
				ncr->mode = 0;
				return(start - len);
			}
			if(unjam > 1) {
				goto errout;
			}
		}
		*((char *) SDMA_ADDR) = *buf++;
		/* if on a 512 boundary, and an extended format device
		 * and a multiple of 512 request, output the extended bytes
		 */
		if(!(--len & (512-1))) {
		    unjam = 0;
		    if(extended && !(start & (512-1))) {
			for(len2 = 20; len2 > 0; --len2) {
			    while((bstat = 
			    (ncr->bus_status & (SBS_PHASE | SBS_DMA))) !=
				    (SBS_PHASE | SBS_DMA)) {
				if(!(bstat & SBS_PHASE) || unjam > 1)
				    goto errout;
			    }
			    *((char *) SDMA_ADDR) = 0;
			}
		    }
		}
	}
	while((ncr->bus_status & (SBS_PHASE | SBS_DMA)) == SBS_PHASE) {
		if(unjam > 1) {
			break;
		}
		scx(scx_out_exit);
	}
	ncr->mode = 0;
	ncr->init_comm &= ~SIC_DB;
	TRACE(T_scsi, ("scsiout complete\n"));
	return(start);
}


/*	scsirequest -- place a request.
 *	     This is the interface to the ncr chip.  If the chip is not
 *	busy, we act on this request.  If the chip is busy, we place the
 *	request where it will be executed later.  If there is already
 *	a request for this ID, we wait for the current request to complete.
 */

scsirequest(id, req)

struct scsireq *req;
{
	register s;
	struct scsitask *stp;

	req->sensesent = req->datasent = req->stat = req->msg = req->ret = 0;
	if(id >= NSCSI) {
		return(req->ret = SST_SEL);
	}
#ifdef	STANDALONE
	if(!isinit)
		scsiinit();
#endif	STANDALONE
	stp = &scsitask[id];
	if(stp->state != ST_IDLE)
		return(req->ret = SST_MULT);
	s = splintr();
	stp->req = req;
	stp->state = ST_CHIP;
	if(sg_state == SG_IDLE)
		scsigo();
	else 
		needed = YES;
	splx(s);
	return(req->ret);
}


/*	scsireset -- reset the SCSI bus.
 *	The bus is reset as part of the watchdog timer code.  Drivers should
 *	be prepared for their devices to be reset.
 *	Note:  The Seagate 225 will return an error on the next access
 *	following the reset.
 *	The quantum drive stays in an undefined state for several seconds
 *	after the reset.
 */

static void
scsireset()

{
	int	i, s;
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;

	if(sg_state == SG_RESET) {
		panic("SCSI reset failure");
	}
	printf("SCSI reset\n");
	if(isinit)
		s = splintr(); 
	VIA_SDMA(NO); 
	ncr->out_data = 0;
	ncr->mode = 0;
	ncr->targ_comm = 0;
	ncr->init_comm = SIC_RST;
	for(i = 0; i < 4000; ++i)
		;	/* TUNE */
	ncr->init_comm= 0;
	if(!isinit) {
		for(i = 0; i < 2700000; ++i)	/* TUNE */
			;
	}
	else {
		reset_savestate = sg_state;
		sg_state = SG_RESET;
#ifndef	STANDALONE
		timeout(finishreset, 0, 2 * v.v_hz);
#else	STANDALONE
		for(i = 0; i < 2700000; ++i)
			;
		finishreset();
#endif	STANDALONE
		splx(s);
	}
}


/*	scsiselect -- select SCSI device.
 *	    Selects the given device prior to an I/O operation.  
 *	Before a device is selected the bus should be arbitrarted via 
 *	scsiget().
 *	There are some unanswered questions regarding priorities of
 *	devices, reselects, retries, etc.
 *	XXX This code is not correct for full spec implementation
 */

int
scsiselect(id)

int	id;	/* device number (0 - 7) */
{
	register int i;
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;

	ncr->targ_comm = 0;
	ncr->out_data = 1 << id;
	ncr->init_comm = SIC_SEL | SIC_DB;
	DELAY;
	for(i = 0; !(ncr->curr_stat & SCS_BSY); ++i) {
		if(i > 100000) {	/* TUNE HERE */
#ifdef	DEBUG
			printf("scsiselect: can't select channel %d\n", id);
			printscsi();
#endif	DEBUG
			ncr->init_comm = 0;
			return(-1);
		}
	}
	ncr->init_comm = 0;
	return(0);
}

/*	scsisense -- perform sense command.
 *	     In response to a check condition, this routine may be called
 *	to determine more detailed error information.
 */

static
scsisense(id, rqp)

int	id;	/* Bus id to ask about */
struct	scsireq *rqp;	/* request block requiring sense info */
{
	struct scsig0cmd cmd;
	char	*buf;
	int	len;
	char	sbuf[35];	/* ample sized junk status buffer */
	u_char	jnk;

	TRACE(T_scsi, ("scsisense\n"));
	if(rqp->senselen && rqp->sensebuf) {
		buf = rqp->sensebuf;
		len = rqp->senselen;
	}
	else {
		buf = sbuf;
		len = sizeof(sbuf);
	}
	cmd.op = SOP_SENSE;
	cmd.addrH = (rqp->cmdbuf[1] & (0x7 << 5));	/* copy logical unit */
	cmd.len = len;
	cmd.addrM = cmd.addrL = cmd.ctl = 0;
	unjam = 0;
	if(scsiget())
		return(-1);
	if(scsiselect(id))
		return(-1);
	if(scsiout(&cmd, sizeof(cmd), SPH_CMD, 0) != sizeof(cmd)) {
		scsiabt(&jnk, &jnk);
		return(-1);
	}
	rqp->sensesent = scsiin(buf, len, SPH_DIN, 0);
	scsiabt(&jnk, &jnk);		/* use abt to guarentee shutdown */ 
	return(0);
}


/*	scsiwrap -- wrap up request processing.
 *	    The current request task is completed.  The user is notified.
 *	If there is more work to do, that work is initiated.
 *	    This routine is always called at interrupt priority.
 */

static
scsiwrap(id, stp)

register struct scsitask *stp;
{
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register struct scsireq *req;
	int	stat;
	int	s;

	TRACE(T_scsi, ("scsiwrap\n"));
	s = splcopy();
	req = stp->req;
	if(stp->state != ST_WRAP) {	/* must be an error */
		if(unjam > 1 && req->ret == SST_TIMEOUT)
			unjam = 1;
		scsiabt(&req->stat, &req->msg);
	}
	else {		/* Get completion status */
		if(scsicomplete(&req->stat, &req->msg)) {
			stat = ncr->curr_stat;	/* phase before abort */
			scsiabt(&req->stat, &req->msg);
			if(((stat >> 2) & 7) <= SPH_DIN) {
				req->ret = SST_LESS;
			}
			else {
				req->ret = SST_COMP;
			}
		}
	}
	if(req->stat & 2) {	/* do an extended sense command */
		if(scsisense(id, req)) {
			req->ret = SST_SENSE;
			goto done;
		}
		if(req->ret == 0)	/* indicate sense if no other error */
			req->ret = SST_STAT;
		
	}
done:
	SPLINTR();
	unjam = 0;
	stp->state = ST_IDLE;
	if(sg_state == SG_ACTIVE)
		sg_state = SG_IDLE;
	else if(sg_state == SG_RESET)
		sg_state = SG_CLEAR; 
#ifdef	STANDALONE
	stat = sg_state;	/* avoid recursion */
	sg_state = SG_WAIT;
#endif	STANDALONE
	TRACE(T_scsi, ("scsi req complete function 0x%x ret = 0x%x\n", 
		req->faddr, req->ret));
	if(req->faddr)		/* useful only for STANDALONE */
		(*req->faddr)(req);
#ifdef	STANDALONE
	sg_state = stat;
#endif	STANDALONE
	if(sg_state == SG_IDLE && needed)
		scsigo();
	splx(s);
	return;
}

/*	startdma -- initialize the chip for a pseudo DMA transfer.
 *	     Called to set up for a DMA transfer which will be expected to
 *	interrupt the 68020 when ready.
 */

static void
startdma(phase, isread)

{
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;

	ncr->mode = 0;
	VIA_CLRDMA();
	ncr->targ_comm = phase;
	if(isread) {
		ncr->init_comm = 0;
		ncr->mode = SMD_DMA;
		ncr->start_Ircv = 0;
	}
	else {
		ncr->init_comm = SIC_DB;
		ncr->mode = SMD_DMA;
		ncr->start_xmt = 0;
	}
}
/*	watchdog -- abort an I/O that takes too long.
 *	     If state shows we are executing our code at this instant, we
 *	set the unjam flag and wait a while longer.  If we are waiting for
 *	an interrupt back, or if the unjam flag was ineffective, then we 
 *	reset the bus.
 */

static
watchdog()

{

#ifndef	STANDALONE
	timeout(watchdog, 0, TOUTTIME * v.v_hz);
	if(unjam) {
		if(scsitask[curtask].req->flags & SRQ_NOTIME) {
			unjam = 0;
			return;
		}
		switch(sg_state) {
		case SG_ACTIVE:
			if(unjam < 3) {
				break;
			}
			/* else fall through */
		case SG_WAIT:
			if(unjam == 1)
				break;
			if(unjam == 2) {
				scsidintr();
				break;
			}
			printf("SCSI bus time out\n");
#if defined(DEBUG) || defined(HOWFAR)
			printscsi();
#endif
			scsireset();
			unjam = 0;
			break;
		}
	}
	++unjam;
#endif	STANDALONE
}


#if defined(DEBUG) || defined(HOWFAR)
static
printscsi()
{
	register struct ncr5380 *scsi;

	scsi = (struct ncr5380 *)SCSI_ADDR;
	printf("Current SCSI Data =          0x%x\n",scsi->curr_data);
	printf("Initiator Command Register = 0x%x\n",scsi->init_comm);
	printf("Mode Register =              0x%x\n",scsi->mode);
	printf("Target Command Register =    0x%x\n",scsi->targ_comm);
	printf("Current SCSI Bus Status =    0x%x\n",scsi->curr_stat);
	printf("Bus & Status Register =      0x%x\n",scsi->bus_status);
	printf("Input Data Register =        0x%x\n\n",scsi->in_data);
}
#endif defined(DEBUG) || defined(HOWFAR)
