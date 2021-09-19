/*	@(#)%M%	UniPlus VVV.2.%I%	*/
/*
 *	This is the user interface 'driver'
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/dir.h>
#include <sys/mmu.h>
#include <sys/time.h>
#include <sys/page.h>
#include <sys/seg.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/uinter.h>
#include <sys/video.h>
#include <sys/mouse.h>
#include <sys/key.h>
#include <sys/region.h>
#include <sys/pfdat.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/debug.h>

/*
 *	This define gives the number of different screens it can
 *		support
 */

#define NDEVICES	1

extern union ptbl *segvptbl();
extern long lbolt;
static ui_lockit();
static ui_unlockit();
static ui_display();
static ui_remcursor();
static ui_remdevices();
static ui_postevent();
static ui_seter();
static ui_mouse();
static ui_keyboard();
static ui_wakeup();

extern ui_exit();
extern ui_exec();
extern ui_fork();

/*
 *	bit masks used for encoding events
 */

static short ui_mask[] = {
	0x0001,	0x0002,	0x0004,	0x0008,	0x0010,	0x0020,	0x0040,	0x0080,
	0x0100,	0x0200,	0x0400,	0x0800,	0x1000,	0x2000,	0x4000,	0x8000,
};
	

typedef int (*procedure_t)();
static procedure_t ui_vtrace[NDEVICES];		/* the old vtrace routine */
static unsigned char ui_cursor[NDEVICES];	/* sais cursor is 'on' */
static unsigned char ui_devices[NDEVICES];	/* sais devices are 'on' */
static pte_t *ui_pt[NDEVICES];			/* page table pointer for shm
						   page */
static reg_t *ui_rp[NDEVICES];			/* region pointer for shm
						   page */
static char ui_exit_work[NDEVICES];		/* there is work for the daemon
						   to do when it returns */
static char ui_dmn_waiting[NDEVICES];		/* the daemon is waiting for
						   work to do */
static char ui_exit_waiting[NDEVICES];		/* there are processes waiting 
						   tell the daemon that they
						   have exited */
static unsigned char ui_key_open[NDEVICES];	/* the key state before we */
static unsigned long ui_key_mode[NDEVICES];	/* opened it */
static unsigned long ui_key_intr[NDEVICES];

static unsigned char ui_mouse_open[NDEVICES];	/* the mouse state before we */
static unsigned long ui_mouse_mode[NDEVICES];	/* opened it */
static unsigned long ui_mouse_intr[NDEVICES];

static struct layer layer[NDEVICES][NLAYERS];	/* The layers */
static struct layer *ui_first[NDEVICES];	/* device 'active' layer list */
static struct layer *ui_last[NDEVICES];		/*        header */
static struct ui_interface *ui_addr[NDEVICES];	/* the user interface struct */
static short ui_mx[NDEVICES];			/* For calc. deltas for mouse */
static short ui_my[NDEVICES];			/*    movement */

extern procedure_t mac_exit;	/* temp hack for now ??? */

ui_open(dev)
dev_t dev;
{
	mac_exit = ui_exit;				/* ??? */
	dev = minor(dev);
	if (dev >= NDEVICES || dev >= video_count)	/* make sure the */
		return(EINVAL);				/*   'device' exists */
	if (UI_FLAG&u.u_user[1])			/* if we are already */
		return(EINVAL);				/*   connected to a */
							/*   interface, fail */
	u.u_user[1] = UI_FLAG|UI_DL(dev)|NOLAYER;	/* label our process */
							/*   as connected to */
							/*   this device */
	return(0);
}

ui_close(dev)
{
	if (ui_devices[dev])			/* turn off the mouse/keyboard*/
		ui_remdevices(dev);
	if (ui_cursor[dev])			/* turn off the cursor */
		ui_remcursor(dev);		/*    tracking */
	ui_unlockit(minor(dev));		/* unlock the locked down page*/
	u.u_user[1] = 0;			/* label the process as */
						/*    unconnected */
}

/*
 *	reading is not supported
 */

ui_read(dev,uio)
struct uio *uio;
{
	return(EINVAL);
}

/*
 *	writing is not supported
 */

ui_write(dev, uio)
struct uio *uio;
{
	return(EINVAL);
}

/*
 *	All the user interface action is done via ioctls
 */

