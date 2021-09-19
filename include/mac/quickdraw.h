/*
    Quickdraw.h -- Color Quickdraw interface

    Version: 2.0a3

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985-1987
    All rights reserved.
*/
/*
    modifications:
    01/22/87    KLH added CQDProcs record instead of lengthening existing
			QDProcs.
    01/26/87    KLH gdPMap previously changed from handle to PixMapHandle,
			but misspelled as gdDPMap.
    15Dec86     DAF     added gamma table format
    16Feb87     KLH OpenCPicture changed to agree with OpenPicture.
    19feb87     KLH SetEntries parameters corrected.

*/


#ifndef __QUICKDRAW__
#define __QUICKDRAW__
#ifndef __TYPES__
#include <types.h>
#endif

#define srcCopy 0
#define srcOr 1
#define srcXor 2
#define srcBic 3
#define notSrcCopy 4
#define notSrcOr 5
#define notSrcXor 6
#define notSrcBic 7
#define patCopy 8
#define patOr 9
#define patXor 10
#define patBic 11
#define notPatCopy 12
#define notPatOr 13
#define notPatXor 14
#define notPatBic 15
#define normalBit 0
#define inverseBit 1
#define redBit 4
#define greenBit 3
#define blueBit 2
#define cyanBit 8
#define magentaBit 7
#define yellowBit 6
#define blackBit 5
#define blackColor 33
#define whiteColor 30
#define redColor 205
#define greenColor 341
#define blueColor 409
#define cyanColor 273
#define magentaColor 137
#define yellowColor 69
#define picLParen 0
#define picRParen 1
#define normal 0x00
#define bold 0x01
#define italic 0x02
#define underline 0x04
#define outline 0x08
#define shadow 0x10
#define condense 0x20
#define extend 0x40

typedef char QDByte, *QDPtr, **QDHandle;
typedef unsigned char Pattern[8];
typedef short Bits16[16];
typedef char GrafVerb;      /* not enum for UNIX C compiler */
#define frame   0
#define paint   1
#define erase   2
#define invert  3
#define fill    4
typedef struct FontInfo {
    short ascent;
    short descent;
    short widMax;
    short leading;
} FontInfo;
typedef struct BitMap {
    Ptr baseAddr;
    short rowBytes;
    Rect bounds;
} BitMap;
typedef struct Cursor {
    Bits16 data;
    Bits16 mask;
    Point hotSpot;
} Cursor;
typedef struct PenState {
    Point pnLoc;
    Point pnSize;
    short pnMode;
    Pattern pnPat;
} PenState;
typedef struct Region {
    short rgnSize;
    Rect rgnBBox;
    short rgnData[1];
} Region,*RgnPtr,**RgnHandle;
typedef struct Picture {
    short picSize;
    Rect picFrame;
    short picData[1];
} Picture,*PicPtr,**PicHandle;
typedef struct Polygon {
    short polySize;
    Rect polyBBox;
    Point polyPoints[1];
} Polygon,*PolyPtr,**PolyHandle;
typedef struct QDProcs {
    Ptr textProc;
    Ptr lineProc;
    Ptr rectProc;
    Ptr rRectProc;
    Ptr ovalProc;
    Ptr arcProc;
    Ptr polyProc;
    Ptr rgnProc;
    Ptr bitsProc;
    Ptr commentProc;
    Ptr txMeasProc;
    Ptr getPicProc;
    Ptr putPicProc;
} QDProcs,*QDProcsPtr;
typedef struct GrafPort {
    short device;
    BitMap portBits;
    Rect portRect;
    RgnHandle visRgn;
    RgnHandle clipRgn;
    Pattern bkPat;
    Pattern fillPat;
    Point pnLoc;
    Point pnSize;
    short pnMode;
    Pattern pnPat;
    short pnVis;
    short txFont;
    Style txFace;           /* txFace is unpacked byte, but push as short */
    char filler;
    short txMode;
    short txSize;
    Fixed spExtra;
    long fgColor;
    long bkColor;
    short colrBit;
    short patStretch;
    PicHandle picSave;
    RgnHandle rgnSave;
    PolyHandle polySave;
    QDProcsPtr grafProcs;
} GrafPort,*GrafPtr;
extern struct qd {
    char private[76];
    long randSeed;
    BitMap screenBits;
    Cursor arrow;
    Pattern dkGray;
    Pattern ltGray;
    Pattern gray;
    Pattern black;
    Pattern white;
    GrafPtr thePort;
} qd;

