/*
    Scrap.h -- Scrap Manager

    version 2.0a3

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985,1986
    All rights reserved.
*/

#ifndef __SCRAP__
#define __SCRAP__
#ifndef __TYPES__
#include <types.h>
#endif

typedef struct ScrapStuff {
    long scrapSize;
    Handle scrapHandle;
    short scrapCount;
    short scrapState;
    StringPtr scrapName;
} ScrapStuff,*PScrapStuff;

PScrapStuff InfoScrap();
long UnloadScrap();
long LoadScrap();
long GetScrap();
long ZeroScrap();
long PutScrap();
#endif
