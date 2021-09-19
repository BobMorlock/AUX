/*
    TextEdit.h -- Text Edit Manager interface

    Version: 2.0a3

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985-1987
    All rights reserved.

    modifications:
    02/17/87    KLH added new TE calls.
*/

#ifndef __TEXTEDIT__
#define __TEXTEDIT__
#ifndef __QUICKDRAW__
#include <quickdraw.h>
#endif
#ifndef __TYPES__
#include <types.h>
#endif

#define teJustLeft 0
#define teJustCenter 1
#define teJustRight (-1)
typedef struct TERec {
    Rect destRect;
    Rect viewRect;
    Rect selRect;
    short lineHeight;
    short fontAscent;
    Point selPoint;
    short selStart;
    short selEnd;
    short active;
    ProcPtr wordBreak;
    ProcPtr clikLoop;
    long clickTime;
    short clickLoc;
    long caretTime;
    short caretState;
    short just;
    short teLength;
    Handle hText;
    short recalBack;
    short recalLines;
    short clikStuff;
    short crOnly;
    short txFont;
    Style txFace;           /* txFace is unpacked byte */
    char filler;
    short txMode;
    short txSize;
    struct GrafPort *inPort;
    ProcPtr highHook;
    ProcPtr caretHook;
    short nLines;
    short lineStarts[16001];
} TERec,*TEPtr,**TEHandle;
typedef char Chars[32001];
typedef Chars *CharsPtr,**CharsHandle;

void TEInit();
TEHandle TENew();
void TEDispose();
void TESetText();
CharsHandle TEGetText();
void TEIdle();
void TESetSelect();
void TEActivate();
void TEDeactivate();
void TEKey();
void TECut();
void TECopy();
void TEPaste();
void TEDelete();
void TEInsert();
void TESetJust();
void TEUpdate();
void TextBox();
void TEScroll();
void TESelView();
void TEPinScroll();
void TEAutoView();
Handle TEScrapHandle();
void TECalText();



/* Define __ALLNU__ to include routines for Macintosh SE or II. */
#ifdef __ALLNU__


#define doFont          1               /* set font (family) number */
#define doFace          2               /* set character style */
#define doSize          4               /* set type size */
#define doColor         8               /* set color */
#define doAll           15              /* set all attributes */
#define addSize         16              /* adjust type size */


typedef struct StyleRun{
    short           startChar;          /* starting character position */
    short           styleIndex;         /* index in style table */
} StyleRun;

typedef struct STElement{
    short           stCount;            /* number of runs in this style */
    short           stHeight;           /* line height */
    short           stAscent;           /* font ascent */
    short           stFont;             /* font (family) number */
    Style           stFace;             /* character Style */
    char            filler;             /* stFace is unpacked byte */
    short           stSize;             /* size in points */
    RGBColor        stColor;            /* absolute (RGB) color */
} STElement;

typedef STElement TEStyleTable[1777], *STPtr, **STHandle;

typedef struct LHElement{
    short           lhHeight;           /* maximum height in line */
    short           lhAscent;           /* maximum ascent in line */
} LHElement;

typedef LHElement LHTable[8001], *LHPtr, **LHHandle;
					/* ARRAY [0..8000] OF LHElement */

typedef struct TEStyleRec{
    short           nRuns;              /* number of style runs */
    short           nStyles;            /* size of style table */
    STHandle        styleTab;           /* handle to style table */
    LHHandle        lhTab;              /* handle to line-height table */
    long            teRefCon;           /* reserved for application use */
    long            teReserved;         /* reserved for future expansion */
    StyleRun        runs[8001];         /* ARRAY [0..8000] OF StyleRun */
} TEStyleRec, *TEStylePtr, **TEStyleHandle;

typedef struct TextStyle{
    short           tsFont;             /* font (family) number */
    Style           tsFace;             /* character Style */
    char            filler;             /* tsFace is unpacked byte */
    short           tsSize;             /* size in point */
    RGBColor        tsColor;            /* absolute (RGB) color */
} TextStyle;

typedef struct ScrpSTElement{
    long            scrpStartChar;      /* starting character position */
    short           scrpHeight;
    short           scrpAscent;
    short           scrpFont;
    Style           scrpFace;           /* unpacked byte */
    char            filler;             /* scrpFace is unpacked byte */
    short           scrpSize;
    RGBColor        scrpColor;
} ScrpSTElement;

typedef ScrpSTElement ScrpSTTable[1601];    /* ARRAY [0..1600] OF ScrpSTElement */

typedef struct StScrpRec{
    short           scrpNStyles;            /* number of styles in scrap */
    ScrpSTTable     scrpStyleTab;           /* table of styles for scrap */
} StScrpRec, *StScrpPtr, **StScrpHandle;



TEHandle TEStylNew();
void SetStylHandle();
TEStyleHandle GetStylHandle();
short TEGetOffset();
void TEGetStyle();
void TEStylPaste();
void TESetStyle();
void TEReplaceStyle();
StScrpHandle GetStylScrap();
void TEStylInsert();
Point TEGetPoint();
long TEGetHeight();


#endif
#endif
