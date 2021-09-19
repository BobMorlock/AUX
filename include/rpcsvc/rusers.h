/*	@(#)rusers.h 1.1 85/05/30 SMI */

/* 
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

#define RUSERSPROC_NUM 1
#define RUSERSPROC_NAMES 2
#define RUSERSPROC_ALLNAMES 3
#define RUSERSPROG 100002
#define RUSERSVERS 1

#define MAXUSERS 100

struct utmparr {
	struct utmp **uta_arr;
	int uta_cnt
};

int xdr_utmparr();
