/*
    Desk.h -- Desk Manager

    version 2.0a3

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985,1986
    All rights reserved.
*/

#ifndef __DESK__
#define __DESK__
#ifndef __TYPES__
#include <types.h>
#endif

#define accEvent 64
#define accRun 65
#define accCursor 66
#define accMenu 67
#define accUndo 68
#define accCut 70
#define accCopy 71
#define accPaste 72
#define accClear 73


void CloseDeskAcc();
void SystemClick();
Boolean SystemEdit();
void SystemTask();
Boolean SystemEvent();
void SystemMenu();

#endif