void InitGraf();
void OpenPort();
void InitPort();
void ClosePort();
void SetPort();
void GetPort();
void GrafDevice();
void SetPortBits();
void PortSize();
void MovePortTo();
void SetOrigin();
void SetClip();
void GetClip();
void ClipRect();
void BackPat();
void InitCursor();
void SetCursor();
void HideCursor();
void ShowCursor();
void ObscureCursor();
void HidePen();
void ShowPen();
void GetPen();
void GetPenState();
void SetPenState();
void PenSize();
void PenMode();
void PenPat();
void PenNormal();
void MoveTo();
void Move();
void LineTo();
void Line();
void TextFont();
void TextFace();
void TextMode();
void TextSize();
void SpaceExtra();
void DrawChar();
void DrawText();
short CharWidth();
short TextWidth();
void MeasureText();
void GetFontInfo();
void ForeColor();
void BackColor();
void ColorBit();
void SetRect();
void OffsetRect();
void InsetRect();
Boolean SectRect();
void UnionRect();
Boolean EqualRect();
Boolean EmptyRect();
void FrameRect();
void PaintRect();
void EraseRect();
void InvertRect();
void FillRect();
void FrameOval();
void PaintOval();
void EraseOval();
void InvertOval();
void FillOval();
void FrameRoundRect();
void PaintRoundRect();
void EraseRoundRect();
void InvertRoundRect();
void FillRoundRect();
void FrameArc();
void PaintArc();
void EraseArc();
void InvertArc();
void FillArc();
RgnHandle NewRgn();
void OpenRgn();
void CloseRgn();
void DisposeRgn();
void CopyRgn();
void SetEmptyRgn();
void SetRectRgn();
void RectRgn();
void OffsetRgn();
void InsetRgn();
void SectRgn();
void UnionRgn();
void DiffRgn();
void XorRgn();
Boolean RectInRgn();
Boolean EqualRgn();
Boolean EmptyRgn();
void FrameRgn();
void PaintRgn();
void EraseRgn();
void InvertRgn();
void FillRgn();
void ScrollRect();
void CopyBits();
void SeedFill();
void CalcMask();
void CopyMask();
PicHandle OpenPicture();
void PicComment();
void ClosePicture();
void DrawPicture();
void KillPicture();
PolyHandle OpenPoly();
void ClosePoly();
void KillPoly();
void OffsetPoly();
void FramePoly();
void PaintPoly();
void ErasePoly();
void InvertPoly();
void FillPoly();
void SetPt();
void LocalToGlobal();
void GlobalToLocal();
short Random();
Boolean GetPixel();
void ScalePt();
void MapPt();
void MapRect();
void MapRgn();
void MapPoly();
void SetStdProcs();
void StdRect();
void StdRRect();
void StdOval();
void StdArc();
void StdPoly();
void StdRgn();
void StdBits();
void StdComment();
short StdTxMeas();
void StdGetPic();
void StdPutPic();



/* Define __ALLNU__ to include routines for Macintosh SE or II. */
#ifdef __ALLNU__


#define invalColReq     -1          /* invalid color table request */

/* VALUES FOR GDType */

#define clutType        0               /*  0 if lookup table */
#define fixedType       1               /*  1 if fixed table */
#define directType      2               /*  2 if direct values */

/*  BIT ASSIGNMENTS FOR GDFlags */