ui_ioctl(dev, cmd, addr, arg)
register int dev;
register caddr_t addr;
{
	register struct video *vp;
	register int s, i;
	register struct ui_interface *uip;
	register struct layer *lp;

	dev = minor(dev);
	switch(cmd) {
	case UI_SET:

		/*
		 *	Set up a lineA trap handler
		 */

		if ((uip = ui_addr[dev]) == NULL ||
		    (i = UI_LAYER(u.u_user[1])) == NOLAYER)
			return(EINVAL);
		uip->c_layer[i].c_aline = * (caddr_t *) addr;
		u.u_user[0] = (long) &uip->c_layer[i].c_aline;
		return(0);

	case UI_CLEAR:

		/*
		 *	Clear a lineA trap handler
		 */

		u.u_user[0] = 0;
		return(0);

	case UI_SCREEN:

		/*
		 *	map in the screen associated with a 'device' to the
		 *		address space requested by the process (this
		 *		must be on a segment boundary)
		 */

		dophys(3, *(unsigned *)addr, 512*1024,
			video_desc[dev]->video_base);
		return(u.u_error);

	case UI_UNSCREEN:

		/*
		 *	unmap the screen
		 */

		dophys(3, 0, 0, 0);
		return(u.u_error);

	case UI_ROM:

		/*
		 *	map in the ROM to the address space requested by the
		 *		process (this must be on a segment boundary)
		 */

		dophys(2, *(unsigned *)addr, 4*512*1024/8, 0x40000000);
		return(u.u_error);

	case UI_UNROM:

		/*
		 *	unmap the ROM
		 */

		dophys(2, 0, 0, 0);
		return(u.u_error);

	case UI_MAP:

		/*
		 *	map in and lock down a page for the user interface
		 *		structure ... the address given must be a page
		 *		in a shared memory segment, and it must be on
		 *		a page boundary
		 */

		ASSERT(sizeof(*uip) <= ptob(1));
		return(ui_lockit(dev, *(caddr_t *)addr));

	case UI_UNMAP:

		/*
		 *	unmap a shared memory page. Also undo any other ioctls
		 *		that have started actions (such as cursors or
		 *		keyboard events etc) that depend on this area.
		 *		rpd -- need to kill any u.u_user[0]'s that point
		 *		into this guy.
		 */

		if (ui_devices[dev]) 
			ui_remdevices(dev);
		if (ui_cursor[dev]) 
			ui_remcursor(dev);
		return(ui_unlockit(dev));

	case UI_CURSOR:

		/*
		 *	display a mouse cursor and enable cursor tracking
		 */

		if (ui_cursor[dev] || (uip = ui_addr[dev]) == NULL)
			return(EINVAL);
		
		uip->c_mx = 0;			/* set the cursor to (0,0) */
		uip->c_my = 0;
		ui_mx[dev] = mouse_x[dev];	/* clear the deltas */
		ui_my[dev] = mouse_y[dev];
		ui_cursor[dev] = 1;		/* mark it as on */
		vp = video_desc[dev];
		s = spl1();			/* turn on interrupts */
		ui_vtrace[dev] = vp->video_intr;
		vp->video_intr = ui_display;
		(*vp->video_func)(vp, VF_ENABLE, 0);
		ui_display(vp);			/* display it */
		splx(s);
		return(0);

	case UI_UNCURSOR:

		/*
		 *	Stop the display of a cursor (this doesn't remove
		 *		the cursor from the screen, it just
		 *		stops its being updated)
		 */

		if (!ui_cursor[dev])
			return(EINVAL);
		ui_remcursor(dev);
		return(0);

	case UI_DEVICES:

		/*
		 *	This connects the devices to the mouse and keyboard
		 *		so that mouse and keyboard events can be
		 *		posted.
		 */

		if (ui_devices[dev] || ui_addr[dev] == NULL)
			return(EINVAL);
		ui_devices[dev] = 1;
		if (ui_mouse_open[dev] = mouse_op(dev, MOUSE_OP_OPEN, 0)) {
			ui_mouse_intr[dev] = mouse_op(dev, MOUSE_OP_INTR,
							ui_mouse);
			ui_mouse_mode[dev] = mouse_op(dev, MOUSE_OP_MODE, 1);
		} else {
			if (mouse_open(dev, ui_mouse, 1) == 0)
				return(EINVAL);
		}
		if (ui_key_open[dev] = key_op(dev, KEY_OP_OPEN, 0)) {
			ui_key_intr[dev] = key_op(dev, KEY_OP_INTR,ui_keyboard);
			ui_key_mode[dev] = key_op(dev, KEY_OP_MODE, KEY_ARAW);
		} else {
			if (key_open(dev, ui_keyboard, KEY_ARAW) == 0)
				return(EINVAL);
		}
		return(0);

	case UI_UNDEVICES:

		/*
		 *	This restores the mouse and keyboard to their previous
		 *		owners (or closes them if they were not
		 *		previously in use)
		 */

		if (!ui_devices[dev])
			return(EINVAL);
		ui_remdevices(dev);
		return(0);


	case UI_DELAY:

		/*
		 *	delay waits n ticks and then returns the current
		 *		tickcount
		 */

		delay(*(long *)addr);
		/* fall through */

	case UI_TICKCOUNT:

		/*
		 *	this returns the current tickcount
		 */

		*(long *)addr = lbolt;
		return(0);

	case UI_DAEMON:

		/*
		 *	This is the call the watchdog daemon makes to find
		 *		out if any processes have died (in which case
		 *		the processes pid is left in the uip structure)
		 *		or any layers have completed (in which case the
		 *		layer is marked LS_DONE in the uip structure)
		 */

		while (!ui_exit_work[dev]) {
			if (ui_exit_waiting[dev]) {
				ui_exit_waiting[dev] = 0;
				wakeup(&ui_exit_waiting[dev]);
			}
			ui_dmn_waiting[dev] = 1;
			if (sleep(&ui_dmn_waiting[dev], (PZERO+1)|PCATCH)) {
				ui_dmn_waiting[dev] = 0;
				return(EINTR);
			}
		}
		ui_exit_work[dev] = 0;
		return(0);

	case UI_POSTEVENT:

		/*
		 *	This posts an event to the layer associated with the
		 *		current process 
		 */

		if ((uip = ui_addr[dev]) == NULL ||
		    (i = UI_LAYER(u.u_user[1])) == NOLAYER)
			return(EINVAL);
		s = spl1();
		u.u_rval1 = ui_postevent(u.u_user[1],
			((struct postevent*)addr)->eventCode,
			((struct postevent*)addr)->eventMsg);
		splx(s);
		return(0);

	case UI_LPOSTEVENT:

		/*
		 *	This posts an event to the layer identified  by the
		 *		field 'layer' in the parameter
		 */

		if ((uip = ui_addr[dev]) == NULL ||
		    (i = ((struct lpostevent*)addr)->layer) < 0 ||
		    i >= NLAYERS ||
		    uip->c_layer[i].c_state != LS_INUSE)
			return(EINVAL);
		s = spl1();
		u.u_rval1 = ui_postevent(UI_DL(dev)|i,
			((struct lpostevent*)addr)->eventCode,
			((struct lpostevent*)addr)->eventMsg);
		splx(s);
		return(0);

	case UI_FLUSHEVENTS:

		/*
		 *	This flushes events from the current processes
		 *		layer's queue
		 */

		if ((uip = ui_addr[dev]) == NULL ||
		    (i = UI_LAYER(u.u_user[1])) == NOLAYER)
			return(EINVAL);
		{
			register short eventMask, stopMask;
			register struct event *ep, *last, *tp;

			eventMask = ((struct flushevents *)addr)->eventMask;
			stopMask = ((struct flushevents *)addr)->stopMask;
			lp = &layer[dev][i];
			s = spl1();
			ep = lp->l_first;
			last = NULL;
			for (;;) {
				if (ep == NULL) {
					lp->l_last = last;
					break;
				}
				i = ui_mask[ep->event.what];
				if (i&stopMask)
					break;
				if (i&eventMask) {
					if (last == NULL) {
						lp->l_first = tp = ep->next;
					} else {	
						last->next = tp = ep->next;
					}
					ep->next = lp->l_free;
					lp->l_free = ep;
					ep = tp;
				} else {
					last = ep;
					ep = ep->next;
				}
			}
			splx(s);
		}
		return(0);

	case UI_GETOSEVENT:

		/*
		 *	return the next event available from the event queue
		 */

		if ((i = UI_LAYER(u.u_user[1])) == NOLAYER)
			return(EINVAL);
		{
			struct getosevent *ge = (struct getosevent *) addr;

			return(ui_getevent(UI_DL(dev)|i,
				ge->eventMask,
				ge->blocking,
				&ge->theEvent));
		}

	case UI_SETEVENTMASK:

		/*
		 *	set the current layer's event mask
		 */

		if ((uip = ui_addr[dev]) == NULL ||
		    (i = UI_LAYER(u.u_user[1])) == NOLAYER)
			return(EINVAL);
		layer[dev][i].l_mask = *(short *)addr; 
		return(0);
		
	case UI_CREATELAYER:

		/*
		 *	Create a new layer on the current device, if one is 
		 *		not available return EAGAIN.
		 */

		if ((uip = ui_addr[dev]) == NULL ||
		    (i = UI_LAYER(u.u_user[1])) != NOLAYER)
			return(EINVAL);
		for (i = 0; i < NLAYERS; i++) {
			if (uip->c_layer[i].c_state == LS_EMPTY) {
				u.u_rval1 = i;
				u.u_user[1] = UI_FLAG|UI_DL(dev)|i;
				lp = &layer[dev][i];
				s = spl1();
				uip->c_layer[i].c_state = LS_INUSE;
				lp->l_sleep = 0;
				lp->l_select = 0;
				lp->l_down = 0;
				lp->l_ref = 1;
				lp->l_first = NULL;
				lp->l_last = NULL;
				lp->l_free = NULL;
				for (i = 0; i < NEVENTS; i++) {
					lp->l_events[i].next = lp->l_free;
					lp->l_free = &lp->l_events[i];
				}
				lp->l_mask = everyEvent&~keyUpMask;
				if (ui_first[dev]) {
					lp->l_next = NULL;
					lp->l_prev = ui_last[dev];
					lp->l_prev->l_next = lp;
					ui_last[dev] = lp;
				} else {
					lp->l_prev = lp->l_next = NULL;
					ui_first[dev] = ui_last[dev] = lp;
				}
				splx(s);
				return(0);
			}
		}
		return(EAGAIN);

	case UI_DETACHLAYER:

		/*
		 *	Disconnect the current process from its current layer
		 *		if no other process is connected to that layer, 
		 *		destroy it.
		 */

		if ((uip = ui_addr[dev]) == NULL ||
		    (i = UI_LAYER(u.u_user[1])) == NOLAYER)
			return(EINVAL);
		if (--(layer[dev][i].l_ref) == 0) {
			lp = &layer[dev][i];
			if (lp->l_next) {
				lp->l_next->l_prev = lp->l_prev;
			} else {
				ui_last[dev] = lp->l_prev;
			}
			if (lp->l_prev) {
				lp->l_prev->l_next = lp->l_next;
			} else {
				ui_first[dev] = lp->l_next;
			}
			if (i == uip->c_active) {
				if (ui_first[dev]) {
					uip->c_active = ui_first[dev] -
							&layer[dev][i];
				} else {
					uip->c_active = NOLAYER;
				}
			}
			uip->c_layer[i].c_state = LS_DONE;
			ui_exit_work[dev] = 1;
			if (ui_dmn_waiting[dev]) {
				ui_dmn_waiting[dev] = 0;
				wakeup(&ui_dmn_waiting[dev]);
			}
		}
		u.u_user[1] = UI_FLAG|UI_DL(dev)|NOLAYER;
		return(0);

	case UI_ATTACHLAYER:

		/*
		 *	Attach the current process to a named layer
		 */

		if ((uip = ui_addr[dev]) == NULL ||
		    UI_LAYER(u.u_user[1]) != NOLAYER ||
		    (i = *(int *)addr) < 0 ||
		    i >= NLAYERS ||
		    uip->c_layer[i].c_state != LS_INUSE)
			return(EINVAL);
		layer[dev][i].l_ref++;
		u.u_user[1] = UI_FLAG|UI_DL(dev)|i;
		return(0);
		
	case UI_SETLAYER:

		/*
		 *	Set a named layer to be the current active one
		 *		for receiving new events
		 */

		i = *(int *)addr;
		uip = ui_addr[dev];
		if (i < 0 || i >= NLAYERS || uip == NULL ||
		    uip->c_layer[i].c_state != LS_INUSE)
			return(EINVAL);
		if (i != uip->c_active) {
			lp = &layer[dev][i];
			if (lp->l_next) {
				lp->l_next->l_prev = lp->l_prev;
			} else {
				ui_last[dev] = lp->l_prev;
			}
			if (lp->l_prev) {
				lp->l_prev->l_next = lp->l_next;
			} else {
				ui_first[dev] = lp->l_next;
			}
			lp->l_prev = NULL;
			if (lp->l_next = ui_first[dev]) {
				lp->l_next->l_prev = lp;
			} else {
				ui_last[dev] = lp;
			}
			ui_first[dev] = lp;
			uip->c_active = i;
		}
		return(0);
		
	default:
		return(EINVAL);
	}
}

