/*
    Time.h -- Time Manager

    version 2.0a3

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985-1987
    All rights reserved.

    modifications:
	10feb87 KLH incorrect extern trap #'s removed.
	13feb87 KLH IM wrong! tmCount is long.
*/

#ifndef __TIME__
#define __TIME__
#ifndef __TYPES__
#include <types.h>
#endif

typedef struct TMTask {
    struct QElem *qLink;
    short qType;
    ProcPtr tmAddr;
    long tmCount;
} TMTask, *TMTaskPtr;

void InsTime();
void PrimeTime();
void RmvTime();
OSErr DTInstall();
#endif