#define gdDevType       0               /* 0 = monochrome; 1 = color */
#define ramInit         10              /* 1 if initialized from 'scrn' resource */
#define mainScrn        11              /* 1 if main screen */
#define allInit         12              /* 1 if all devices initialized */
#define screenDevice    13              /* 1 if screen device [not used] */
#define noDriver        14              /* 1 if no driver for this GDevice */
#define scrnActive      15              /* 1 if in use */

#define hiliteBit       7               /* flag bit in HiliteMode (lowMem flag) */

#define defQDColors     127         /* resource ID of clut for default QDColors */


typedef struct RGBColor{
    short           red;                /* magnitude of red component */
    short           green;              /* magnitude of green component */
    short           blue;               /* magnitude of blue component */
} RGBColor;

typedef struct ColorSpec{
    short           value;              /* index or other value */
    RGBColor        rgb;                /* true color */
} ColorSpec;

typedef struct CSpecArray{
    ColorSpec       ctTable[1];         /* array [0..0] of ColorSpec */
} CSpecArray;

typedef struct ColorTable{
    long            ctSeed;             /* unique identifier for table */
    short           transIndex;         /* index of transparent pixel */
    short           ctSize;             /* number of entries in CTTable */
    CSpecArray      ctTable;            /* array [0..0] of ColorSpec */
} ColorTable, *CTabPtr, **CTabHandle;

typedef struct PixMap{
    Ptr             baseAddr;           /* pointer to pixels */
    short           rowBytes;           /* offset to next line */
    Rect            bounds;             /* encloses bitmap */
    short           pmVersion;          /* pixMap version number */
    short           packType;           /* defines packing format */
    long            packSize;           /* length of pixel data */
    Fixed           hRes;               /* horiz. resolution (ppi) */
    Fixed           vRes;               /* vert. resolution (ppi) */
    short           pixelType;          /* defines pixel type */
    short           pixelSize;          /* # bits in pixel */
    short           cmpCount;           /* # components in pixel */
    short           cmpSize;            /* # bits per component */
    long            planeBytes;         /* offset to next plane */
    CTabHandle      pmTable;            /* color map for this pixMap */
    long            pmReserved;         /* for future use. MUST BE 0 */
} PixMap, *PixMapPtr, **PixMapHandle;

typedef struct PatXMap{
    short           patXRow;            /* rowbytes of expanded pattern */
    short           patXHMask;          /* horizontal mask */
    short           patXVMask;          /* vertical mask */
    long            lastCTable;         /* seed value for last color table */
    short           lastOfst;           /* last global-local offset */
    long            lastInvert;         /* last invert value */
    long            lastAlign;          /* last horizontal align */
    short           lastStretch;        /* last stretch */
} PatXMap, *PatXMapPtr, **PatXMapHandle;

typedef struct PixPat{
    short           patType;            /* type of pattern */
    PixMapHandle    patMap;             /* the pattern's pixMap */
    Handle          patData;            /* pixmap's data */
    Handle          patXData;           /* expanded pattern data */
    short           patXValid;          /* [word] flags whether expanded pattern valid */
    PatXMapHandle   patXMap;            /* [long] handle to expanded pattern data */
    Pattern         pat1Data;           /* old-style pattern/RGB color */
} PixPat, *PixPatPtr, **PixPatHandle;

typedef struct CCrsr{
    short           crsrType;           /* type of cursor */
    PixMapHandle    crsrMap;            /* the cursor's pixmap */
    Handle          crsrData;           /* cursor's data */
    Handle          crsrXData;          /* expanded cursor data */
    short           crsrXValid;         /* depth of expanded data (0 if none) */
    Handle          crsrXHandle;        /* future use */
    Bits16          crsr1Data;          /* one-bit cursor */
    Bits16          crsrMask;           /* cursor's mask */
    Point           crsrHotSpot;        /* cursor's hotspot */
    long            crsrXTable;         /* private */
    long            crsrID;             /* private */
} CCrsr, *CCrsrPtr, **CCrsrHandle;