/*
 *	This routine is called when a fork(2) occurs, it bumps the reference
 *		count on the processes layer (if there is one).
 */

ui_fork()
{
	register struct ui_interface *uip;
	register int l;

	if (!(UI_FLAG&u.u_user[1]))
		return;
	if((l = UI_LAYER(u.u_user[1])) != NOLAYER)
		layer[UI_DEVICE(u.u_user[1])][l].l_ref++;
}

/*
 *	This routine is called when an exec occurs ... layers are not
 *		inherited across exec(2)s
 */

ui_exec()
{
	ui_exitp(1);
}

/*
 *	This routine is called whenever a process exit(2)s, it disconnects
 *		the process from the layer, if no other processes are
 *		connected to the layer it frees it (and wakes the daemon)
 *		we also wake the daemon to tell us the pid of the process
 *		that is going (in case it holds an locks). If more than one
 *		process exits at the same time they may wait
 */

ui_exit()
{
	ui_exitp(1);
}

ui_exitp(wait)
{
	register int dev, l;
	register struct ui_interface *uip;
	register struct layer *lp;

	if (!(UI_FLAG&u.u_user[1]))
		return;
	dev = UI_DEVICE(u.u_user[1]);
	if (wait) {
		if (ui_exit_work[dev] == 0 && ui_dmn_waiting[dev] == 0) {
			ui_exit_work[dev] = 1;
		} else {
			while (!ui_dmn_waiting[dev]) {
				ui_exit_waiting[dev] = 1;
				sleep(&ui_exit_waiting[dev], PZERO);
			}
			ui_dmn_waiting[dev] = 0;
			wakeup(&ui_dmn_waiting[dev]);
		}
	}
	uip = ui_addr[dev];
	uip->c_exitpid = u.u_procp->p_pid;
	if ((l = UI_LAYER(u.u_user[1])) != NOLAYER) {
		if (--(layer[dev][l].l_ref) == 0) {
			lp = &layer[dev][l];
			if (lp->l_next) {
				lp->l_next->l_prev = lp->l_prev;
			} else {
				ui_last[dev] = lp->l_prev;
			}
			if (lp->l_prev) {
				lp->l_prev->l_next = lp->l_next;
			} else {
				ui_first[dev] = lp->l_next;
			}
			if (l == uip->c_active) {
				if (ui_first[dev]) {
					uip->c_active = ui_first[dev] -
							&layer[dev][l];
				} else {
					uip->c_active = NOLAYER;
				}
			}
			uip->c_layer[l].c_state = LS_DONE;
			ui_exit_work[dev] = 1;
			if (ui_dmn_waiting[dev]) {
				ui_dmn_waiting[dev] = 0;
				wakeup(&ui_dmn_waiting[dev]);
			}
		}
	}
}

