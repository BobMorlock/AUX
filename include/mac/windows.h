/*
    Windows.h -- Color Window Manager interface

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985-1987
    All rights reserved.

    modifications:
    01/26/87    KLH     Corrected AuxWinRecHndl by adding AuxWinRecPtr.
*/

#ifndef __WINDOWS__
#define __WINDOWS__
#ifndef __QUICKDRAW__
#include <quickDraw.h>
#endif

#define documentProc 0
#define dBoxProc 1
#define plainDBox 2
#define altDBoxProc 3
#define noGrowDocProc 4
#define zoomDocProc 8
#define zoomNoGrow 12
#define rDocProc 16
#define dialogKind 2
#define userKind 8
#define inDesk 0
#define inMenuBar 1
#define inSysWindow 2
#define inContent 3
#define inDrag 4
#define inGrow 5
#define inGoAway 6
#define inZoomIn 7
#define inZoomOut 8
#define noConstraint 0
#define hAxisOnly 1
#define vAxisOnly 2
#define wDraw 0
#define wHit 1
#define wCalcRgns 2
#define wNew 3
#define wDispose 4
#define wGrow 5
#define wDrawGIcon 6
#define wNoHit 0
#define wInContent 1
#define wInDrag 2
#define wInGrow 3
#define wInGoAway 4
#define wInZoomIn 5
#define wInZoomOut 6
#define deskPatID 16

typedef GrafPtr WindowPtr;
typedef struct WindowRecord {
     GrafPort port;
     short windowKind;
     Boolean visible;
     Boolean hilited;
     Boolean goAwayFlag;
     Boolean spareFlag;
     RgnHandle strucRgn;
     RgnHandle contRgn;
     RgnHandle updateRgn;
     Handle windowDefProc;
     Handle dataHandle;
     StringHandle titleHandle;
     short titleWidth;
     struct ControlRecord **controlList;
     struct WindowRecord *nextWindow;
     PicHandle windowPic;
     long refCon;
} WindowRecord,*WindowPeek;
typedef struct WStateData {
     Rect userState;
     Rect stdState;
} WStateData;

void InitWindows();
void GetWMgrPort();
WindowPtr NewWindow();
WindowPtr GetNewWindow();
void CloseWindow();
void DisposeWindow();
void SelectWindow();
void HideWindow();
void ShowWindow();
void ShowHide();
void HiliteWindow();
void BringToFront();
void SendBehind();
WindowPtr FrontWindow();
void DrawGrowIcon();
void MoveWindow();
void SizeWindow();
void ZoomWindow();
void InvalRect();
void InvalRgn();
void ValidRect();
void ValidRgn();
void BeginUpdate();
void EndUpdate();
void SetWRefCon();
long GetWRefCon();
void SetWindowPic();
PicHandle GetWindowPic();
Boolean CheckUpdate();
void ClipAbove();
void SaveOld();
void DrawNew();
void PaintOne();
void PaintBehind();
void CalcVis();
void CalcVisBehind();



/* Define __ALLNU__ to include routines for Macintosh SE or II. */
#ifdef __ALLNU__


    /* Window Part Identifiers which correlate color table
	entries with window elements */
#define wContentColor       0
#define wFrameColor         1
#define wTextColor          2
#define wHiliteColor        3
#define wTitleBarColor      4


typedef struct AuxWinRec{
    struct AuxWinRec **nextAuxWin;      /* handle to next AuxWinRec */
    WindowPtr       auxWinOwner;        /* ptr to window */
    CTabHandle      winCTable;          /* color table for this window */
    Handle          dialogCTable;       /* handle to dialog manager structures */
    Handle          waResrv2;           /* handle reserved */
    long            waResrv;            /* for expansion */
    long            waRefCon;           /* user constant */
} AuxWinRec, *AuxWinPtr, **AuxWinHndl;

typedef struct WinCTab{
    long            wCSeed;             /* reserved */
    short           wCReserved;         /* reserved */
    short           ctSize;             /* usually 4 for windows */
    ColorSpec       ctTable[5];
} WinCTab, *WCTabPtr, **WCTabHandle;


void GetCWMgrPort();
void SetWinColor();
Boolean GetAuxWin();
void SetDeskCPat();
WindowPtr NewCWindow();
WindowPtr GetNewCWindow();
short GetWVariant();


#endif
#endif
