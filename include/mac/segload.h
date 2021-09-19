/*
    Segload.h - Segment Loader

    version 2.0a3

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985,1986
    All rights reserved.
*/

#ifndef __SEGLOAD__
#define __SEGLOAD__
#ifndef __TYPES__
#include <types.h>
#endif

#define appOpen 0
#define appPrint 1
typedef struct AppFile {
    short vRefNum;
    OSType fType;
    short versNum;
    Str255 fName;
} AppFile;
void UnloadSeg();
void ExitToShell();
#endif