/*
 *	This routine posts events to the device and layer named by 'code'.
 *	Special processing is done for key down so that autokey events are
 *	delivered to processes.
 *	Update events are not queued but flagged instead
 */

static
ui_postevent(code, what, message)
int code;
{
	register struct ui_interface *uip;
	register struct layer *lp, *elp;
	register struct event *ep;
	register dev, i, l;

	if ((uip = ui_addr[dev = UI_DEVICE(code)]) == NULL)
		return(0);
	l = UI_LAYER(code);
	if (l == NOLAYER)
		return(0);
	lp = &layer[dev][l];
	switch (what) {
	case keyUp:
		message <<= 8;
		break;

	case keyDown:
		if (lp->l_down)
			untimeout(ui_wakeup, lp);
		else
			lp->l_down = 1;
		lp->l_time = lbolt+uip->c_keythres;
		lp->l_char = message;
		message <<= 8;
		break;

	case updateEvt:
		lp->l_update = 1;
		goto out;

	} 
	if (!(lp->l_mask&ui_mask[what]))
		return(evtNotEnb);
	if (lp->l_free == NULL) {
		ep = lp->l_first;
		lp->l_first = ep->next;
	} else {
		ep = lp->l_free;
		lp->l_free = ep->next;
	}
	ui_seter(&ep->event,uip,what,message);
	ep->next = NULL;
	if (lp->l_first == NULL) {
		lp->l_first = ep;
	} else {
		lp->l_last->next = ep;
	}
	lp->l_last = ep;
    out:
	if (lp->l_sleep&ui_mask[what]) {
		lp->l_sleep = 0;
		wakeup(lp);
	}
	if (lp->l_select) {
		selwakeup(lp->l_select,0);
		lp->l_select = 0;
	}
	return(0);
}

