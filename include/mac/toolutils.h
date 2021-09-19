/*
    Toolutils.h -- Toolbox Utilities

    version 2.0a3

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985,1986
    All rights reserved.
*/

#ifndef __TOOLUTILS__
#define __TOOLUTILS__
#ifndef __QUICKDRAW__
#include <quickdraw.h>
#endif

#define sysPatListID 0
#define iBeamCursor 1
#define crossCursor 2
#define plusCursor 3
#define watchCursor 4
typedef struct Int64Bit {
    long hiLong;
    long loLong;
} Int64Bit;
typedef struct Cursor *CursPtr,**CursHandle;
typedef Pattern *PatPtr,**PatHandle;

Fixed FixRatio();
Fixed FixMul();
short FixRound();
StringHandle NewString();
StringHandle GetString();
long Munger();
void PackBits();
void UnpackBits();
Boolean BitTst();
void BitSet();
void BitClr();
long BitAnd();
long BitOr();
long BitXor();
long BitNot();
long BitShift();
short HiWord();
short LoWord();
void LongMul();
Handle GetIcon();
void PlotIcon();
PatHandle GetPattern();
CursHandle GetCursor();
struct Picture **GetPicture();
Fixed SlopeFromAngle();
short AngleFromSlope();
#endif
