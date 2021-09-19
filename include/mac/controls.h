/*
    Controls.h -- Color Control Manager interface

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985-87
    All rights reserved.
*/

#ifndef __CONTROLS__
#define __CONTROLS__
#ifndef __QUICKDRAW__
#include <quickdraw.h>
#endif

#define pushButProc 0
#define checkBoxProc 1
#define radioButProc 2
#define useWFont 8
#define scrollBarProc 16
#define inButton 10
#define inCheckBox 11
#define inUpButton 20
#define inDownButton 21
#define inPageUp 22
#define inPageDown 23
#define inThumb 129
#define noConstraint 0
#define hAxisOnly 1
#define vAxisOnly 2
#define drawCntl 0
#define testCntl 1
#define calcCRgns 2
#define initCntl 3
#define dispCntl 4
#define posCntl 5
#define thumbCntl 6
#define dragCntl 7
#define autoTrack 8

typedef struct ControlRecord {
    struct ControlRecord **nextControl;
    struct GrafPort *contrlOwner;
    Rect contrlRect;
    unsigned char contrlVis;
    unsigned char contrlHilite;
    short contrlValue;
    short contrlMin;
    short contrlMax;
    Handle contrlDefProc;
    Handle contrlData;
    ProcPtr contrlAction;
    long contrlRfCon;
    Str255 contrlTitle;
} ControlRecord,*ControlPtr,**ControlHandle;

ControlHandle NewControl();
ControlHandle GetNewControl();
void DisposeControl();
void KillControls();
void HideControl();
void ShowControl();
void DrawControls();
void Draw1Control();
void HiliteControl();
void UpdtControl();
void MoveControl();
void SizeControl();
void SetCtlValue();
short GetCtlValue();
void SetCtlMin();
short GetCtlMin();
void SetCtlMax();
short GetCtlMax();
void SetCRefCon();
long GetCRefCon();
void SetCtlAction();
ProcPtr GetCtlAction();



/* Define __ALLNU__ to include routines for Macintosh SE or II. */
#ifdef __ALLNU__


    /* Constants for the colors of control parts */

#define cFrameColor         0
#define cBodyColor          1
#define cTextColor          2
#define cElevatorColor      3


typedef struct AuxCtlRec{
    Handle          nextAuxCtl;         /* handle to next AuxCtlRec */
    ControlHandle   auxCtlOwner;        /* handle for aux record's control */
    CTabHandle      ctlCTable;          /* color table for this control */
    short           auxCtlFlags;        /* misc flag byte */
    long            caReserved;         /* reserved for use by Apple */
    long            caRefCon;           /* for use by application */
} AuxCtlRec, *AuxCtlPtr, **AuxCtlHndl;


typedef struct CtlCTab{
    long            ccSeed;             /* reserved */
    short           ccRider;            /* see what you have done - reserved */
    short           ctSize;             /* usually 3 for controls */
    ColorSpec       ctTable[4];
} CtlCTab, *CCTabPtr, **CCTabHandle;


void SetCtlColor();
Boolean GetAuxCtl();
short GetCVariant();


#endif
#endif