/*
 *	This returns events from a layer
 *		update events are returned
 *			with out any data (only one of
 *			these is 'queued' at any one time)
 *		the keyUp/keyDown events do not return an
 *			encoded character value in the low byte
 *			of the message, only a key number
 *			in the next byte.
 *		the blocking parameter can be one of
 *			NOBLOCK - return if no
 *				  events are present
 *			BLOCK   - block until an event is
 *				  is available
 *			MBLOCK	- block until an event is
 *				  available or the mouse moves
 *			AVAIL	- return if no events are present.
 *				  also, don't remove event from queue.
 */

static
ui_getevent(code,mask,blocking,erp)
int code,mask,blocking;
EventRecord *erp;
{
	register struct ui_interface *uip;
	register struct layer *lp;
	int dev,l,s;

	if ((uip = ui_addr[dev = UI_DEVICE(code)]) == NULL)
		return(0);
	if ((l = UI_LAYER(u.u_user[1])) == NOLAYER)
		return(EINVAL);
	{
		register struct event *ep, *last;

		lp = &layer[dev][l];
		lp->l_mouse = 0;
		for(;;) {
		    last = NULL;
		    s = spl1();
		    for (ep = lp->l_first; ep != NULL; ep = ep->next) {
			if (ui_mask[ep->event.what]&mask) {
			    *erp = ep->event;
			    if (blocking != AVAIL) {
				if (last == NULL) {
				    lp->l_first = ep->next;
				} else {
				    last->next = ep->next;
				}
				if (ep->next == NULL)
				    lp->l_last = last;
				ep->next = lp->l_free;
				lp->l_free = ep;
			    }
			    splx(s);
			    return(0);
			}
			last = ep;
		    }
		    if (lp->l_down &&
			lp->l_time <= lbolt &&
			mask&autoKeyMask) {
			    if (blocking != AVAIL)
				lp->l_time = lbolt + uip->c_keyrate;
			    ui_seter(erp,uip,autoKey,lp->l_char<<8);
			    splx(s);
			    return(0);
		    }
		    if (lp->l_update && mask&updateMask) {
			    if (blocking != AVAIL)
				lp->l_update = 0;
			    ui_seter(erp,uip,updateEvt,0);
			    splx(s);
			    return(0);
		    }
		    switch(blocking) {
		    case NOBLOCK:
		    case AVAIL:
		    null:
			ui_seter(erp,uip,nullEvent,0);
			splx(s);
			return(0);
		    case MBLOCK:
			if (lp->l_mouse) {
				lp->l_mouse = 0;
				goto null;
			}
			lp->l_sleep |= mouseMoveMask;
		    case BLOCK:
			if (lp->l_down &&
			    !(lp->l_sleep&autoKeyMask) &&
			    mask&autoKeyMask)
				timeout(ui_wakeup, lp,
					lbolt - lp->l_time);
			lp->l_sleep |= mask;
			(void)sleep(lp, PZERO+1);
			break;
		    }
		}
	}
}

