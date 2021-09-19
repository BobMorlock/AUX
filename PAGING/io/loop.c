/*
 *	Streams loopback driver
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

#include <sys/types.h>
#include <sys/stream.h>

#ifndef NULL
#define NULL	0
#endif NULL

static int loop_open();
static int loop_close();

static int loop_open();
static int loop_close();
static int loop_wsrvc();

extern nulldev();

static struct 	module_info loop_info = { 93, "loop", 0, 256, 256, 256, NULL };
static struct	qinit looprdata = { putq, NULL, loop_open, loop_close,
			nulldev, &loop_info, NULL};
static struct	qinit loopwdata = { putq, loop_wsrvc, loop_open, loop_close, 
			nulldev, &loop_info, NULL};
struct	streamtab loopinfo = {&looprdata, &loopwdata, NULL, NULL};

static int
loop_open(q, dev, flag, sflag, err)
queue_t *q;
int *err;
{
	return(93);
}

static int
loop_close(q, flag)
queue_t *q;
{
}

static int
loop_wsrvc(q)
queue_t *q;
{
	mblk_t *m;

	while ((m = getq(q)) != NULL) {
		switch (m->b_datap->db_type) {
		case M_DATA:
			putnext(OTHERQ(q), m);
			break;
		default:
			freemsg(m);
		}
	}
}
