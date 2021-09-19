/*
    Dialogs.h -- Dialog Manager

    version 2.0a3

    C Interface to the Macintosh Libraries
    Copyright Apple Computer,Inc. 1985,1986
    All rights reserved.
*/

#ifndef __DIALOGS__
#define __DIALOGS__
#ifndef __WINDOWS__
#include <windows.h>
#endif

#define ctrlItem 4
#define btnCtrl 0
#define chkCtrl 1
#define radCtrl 2
#define resCtrl 3
#define statText 8
#define editText 16
#define iconItem 32
#define picItem 64
#define userItem 0
#define itemDisable 128
#define ok 1
#define cancel 2
#define stopIcon 0
#define noteIcon 1
#define cautionIcon 2

typedef WindowPtr DialogPtr;
typedef struct DialogRecord {
    WindowRecord window;
    Handle items;
    struct TERec **textH;
    short editField;
    short editOpen;
    short aDefItem;
} DialogRecord,*DialogPeek;
typedef struct DialogTemplate {
    Rect boundsRect;
    short procID;
    Boolean visible;
    Boolean filler1;
    Boolean goAwayFlag;
    Boolean filler2;
    long refCon;
    short itemsID;
    Str255 title;
} DialogTemplate,*DialogTPtr,**DialogTHndl;
typedef short StageList;
typedef struct AlertTemplate {
    Rect boundsRect;
    short itemsID;
    StageList stages;
} AlertTemplate,*AlertTPtr,**AlertTHndl;

void InitDialogs();
void ErrorSound();
DialogPtr NewDialog();
DialogPtr GetNewDialog();
void CloseDialog();
void DisposDialog();
void CouldDialog();
void FreeDialog();
void ModalDialog();
Boolean IsDialogEvent();
Boolean DialogSelect();
void DrawDialog();
void UpdtDialog();
short Alert();
short StopAlert();
short NoteAlert();
short CautionAlert();
void CouldAlert();
void FreeAlert();
void GetDItem();
void SetDItem();
void HideDItem();
void ShowDItem();
void SelIText();



/* Define __ALLNU__ to include routines for Macintosh SE or II. */
#ifdef __ALLNU__


DialogPtr NewCDialog();

#endif
#endif