/*
 *	This routine fills in the fields of an event record.  The what and
 *	message parameters are taken as arguments.  The other fields are
 *	taken from the current state of the machine.
 */

static
ui_seter(erp,uip,what,message)
EventRecord *erp;
struct ui_interface *uip;
{
	erp->what = what;
	erp->message = message;
	erp->when = lbolt;
	erp->where.x = uip->c_mx;
	erp->where.y = uip->c_my;
	erp->modifiers = uip->c_modifiers;
}

/*
 *	This routine is called from the keyboard driver in ARAW (ascii raw) mode
 *		id 	device
 *		cmd 	(ignored)
 *		c	character code (bit 7 on means key up)
 *		next	(ignored)
 *	Basicly we locate the current layer and post the appropriate event. If
 *	a key up occurs, cancel any autokey events on any layers that have 
 *	timeouts waiting for them.
 */

static
ui_keyboard(id, cmd, c, next)
int id, cmd, c, next;
{
	register struct ui_interface *uip;
	register int i, l;
	register struct layer *lp;

	if ((uip = ui_addr[id]) == NULL)
		return;
	l = uip->c_active;
	if (c&0x80) {
		c &= 0x7f;
		i = uip->c_key[c];
		if (i != NOLAYER &&
		    uip->c_layer[i].c_state == LS_INUSE &&
		    ((lp = &layer[id][i])->l_down) &&
		    lp->l_char == c) {
			untimeout(ui_wakeup, lp);
			lp->l_down = 0;
		}
		uip->c_key[c] = NOLAYER;
		switch(c) {
		case 0x36:
			uip->c_modifiers &= ~cntlKey;
			break;
		case 0x37:
			uip->c_modifiers &= ~cmdKey;
			break;
		case 0x38:
			uip->c_modifiers &= ~shiftKey;
			break;
		case 0x39:
			uip->c_modifiers &= ~alphaLock;
			break;
		case 0x3a:
			uip->c_modifiers &= ~optionKey;
			break;
		default:
			if (l != NOLAYER)
				ui_postevent(l|UI_DL(id), keyUp, c);
		}
	} else {
		switch(c) {
		case 0x36:
			uip->c_modifiers |= cntlKey;
			break;
		case 0x37:
			uip->c_modifiers |= cmdKey;
			break;
		case 0x38:
			uip->c_modifiers |= shiftKey;
			break;
		case 0x39:
			uip->c_modifiers |= alphaLock;
			break;
		case 0x3a:
			uip->c_modifiers |= optionKey;
			break;
		default:
			if (l != NOLAYER) {
				uip->c_key[c] = l;
				ui_postevent(l|UI_DL(id), keyDown, c & 0x7f);
			}
		}
	}
}

