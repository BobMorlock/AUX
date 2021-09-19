/*
    Resources.h -- Resource Manager

    version 2.0a3

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985-1987
    All rights reserved.
*/

#ifndef __RESOURCES__
#define __RESOURCES__
#ifndef __TYPES__
#include <types.h>
#endif

#define resSysHeap 64
#define resPurgeable 32
#define resLocked 16
#define resProtected 8
#define resPreload 4
#define resChanged 2
#define mapReadOnly 128
#define mapCompact 64
#define mapChanged 32

short InitResources();
void RsrcZoneInit();
void CloseResFile();
short ResError();
short CurResFile();
short HomeResFile();
void UseResFile();
short CountTypes();
short Count1Types();
void GetIndType();
void Get1IndType();
void SetResLoad();
short CountResources();
short Count1Resources();
Handle GetIndResource();
Handle Get1IndResource();
Handle GetResource();
Handle Get1Resource();
extern Handle GetNamedResource();
extern Handle Get1NamedResource();
void LoadResource();
void ReleaseResource();
void DetachResource();
short UniqueID();
short Unique1ID();
short GetResAttrs();
long SizeResource();
long MaxSizeRsrc();
long RsrcMapEntry();
void SetResAttrs();
void ChangedResource();
void RmveResource();
void UpdateResFile();
void WriteResource();
void SetResPurge();
short GetResFileAttrs();
void SetResFileAttrs();



/* Define __ALLNU__ to include routines for Macintosh SE or II. */
#ifdef __ALLNU__


Handle RGetResource();

#endif
#endif
