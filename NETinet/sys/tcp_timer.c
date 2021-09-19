/*	@(#)tcp_timer.c 1.1 86/02/03 SMI; from UCB 4.32 83/05/27	*/
/*	@(#)tcp_timer.c	2.1 86/04/15 NFSSRC */

#include "sys/param.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/time.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/page.h"
#endif PAGING
#include "sys/systm.h"
#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/protosw.h"

#include "net/route.h"
#include "net/if.h"

#include "netinet/in.h"
#include "netinet/in_pcb.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/ip_var.h"
#include "netinet/tcp.h"
#include "netinet/tcp_fsm.h"
#include "netinet/tcp_seq.h"
#include "netinet/tcp_timer.h"
#include "netinet/tcp_var.h"
#include "netinet/tcpip.h"

int	tcpnodelack = 0;
/*
 * Fast timeout routine for processing delayed acks
 */
tcp_fasttimo()
{
	register struct inpcb *inp;
	register struct tcpcb *tp;
	int s = splnet();

	inp = tcb.inp_next;
	if (inp)
	for (; inp != &tcb; inp = inp->inp_next)
		if ((tp = (struct tcpcb *)inp->inp_ppcb) &&
		    (tp->t_flags & TF_DELACK)) {
			tp->t_flags &= ~TF_DELACK;
			tp->t_flags |= TF_ACKNOW;
			(void) tcp_output(tp);
		}
	(void) splx(s);
}

/*
 * Tcp protocol timeout routine called every 500 ms.
 * Updates the timers in all active tcb's and
 * causes finite state machine actions if timers expire.
 */
tcp_slowtimo()
{
	register struct inpcb *ip, *ipnxt;
	register struct tcpcb *tp;
	int s = splnet();
	register int i;

	/*
	 * Search through tcb's and update active timers.
	 */
	ip = tcb.inp_next;
	if (ip == 0) {
		(void) splx(s);
		return;
	}
	while (ip != &tcb) {
		tp = intotcpcb(ip);
		if (tp == 0)
			continue;
		ipnxt = ip->inp_next;
		for (i = 0; i < TCPT_NTIMERS; i++) {
			if (tp->t_timer[i] && --tp->t_timer[i] == 0) {
				(void) tcp_usrreq(tp->t_inpcb->inp_socket,
				    PRU_SLOWTIMO, (struct mbuf *)0,
				    (struct mbuf *)i, (struct mbuf *)0);
				if (ipnxt->inp_prev != ip)
					goto tpgone;
			}
		}
		tp->t_idle++;
		if (tp->t_rtt)
			tp->t_rtt++;
tpgone:
		ip = ipnxt;
	}
	tcp_iss += TCP_ISSINCR/PR_SLOWHZ;		/* increment iss */
	(void) splx(s);
}

/*
 * Cancel all timers for TCP tp.
 */
tcp_canceltimers(tp)
	struct tcpcb *tp;
{
	register int i;

	for (i = 0; i < TCPT_NTIMERS; i++)
		tp->t_timer[i] = 0;
}

long	tcp_backoff[TCP_MAXRXTSHIFT] = {
#ifdef vax
/* vax C compiler won't convert float to int for initialization */
	FSCALE, 12*FSCALE/10, 14*FSCALE/10, 17*FSCALE/10, 2*FSCALE,
	3*FSCALE, 5*FSCALE, 8*FSCALE, 16*FSCALE, 32*FSCALE
#else
	1.0*FSCALE, 1.2*FSCALE, 1.4*FSCALE, 1.7*FSCALE, 2.0*FSCALE,
	3.0*FSCALE, 5.0*FSCALE, 8.0*FSCALE, 16.0*FSCALE, 32.0*FSCALE
#endif
};
int	tcpexprexmtbackoff = 0;
/*
 * TCP timer processing.
 */
struct tcpcb *
tcp_timers(tp, timer)
	register struct tcpcb *tp;
	int timer;
{

	switch (timer) {

	/*
	 * 2 MSL timeout in shutdown went off.  Delete connection
	 * control block.
	 */
	case TCPT_2MSL:
		tp = tcp_close(tp);
		break;

	/*
	 * Retransmission timer went off.  Message has not
	 * been acked within retransmit interval.  Back off
	 * to a longer retransmit interval and retransmit all
	 * unacknowledged messages in the window.
	 */
	case TCPT_REXMT:
		tp->t_rxtshift++;
		if (tp->t_rxtshift > TCP_MAXRXTSHIFT) {
			tp = tcp_drop(tp, ETIMEDOUT);
			break;
		}
		TCPT_RANGESET(tp->t_timer[TCPT_REXMT],
		    tp->t_srtt>>FSHIFT, TCPTV_MIN, TCPTV_MAX);
		if (tcpexprexmtbackoff) {
			TCPT_RANGESET(tp->t_timer[TCPT_REXMT],
			    tp->t_timer[TCPT_REXMT] << tp->t_rxtshift,
			    TCPTV_MIN, TCPTV_MAX);
		} else {
			TCPT_RANGESET(tp->t_timer[TCPT_REXMT],
			    (tp->t_timer[TCPT_REXMT] *
			        tcp_backoff[tp->t_rxtshift - 1]) >> FSHIFT,
			    TCPTV_MIN, TCPTV_MAX);
		}
		tp->snd_nxt = tp->snd_una;
		/* this only transmits one segment! */
		(void) tcp_output(tp);
		break;

	/*
	 * Persistance timer into zero window.
	 * Force a byte to be output, if possible.
	 */
	case TCPT_PERSIST:
		/*
		 * If we have persisted enough times in LAST_ACK give up.
		 * This is to fix the other end of the FIN_WAIT_2 bug.
		 */
		if( tcp_setpersist(tp) && (tp->t_state == TCPS_LAST_ACK)) {
			tp = tcp_drop(tp, ETIMEDOUT);
			break;
		}
		tp->t_force = 1;
		(void) tcp_output(tp);
		tp->t_force = 0;
		break;

	/*
	 * Keep-alive timer went off; send something
	 * or drop connection if idle for too long.
	 */
	case TCPT_KEEP:
		if (tp->t_state < TCPS_ESTABLISHED)
			goto dropit;
		if (tp->t_state == TCPS_FIN_WAIT_2 && 
			    tp->t_timer[TCPT_2MSL] == 0 &&
		      (tp->t_inpcb->inp_socket->so_state & SS_NOFDREF) )
		        	tp->t_timer[TCPT_2MSL] = TCPTV_MSL;
		if (tp->t_inpcb->inp_socket->so_options & SO_KEEPALIVE) {
		    	if (tp->t_idle >= TCPTV_MAXIDLE)
				goto dropit;
			/*
			 * Saying tp->rcv_nxt - 1 lies about what
			 * we have received, and by the protocol spec
			 * requires the correspondent TCP to respond.
			 * Saying tp->snd_una - 1 causes the transmitted
			 * byte to lie outside the receive window; this
			 * is important because we don't necessarily
			 * have a byte in the window to send (consider
			 * a one-way stream!)
			 */
			tcp_respond(tp, tp->t_template,
			    (tcp_seq)(tp->rcv_nxt - 1),
			    (tcp_seq)(tp->snd_una - 1), 0);
		} else
			tp->t_idle = 0;
		tp->t_timer[TCPT_KEEP] = TCPTV_KEEP;
		break;
	dropit:
		tp = tcp_drop(tp, ETIMEDOUT);
		break;
	}
	return (tp);
}
