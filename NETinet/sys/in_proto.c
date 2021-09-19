/*	@(#)in_proto.c 1.1 86/02/03 SMI; from UCB 6.2 83/12/15  */
/*	@(#)in_proto.c	2.1 86/04/15 NFSSRC */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "sys/socket.h"
#include "sys/protosw.h"
#include "sys/domain.h"

#include "netinet/in.h"
#include "netinet/in_systm.h"

/*
 * TCP/IP protocol family: IP, ICMP, UDP, TCP.
 */
int	ip_output();
int	ip_init(),ip_slowtimo(),ip_drain();
int	icmp_input();
int	udp_input(),udp_ctlinput();
int	udp_usrreq();
int	udp_init();
int	tcp_input(),tcp_ctlinput();
int	tcp_usrreq();
int	tcp_init(),tcp_fasttimo(),tcp_slowtimo(),tcp_drain();
int	rip_input(),rip_output();
extern	int raw_usrreq();
/*
 * IMP protocol family: raw interface.
 * Using the raw interface entry to get the timer routine
 * in is a kludge.
 */
#if NIMP > 0
int	rimp_output(), hostslowtimo();
#endif
/*
 * Network disk protocol: runs on top of IP
 */
#if NND > 0
int	nd_input(), nd_init();
#endif

extern struct domain inetdomain;

struct protosw inetsw[] = {
{ 0,		&inetdomain,	0,		0,
  0,		ip_output,	0,		0,
  0,
  ip_init,	0,		ip_slowtimo,	ip_drain,
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_ICMP,	PR_ATOMIC|PR_ADDR,
  icmp_input,	rip_output,	0,		0,
  raw_usrreq,
  0,		0,		0,		0,
},
{ SOCK_DGRAM,	&inetdomain,	IPPROTO_UDP,	PR_ATOMIC|PR_ADDR,
  udp_input,	0,		udp_ctlinput,	0,
  udp_usrreq,
  udp_init,	0,		0,		0,
},
{ SOCK_STREAM,	&inetdomain,	IPPROTO_TCP,	PR_CONNREQUIRED|PR_WANTRCVD,
  tcp_input,	0,		tcp_ctlinput,	0,
  tcp_usrreq,
  tcp_init,	tcp_fasttimo,	tcp_slowtimo,	tcp_drain,
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_RAW,	PR_ATOMIC|PR_ADDR,
  rip_input,	rip_output,	0,	0,
  raw_usrreq,
  0,		0,		0,		0,
},
#if NND > 0
{ 0,		&inetdomain,	IPPROTO_ND,	0,
  nd_input,	0,		0,		0,
  0,
  nd_init,	0,		0,		0,
},
#endif
};

struct domain inetdomain =
    { AF_INET, "internet", inetsw, &inetsw[sizeof(inetsw)/sizeof(inetsw[0])] };

#if NIMP > 0
struct protosw impsw[] = {
{ SOCK_RAW,	PF_IMPLINK,	0,		PR_ATOMIC|PR_ADDR,
  0,		rimp_output,	0,		0,
  raw_usrreq,
  0,		0,		hostslowtimo,	0,
},
};

struct domain impdomain =
    { AF_IMPLINK, "imp", impsw, &impsw[sizeof (impsw)/sizeof(impsw[0])] };
#endif
