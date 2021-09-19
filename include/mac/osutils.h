/*
    OSUtils.h -- Operating System Utilities

    version 2.0a3

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985-1987
    All rights reserved.

    modifications:
    19feb87 KLH added KeyTrans function.
*/

#ifndef __OSUTILS__
#define __OSUTILS__
#ifndef __TYPES__
#include <types.h>
#endif

#define useFree 0
#define useATalk 1
#define useAsync 2
#define macXLMachine 0
#define macMachine 1
typedef struct SysParmType {
    char valid;
    char aTalkA;
    char aTalkB;
    char config;
    short portA;
    short portB;
    long alarm;
    short font;
    short kbdPrint;
    short volClik;
    short misc;
} SysParmType,*SysPPtr;
typedef struct QElem {
    struct QElem *qLink;
    short qType;
    short qData[1];
} QElem,*QElemPtr;
typedef struct QHdr {
    short qFlags;
    QElemPtr qHead;
    QElemPtr qTail;
} QHdr,*QHdrPtr;
typedef char QTypes;        /* NOT enum for UNIX C compiler */
#define dummyType   0
#define vType       1
#define ioQType     2
#define drvQType    3
#define evType      4
#define fsQType     5
typedef struct DateTimeRec {
    short year;
    short month;
    short day;
    short hour;
    short minute;
    short second;
    short dayOfWeek;
} DateTimeRec;
typedef char TrapType;      /* NOT enum for UNIX C compiler */
#define OSTrap      0
#define ToolTrap    1


SysPPtr GetSysPPtr();

void SysBeep();

long KeyTrans();


#endif
