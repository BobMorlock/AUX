/*	@(#)uipc_proto.c 1.1 85/05/30 SMI; from UCB 4.28 83/05/27	*/

#include "sys/param.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "sys/protosw.h"
#include "sys/domain.h"
#include "sys/mbuf.h"

/*
 * Definitions of protocols supported in the UNIX domain.
 */

int	raw_init(),raw_usrreq(),raw_input(),raw_ctlinput();

struct protosw unixsw[] = {
{ 0,		0,		0,		0,
  raw_input,	0,		raw_ctlinput,	0,
  raw_usrreq,
  raw_init,	0,		0,		0,
}
};

struct domain unixdomain =
    { AF_UNIX, "unix", unixsw, &unixsw[sizeof(unixsw)/sizeof(unixsw[0])] };
