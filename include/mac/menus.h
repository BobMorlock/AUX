/*
    Menus.h -- Menu Manager interface

    version 2.0a3

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985-1987
    All rights reserved.
*/


#ifndef __MENUS__
#define __MENUS__
#ifndef __TYPES__
#include <types.h>
#endif
#ifndef __QUICKDRAW__
#include <quickdraw.h>
#endif


#define noMark '\0'
#define mDrawMsg 0
#define mChooseMsg 1
#define mSizeMsg 2
#define textMenuProc 0
typedef struct MenuInfo {
    short menuID;
    short menuWidth;
    short menuHeight;
    Handle menuProc;
    long enableFlags;
    Str255 menuData;
} MenuInfo,*MenuPtr,**MenuHandle;
void InitMenus();
MenuHandle NewMenu();
MenuHandle GetMenu();
void DisposeMenu();
void AddResMenu();
void InsertResMenu();
void InsertMenu();
void DrawMenuBar();
void DeleteMenu();
void ClearMenuBar();
Handle GetNewMBar();
Handle GetMenuBar();
void SetMenuBar();
void DelMenuItem();
long MenuKey();
void HiliteMenu();
void DisableItem();
void EnableItem();
void CheckItem();
void SetItemMark();
void GetItemMark();
void SetItemIcon();
void GetItemIcon();
void SetItemStyle();
void GetItemStyle();
void CalcMenuSize();
short CountMItems();
MenuHandle GetMHandle();
void FlashMenuBar();
void SetMenuFlash();



/* Define __ALLNU__ to include routines for Macintosh SE or II. */
#ifdef __ALLNU__


#define hMenuMark       0xC8    /* 'H' character indicates a hierarchical menu */

#define hMenuCmd        0x1B    /* itemCmd == 0x1B ==> hierarchical menu */
#define scriptMenuCmd   0x1C    /* itemCmd == 0x1C ==> item displayed in script font */
#define altMenuCmd1     0x1D    /* itemCmd == 0x1D ==> unused indicator, reserved */
#define altMenuCmd2     0x1E    /* itemCmd == 0x1E ==> unused indicator, reserved */
#define altMenuCmd3     0x1F    /* itemCmd == 0x1F ==> unused indicator, reserved */

#define hierMenu        -1      /* a hierarchical menu - for InsertMenu call */
#define mbRightDir      0       /* menu went to the right (direction) */
#define mbLeftDir       1       /* menu went to the left (direction) */


typedef struct MenuList{
    short           lastMenu;       /* offset */
    short           lastRight;      /* pixels */
    char            mbVariant;      /* mbarproc variant */
    char            mbResID;        /* mbarproc rsrc ID */
    short           lastHMenu;      /* offset */
    short           menuTitleSave;  /* handle to bits behind
					inverted menu title */
} MenuList;

typedef struct MBarDataRec{
    Rect            mbRectSave;     /* menu's rectangle on screen */
    PixMapHandle    mbBitsSave;     /* handle to bits behind menu */
    short           mbMenuDir;      /* did menu go left or right? */
    short           mbMLOffset;     /* menu's MenuList offset */
    short           mbTopScroll;    /* save global TopMenuItem */
    short           mbBotScroll;    /* save global AtMenuBottom */
    long            mbReserved;     /* reserved by Apple */
} MBarDataRec;

typedef struct MBarProcRec{
    short           lastMBSave;     /* offset to last menu saved in structrue */
    Rect            mbItemRect;     /* rect of currently chosen menu item */
    Handle          mbCustomStorage; /* private storage for custom mbarprocs */
} MBarProcRec;

typedef struct MCInfoRec{
    short           mctID;          /* menu ID.  ID = 0 is the menu bar */
    short           mctItem;        /* menu Item. Item = 0 is a title */
    RGBColor        mctRGB1;        /* usage depends on ID and Item */
    RGBColor        mctRGB2;        /* usage depends on ID and Item */
    RGBColor        mctRGB3;        /* usage depends on ID and Item */
    RGBColor        mctRGB4;        /* usage depends on ID and Item */
    short           mctSanityChk;   /* reserved for internal use */
} MCInfoRec, *MCInfoPtr, **MCInfoHandle;


/* Menu Manager */

void InitProcMenu();

/* Color Menu Manager */

void DelMCEntries();
MCInfoHandle GetMCInfo();
void SetMCInfo();
void DispMCInfo();
MCInfoPtr GetMCEntry();
void SetMCEntries();
long MenuChoice();


#endif
#endif