typedef struct CIcon{
    PixMap          iconPMap;           /* the icon's pixMap */
    BitMap          iconMask;           /* the icon's mask */
    BitMap          iconBMap;           /* the icon's bitMap */
    Handle          iconData;           /* the icon's data */
    short           iconMaskData[1];    /* icon's mask and bitmap data */
} CIcon, *CIconPtr, **CIconHandle;

typedef struct GammaTbl{
    short           gVersion;           /* gamma version number  */
    short           gType;              /* gamma data type */
    short           gFormulaSize;       /* Formula data size */
    short           gChanCnt;           /* number of channels of data */
    short           gDataCnt;           /* number of values/channel */
    short           gDataWidth;         /* bits/corrected value (data packed to next larger byte size) */
    short           gFormulaData[1];    /* data for formulas, followed by gamma values */
} GammaTbl, *GammaTblPtr, **GammaTblHandle;

typedef struct ITab{
    long            iTabSeed;           /* copy of CTSeed from source CTable */
    short           iTabRes;            /* bits/channel resolution of iTable */
    unsigned char   iTTable[1];         /* byte colortable index values */
} ITab, *ITabPtr, **ITabHandle;

typedef struct GDevice{
    short           gdRefNum;           /* driver's unit number */
    short           gdID;               /* client ID for search procs */
    short           gdType;             /* fixed/CLUT/direct */
    ITabHandle      gdITable;           /* handle to inverse lookup table */
    short           gdResPref;          /* preferred resolution of GDITable */
    ProcPtr         gdSearchProc;       /* search proc list head */
    ProcPtr         gdCompProc;         /* complement proc list */
    short           gdFlags;            /* grafDevice flags word */
    PixMapHandle    gdPMap;             /* describing pixMap */
    long            gdRefCon;           /* reference value */
    Handle          gdNextGD;   /*GDHandle*/    /* handle of next gDevice */
    Rect            gdRect;             /*device's bounds in global coordinates */
    long            gdMode;             /* device's current mode */
    short           gdCCBytes;          /* depth of expanded cursor data */
    short           gdCCDepth;          /* depth of expanded cursor data */
    Handle          gdCCXData;          /*handle to cursor's expanded data */
    Handle          gdCCXMask;          /* handle to cursor's expanded mask */
    long            gdReserved;         /* future use. MUST BE 0 */
} GDevice, *GDPtr, **GDHandle;

typedef struct CGrafPort{
    short           device;
    PixMapHandle    portPixMap;         /* port's pixel map */
    short           portVersion;        /* high 2 bits always set */
    Handle          grafVars;           /* handle to more fields */
    short           chExtra;            /* character extra */
    short           pnLocHFrac;         /* pen fraction */
    Rect            portRect;
    RgnHandle       visRgn;
    RgnHandle       clipRgn;
    PixPatHandle    bkPixPat;           /* background pattern */
    RGBColor        rgbFgColor;         /* RGB components of fg */
    RGBColor        rgbBkColor;         /* RGB components of bk */
    Point           pnLoc;
    Point           pnSize;
    short           pnMode;
    PixPatHandle    pnPixPat;           /* pen's pattern */
    PixPatHandle    fillPixPat;         /* fill pattern */
    short           pnVis;
    short           txFont;
    Style           txFace;     /* txFace is unpacked byte, push as short */
    char            filler;
    short           txMode;
    short           txSize;
    Fixed           spExtra;
    long            fgColor;
    long            bkColor;
    short           colrBit;
    short           patStretch;
    QDHandle        picSave;
    QDHandle        rgnSave;
    QDHandle        polySave;
    QDProcsPtr      grafProcs;
} CGrafPort, *CGrafPtr;

typedef struct GrafVars{
    RGBColor        rgbOpColor;         /* color for addPin, subPin and average */
    RGBColor        rgbHiliteColor;     /* color for hiliting */
    Handle          pmFgColor;          /* palette handle for foreground color */
    short           pmFgIndex;          /* index value for foreground */
    Handle          pmBkColor;          /* palette handle for background color */
    short           pmBkIndex;          /* index value for background */
    short           pmFlags;            /* flags for Palette Manager */
} GrafVars;


