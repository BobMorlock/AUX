/*
    Lists.h -- List Manager

    version 2.0a3

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985,1986
    All rights reserved.
*/

#ifndef __LISTS__
#define __LISTS__
#ifndef __TYPES__
#include <types.h>
#endif

#define lDoVAutoscroll 2
#define lDoHAutoscroll 1
#define lOnlyOne (-128)
#define lExtendDrag 0x40
#define lNoDisjoint 0x20
#define lNoExtend 0x10
#define lNoRect 0x08
#define lUseSense 0x04
#define lNoNilHilite 0x02

#define lInitMsg 0
#define lDrawMsg 1
#define lHiliteMsg 2
#define lCloseMsg 3

typedef Point Cell;
typedef struct ListRec {
    Rect rView;
    struct GrafPort *port;
    Point indent;
    Point cellSize;
    Rect visible;
    struct ControlRecord **vScroll;
    struct ControlRecord **hScroll;
    char selFlags;
    char lActive;
    char lReserved;
    char listFlags;
    long clikTime;
    Point clikLoc;
    Point mouseLoc;
    ProcPtr lClikLoop;
    Cell lastClick;
    long refCon;
    Handle listDefProc;
    Handle userHandle;
    Rect dataBounds;
    Handle cells;
    short maxIndex;
    short cellArray[1];
} ListRec,*ListPtr,**ListHandle;

extern ListHandle LNew();
void LDispose();
short LAddColumn();
short LAddRow();
void LDelColumn();
void LDelRow();
Boolean LGetSelect ();
Cell LLastClick ();
Boolean LNextCell();
Boolean LSearch();
void LSize();
void LDoDraw();
void LScroll();
void LAutoScroll();
void LUpdate();
void LActivate();
#endif