/*
 *	Mouse events are passed in to the appropriate layer. If the layer is
 *	in mouse blocking mode and the mouse has moved wake it.
 *		id	device
 *		cmd	(ignore)
 *		b	raw mouse data from the device
 *		change	bit 0 -> button change
 *			bit 1 -> displacement change
 */

static
ui_mouse(id, cmd, b, change)
int id, cmd, b, change;
{
	struct ui_interface *uip;
	struct layer *lp;

	if ((uip = ui_addr[id]) == NULL)
		return;
	uip->c_button = mouse_button[id];
	b = uip->c_active;
	if (b == NOLAYER)
		return;
	lp = &layer[id][b];
	if (change&2 && lp->l_sleep&mouseMoveMask) {
		wakeup(lp);
		lp->l_mouse = 1;
		if (lp->l_down) {
			untimeout(ui_wakeup, lp);
			lp->l_down = 0;
		}
	}
	b |= UI_DL(id);
	if (change&1) {
		if (uip->c_button) {
			uip->c_modifiers &= ~btnState;
			ui_postevent(b, mouseDown, 0);
		} else {
			uip->c_modifiers |= btnState;
			ui_postevent(b, mouseUp, 0);
		}
	}
}

/*
 *	Page in and lock down a page in a shared memory section
 *		in a user's address space. Max of 1 page per 'device'
 */

static
ui_lockit(dev, base)
int dev;
caddr_t base;
{
	register reg_t *rp;
	register pte_t *pt;
	register struct ui_interface *uip;
	register struct video *vp;
	int x;

	if (ui_rp[dev] || poff(base))
		return(EINVAL);
	rp = findreg(u.u_procp, (caddr_t)base);
	rp->r_flags |= RG_NOSWAP;
	regrele(rp);
	if (rp->r_type != RT_SHMEM)
		goto out;
#ifndef lint
	pt = (pte_t *) segvptbl(&u.u_stbl[snum(base)]) + pnum(base);
#else
	pt = (pte_t *)0;
#endif
	x = fubyte((caddr_t)base);
	if(x == -1)
		goto out;
	if(subyte((caddr_t)base, x) == -1)
		goto out;
	memlock();
	pg_setlock(pt);
	pfdat[pftopfi(pt->pgm[0].pg_pfn)].pf_rawcnt++;
	memunlock();
	reglock(rp);
	rp->r_flags &= ~RG_NOSWAP;
	rp->r_refcnt++;
	regrele(rp);
	ui_rp[dev] = rp;
	ui_pt[dev] = pt;
	uip = (struct ui_interface *)ptob(pt->pgm[0].pg_pfn);
	uip->c_button = mouse_button[dev];
	uip->c_mx = 0;
	uip->c_my = 0;
	uip->c_cx = 0;
	uip->c_cy = 0;
	vp = video_desc[dev];
	uip->c_ssx = vp->video_scr_x;
	uip->c_ssy = vp->video_scr_y;
	uip->c_smx = vp->video_mem_x;
	uip->c_smy = vp->video_mem_y;
	ui_addr[dev] = uip;
	return(0);
out:
	reglock(rp);
	rp->r_flags &= ~RG_NOSWAP;
	regrele(rp);
	return(EINVAL);
}

/*
 *	Unlock the locked down page
 */

static
ui_unlockit(dev)
int dev;
{
	register reg_t *rp;
	register pte_t *pt;

	if (ui_rp[dev] == 0)
		return(EINVAL);
	ui_addr[dev] = (struct ui_interface *)0;
	rp = ui_rp[dev];
	reglock(rp);
	rp->r_flags |= RG_NOSWAP;
	regrele(rp);
	pt = ui_pt[dev];
	memlock();
	if (--pfdat[pftopfi(pt->pgm[0].pg_pfn)].pf_rawcnt == 0)
		pg_clrlock(pt);
	memunlock();
	reglock(rp);
	rp->r_flags &= ~RG_NOSWAP;
	rp->r_refcnt--;
	if (rp->r_refcnt == 0) {	/* if this is the last reference */
		freereg(rp);		/* remove it */
	} else {
		regrele(rp);
	}
	ui_rp[dev] = 0;
	ui_exitp(0);
	u.u_user[1] = UI_FLAG|UI_DL(dev)|NOLAYER;
	return(0);
}

/*
 *	Called from the vertical retrace interrupt
 *	Update the current mouse position
 *	If required redraw the cursor
 *		1)	See if we need to (or are allowed to)
 *		2)	write back the old data
 *		3)	save the new data
 *		4)	write the cursor
 *		5)	update the cursor position
 */

static
ui_display(vp)
struct video *vp;
{
	register struct ui_interface *uip;
	register int dev;
	register short i, x, y, mx, my;
	register unsigned short *sp;

