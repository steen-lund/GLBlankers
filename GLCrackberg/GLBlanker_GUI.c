#include <utility/tagitem.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/minigl.h>

// Reaction Stuff
#include <classes/window.h>
#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>
#include <gadgets/layout.h>
#include <gadgets/getscreenmode.h>
#include <images/label.h>
#include <intuition/icclass.h>
#include <proto/getscreenmode.h>
#include <proto/label.h>
#include <proto/layout.h>

#include "blanker.h"

#define GA_ScreenMode   0x1001

struct Library* GetScreenModeBase   = NULL;
struct Library* LabelBase           = NULL;
struct Library* LayoutBase          = NULL;

struct GetScreenModeIFace*  IGetScreenMode  = NULL;
struct LabelIFace*          ILabel          = NULL;
struct LayoutIFace*         ILayout         = NULL;

struct Gadget *screenmode_requester = NULL;

GLint GUIModeID = -1;
ULONG temp_mode_ID = 0xffffffff;
struct Hook* ScreenmodeHook = NULL;

void GUIEventFunc(struct Hook* hook, struct BlankerModuleIFace* Self, struct BlankerPrefsWinGUIEvent* event);

const struct Hook GUIEventHook = {{ NULL, NULL }, (HOOKFUNC)GUIEventFunc, NULL, NULL };

uint32 OpenGUILibraries()
{
    GetScreenModeBase = IExec->OpenLibrary("gadgets/getscreenmode.gadget", 51);
    IGetScreenMode = (struct GetScreenModeIFace *)IExec->GetInterface( GetScreenModeBase, "main", 1, NULL);
    if (IGetScreenMode == NULL)
    {
        return FALSE;
    }

    LabelBase = IExec->OpenLibrary( "images/label.image", 51 );
    ILabel = (struct LabelIFace *)IExec->GetInterface( LabelBase, "main", 1, NULL );
    if ( ILabel == NULL )
    {
        return FALSE;
    }

    LayoutBase = IExec->OpenLibrary( "gadgets/layout.gadget", 51 );
    ILayout = (struct LayoutIFace *)IExec->GetInterface( LayoutBase, "main", 1, NULL );
    if ( ILayout == NULL )
    {
        return FALSE;
    }

    return TRUE;
}

void CloseGUILibraries()
{
    if (IGetScreenMode) IExec->DropInterface((struct Interface*)IGetScreenMode);
    IGetScreenMode = NULL;
    if (GetScreenModeBase) IExec->CloseLibrary(GetScreenModeBase);
    GetScreenModeBase = NULL;

    if (ILabel) IExec->DropInterface((struct Interface*)ILabel);
    ILabel = NULL;
    if (LabelBase) IExec->CloseLibrary(LabelBase);
    LabelBase = NULL;

    if (ILayout) IExec->DropInterface((struct Interface*)ILayout);
    ILayout = NULL;
    if (LayoutBase) IExec->CloseLibrary(LayoutBase);
    LayoutBase = NULL;
}

GLboolean GUIMGLCallback(MGLScreenMode* mode)
{
    if (mode->id == temp_mode_ID)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL ScreenmodeCallback(struct Hook* hook, VOID* object, ULONG modeid)
{
    temp_mode_ID = modeid;
    GUIModeID = IMiniGL->GetSupportedScreenModes(GUIMGLCallback);
    if (GUIModeID != MGL_SM_BESTMODE)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL MakeGUI(struct BlankerData* bd, struct BlankerPrefsWindowSetup* bpws)
{
    BOOL result = FALSE;

    if ( bd && bpws )
    {
        ScreenmodeHook = IExec->AllocSysObjectTags(ASOT_HOOK, ASOHOOK_Entry, ScreenmodeCallback, TAG_DONE);

        bpws->winWidth      = 300;
        bpws->winHeight     = 140;
        bpws->eventHook     = (APTR)&GUIEventHook;

        bpws->rootLayout = LayoutObject,
                LAYOUT_Orientation, LAYOUT_VERTICAL,
                LAYOUT_SpaceInner, TRUE,
                LAYOUT_SpaceOuter, TRUE,
                LAYOUT_DeferLayout, TRUE,

                /* Screen mode requester */
                LAYOUT_AddChild, screenmode_requester =
                GetScreenModeObject,
                    GA_RelVerify, TRUE,
                    GA_ID, GA_ScreenMode,
                    GETSCREENMODE_FilterFunc, ScreenmodeHook,
                GetScreenModeEnd,

                CHILD_Label,
                LabelObject,
                    LABEL_Justification, LABEL_LEFT,
                    LABEL_Text, "Screen Mode: ",
                LabelEnd,
             LayoutEnd; /* End Main Layout */
        result = TRUE;
    }

    return( result );
}

void DestroyGUI()
{
    IExec->FreeSysObject(ASOT_HOOK, ScreenmodeHook);
}

void GUIEventFunc(struct Hook* hook, struct BlankerModuleIFace* Self, struct BlankerPrefsWinGUIEvent* event )
{
    struct BlankerData *bd;
    uint32 gadgetID;
    uint32 refetch;
    uint32 attr = 0;

    if (( Self != NULL ) && ( event != NULL ))
    {
        bd = (struct BlankerData *)((uint32)Self - Self->Data.NegativeSize);
        gadgetID = event->result & WMHI_GADGETMASK;

        refetch = bd->refetchSettings;

        switch( gadgetID )
        {
              case GA_ScreenMode:
                RequestScreenMode(screenmode_requester, bd->WinInfo.window);
                IIntuition->GetAttr(GETSCREENMODE_DisplayID, screenmode_requester, &attr);
                bd->screenmodeID = attr;
                break;
              default:
                break;
        } /* switch gadget */

        bd->refetchSettings = refetch;
    }
}

void UpdateWindowSettings(struct BlankerData* bd)
{
    struct TagItem tags[] = { {0,0}, {TAG_DONE}};

    struct Window* gui_window = bd->WinInfo.window;

    tags[0].ti_Tag = GETSCREENMODE_DisplayID;
    tags[0].ti_Data = bd->screenmodeID;
    IIntuition->SetGadgetAttrsA(screenmode_requester, gui_window, NULL, tags);
}