typedef struct SProcRec{
    Handle          nxtSrch;    /*SProcHndl*/       /* handle to next SProcRec */
    ProcPtr         srchProc;           /* pointer to search procedure */
} SProcRec, *SProcPtr, **SProcHndl;

typedef struct CProcRec{
    Handle          nxtComp;    /*CProcHndl*/       /* handle to next CProcRec */
    ProcPtr         compProc;           /* pointer to complement procedure */
} CProcRec, *CProcPtr, **CProcHndl;

typedef struct CQDProcs {
    Ptr             textProc;
    Ptr             lineProc;
    Ptr             rectProc;
    Ptr             rRectProc;
    Ptr             ovalProc;
    Ptr             arcProc;
    Ptr             polyProc;
    Ptr             rgnProc;
    Ptr             bitsProc;
    Ptr             commentProc;
    Ptr             txMeasProc;
    Ptr             getPicProc;
    Ptr             putPicProc;
    Ptr             opcodeProc;         /* fields added to QDProcs */
    Ptr             newProc1;
    Ptr             newProc2;
    Ptr             newProc3;
    Ptr             newProc4;
    Ptr             newProc5;
    Ptr             newProc6;
} CQDProcs,*CQDProcsPtr;

typedef short QDErr;

typedef struct ReqListRec{              /* request List structure */
    short           reqLSize;           /* request list size */
    short           reqLData[1];        /* request list data */
} ReqListRec;


	    /* Routines for Manipulating the CGrafport */

void OpenCPort();
void InitCPort();
void CloseCPort();

	    /* Routines for Manipulating PixMaps */

PixMapHandle NewPixMap();
void DisposPixMap();
void CopyPixMap();
void SetCPortPix();

	    /* Routines for Manipulating PixPats */

PixPatHandle NewPixPat();
void DisposPixPat();
void CopyPixPat();
void PenPixPat();
void BackPixPat();
PixPatHandle GetPixPat();
void MakeRGBPat();

void FillCRect();
void FillCOval();
void FillCRoundRect();
void FillCArc();
void FillCRgn();
void FillCPoly();

void RGBForeColor();
void RGBBackColor();
void SetCPixel();
ColorSpec GetCPixel();
void GetForeColor();
void GetBackColor();

	    /* Transfer Mode Utilities  */

void OpColor();
void HiliteColor();

	    /* Color Table Handling Routines */

void DisposCTable();
CTabHandle GetCTable();

	    /* Color Cursor Handling Routines */

CCrsrHandle GetCCursor();
void SetCCursor();
void AllocCursor();
void DisposCCursor();

	    /* Icon Handling Routines */

CIconHandle GetCIcon();
void PlotCIcon();
void DisposCIcon();

	    /* PixMap Handling Routines */

void CopyPix();
void CopyCMask();

	    /* Picture Routines */

PicHandle OpenCPicture();

	    /* Text Routines */

void CharExtra();

	    /* GDevice Routines */

GDHandle GetMaxDevice();
long GetCTSeed();
GDHandle GetDeviceList();
GDHandle GetMainDevice();
GDHandle GetNextDevice();
Boolean TestDeviceAttribute();
void SetDeviceAttribute();
void InitGDevice();
GDHandle NewGDevice();
void DisposGDevice();
void SetGDevice();
GDHandle GetGDevice();

	    /* Color Manager Interface */

void Color2Index();
void Index2Color();
void InvertColor();
Boolean RealColor();
void GetSubTable();
void UpdatePixMap();
void MakeITable();
void AddSearch();
void AddComp();
void DelSearch();
void DelComp();
void SetClientID();
void ProtectEntry();
void ReserveEntry();
void SetEntries();
void SaveEntries();
void RestoreEntries();
short QDError();


#endif                  /* __ALLNU__ */
#endif                  /* __QUICKDRAW__ */