	dev = vp->video_off;
	if (ui_vtrace[dev])
		(*ui_vtrace[dev])(vp);
	if ((uip = ui_addr[dev]) == NULL)
		return;
	if ((x = mouse_x[dev] - ui_mx[dev]) < 0) {/* calc the movement deltas */
		mx = -x;
	} else {
		mx = x;
	}
	if ((y = mouse_y[dev] - ui_my[dev]) < 0) {
		my = -y;
	} else {
		my = y;
	}
	ui_mx[dev] = mouse_x[dev];
	ui_my[dev] = mouse_y[dev];
	if (mx > my) {				/* calc. movement dist. */
		i = mx + (my>>1);
	} else {
		i = my + (mx>>1);
	}
	if (i >= 256)
		i = 255;
	sp = &uip->c_mlookup[0];		/* <- do mouse acc. */
	if (*sp) {
		mx = x;
		my = y;
		while (i > *sp) {
			x += mx;
			y += my;
			sp++;
		}
	}
	if (x < 0) {				/* update the mouse position */
		if ((-x) > uip->c_mx) {
			uip->c_mx = 0;
		} else {
			uip->c_mx += x;
		}
	} else {
		uip->c_mx += x;
		if (uip->c_mx >= uip->c_ssx) {
			uip->c_mx = uip->c_ssx - 1;
		}
	}
	if (y < 0) {
		if ((-y) > uip->c_my) {
			uip->c_my = 0;
		} else {
			uip->c_my += y;
		}
	} else {
		uip->c_my += y;
		if (uip->c_my >= uip->c_ssy) {
			uip->c_my = uip->c_ssy - 1;
		}
	}

	/*
  	 *	decide if the cursor must be redrawn
	 */

	if (!uip->c_draw) {
		if(uip->c_lock || !uip->c_visible ||
		 	(uip->c_mx == uip->c_cx && uip->c_my == uip->c_cy))
		return;
	} else {
		uip->c_draw = 0;
	}

	/*
	 *	draw the cursor 
	 */

	switch (uip->c_style) {
	case CUR_SMALL1:
		ui_small1(vp, uip);
		break;
	case CUR_SMALL2:
		ui_small2(vp, uip);
		break;
	case CUR_SMALL4:
		ui_small4(vp, uip);
		break;
	case CUR_SMALL8:
		ui_small8(vp, uip);
		break;
	}

	/*
	 *	update the cursor position
	 */

	uip->c_cx = uip->c_mx;
	uip->c_cy = uip->c_my;
}

/*
 *	remove the cursor display routine from the VTRACE interrupt
 */

static
ui_remcursor(dev)
register int dev;
{
	register int s;
	register struct video *vp;

	ui_cursor[dev] = 0;
	vp = video_desc[dev];
	s = spl1();
	vp->video_intr = ui_vtrace[dev];
	if (vp->video_intr == NULL) 
		(*vp->video_func)(vp, VF_DISABLE, 0);
	splx(s);
}

/*
 *	restore owership of the mouse/keyboard to their previous users
 *		(or close them if they were previously free)
 */

static
ui_remdevices(dev)
register int dev;
{
	register int s;

	ui_devices[dev] = 0;
	s = spl1();
	if (ui_mouse_open[dev]) {
		mouse_op(dev, MOUSE_OP_MODE, ui_mouse_mode[dev]);
		mouse_op(dev, MOUSE_OP_INTR, ui_mouse_intr[dev]);
	} else {
		mouse_close(dev);
	}
	if (ui_key_open[dev]) {
		key_op(dev, KEY_OP_MODE, ui_key_mode[dev]);
		key_op(dev, KEY_OP_INTR, ui_key_intr[dev]);
	} else {
		key_close(dev);
	}
	splx(s);
}

/*
 *	This is the routine that is used to wake up autokey
 *		events
 */

static
ui_wakeup(lp)
struct layer *lp;
{
	wakeup(lp);
	lp->l_sleep = 0;
	if (lp->l_select) {
		selwakeup(lp->l_select,0);
		lp->l_select = 0;
	}
}

ui_select(dev,rw)
dev_t dev;
int rw;
{ 
	int i;
	EventRecord er;
	struct layer *lp;

	dev = minor(dev);
	if ((i = UI_LAYER(u.u_user[1])) == NOLAYER)
		return(0);
	ui_getevent(UI_DL(dev)|i,everyEvent,AVAIL,&er);
	if (er.what != nullEvent)
		return 1;
	lp = &layer[dev][i];
	if (lp->l_down && !(lp->l_sleep&autoKeyMask) &&
	    lp->l_mask&autoKeyMask) {
		lp->l_sleep |= autoKeyMask;
		timeout(ui_wakeup, lp, lbolt - lp->l_time);
	}
	lp->l_select = u.u_procp;
	return 0;
}

