#include <utility/tagitem.h>

#include <proto/blankermodule.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/minigl.h>

// Reaction Stuff
#include <classes/window.h>
#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>
#include <gadgets/layout.h>
#include <gadgets/integer.h>
#include <gadgets/getscreenmode.h>
#include <gadgets/slider.h>
#include <gadgets/space.h>
#include <images/label.h>
#include <intuition/icclass.h>
#include <proto/getscreenmode.h>
#include <proto/integer.h>
#include <proto/label.h>
#include <proto/layout.h>
#include <proto/slider.h>
#include <proto/space.h>

#include "blanker.h"

#define GA_Speed                0x1000
#define GA_Speed_Integer        0x1001
#define GA_Balls	            0x1002
#define GA_Balls_Integer        0x1003
#define GA_BallSize	       	    0x1004
#define GA_BallSize_Integer     0x1005
#define GA_Explosion            0x1006
#define GA_Explosion_Integer    0x1007
#define GA_Decay	            0x1008
#define GA_Decay_Integer        0x1009
#define GA_ScreenMode           0x100a

struct Library* GetScreenModeBase   = NULL;
struct Library* IntegerBase         = NULL;
struct Library* LabelBase           = NULL;
struct Library* LayoutBase          = NULL;
struct Library* SliderBase     	    = NULL;
struct Library* SpaceBase           = NULL;

struct GetScreenModeIFace*  IGetScreenMode  = NULL;
struct IntegerIFace*        IInteger        = NULL;
struct LabelIFace*          ILabel          = NULL;
struct LayoutIFace*         ILayout         = NULL;
struct SliderIFace*         ISlider         = NULL;
struct SpaceIFace*          ISpace          = NULL;

struct Gadget *speed_slider = NULL, *speed_integer = NULL;
struct Gadget *balls_slider = NULL, *balls_integer = NULL;
struct Gadget *ballsize_slider = NULL, *ballsize_integer = NULL;
struct Gadget *explosion_slider = NULL, *explosion_integer = NULL;
struct Gadget *decay_slider = NULL, *decay_integer = NULL;
struct Gadget *screenmode_requester = NULL;

struct TagItem sl_2_int_map[] =
{
    {SLIDER_Level, INTEGER_Number},
    {TAG_DONE}
};

struct TagItem int_2_sl_map[] =
{
    {INTEGER_Number, SLIDER_Level},
    {TAG_DONE}
};

struct TagItem ica_targets[] =
{
    {ICA_TARGET, 0},
    {TAG_DONE}
};

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

    IntegerBase = IExec->OpenLibrary("gadgets/integer.gadget", 51);
    IInteger = (struct IntegerIFace *)IExec->GetInterface( IntegerBase, "main", 1, NULL);
    if (IInteger == NULL)
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

    SliderBase = IExec->OpenLibrary("gadgets/slider.gadget", 51);
    ISlider = (struct SliderIFace *)IExec->GetInterface( SliderBase, "main", 1, NULL);
    if (ISlider == NULL)
    {
        return FALSE;
    }

    SpaceBase = IExec->OpenLibrary("gadgets/space.gadget", 51);
    ISpace = (struct SpaceIFace *)IExec->GetInterface( SpaceBase, "main", 1, NULL);
    if (ISpace == NULL)
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

    if (IInteger) IExec->DropInterface((struct Interface*)IInteger);
    IInteger = NULL;
    if (IntegerBase) IExec->CloseLibrary(IntegerBase);
    IntegerBase = NULL;

    if (ILabel) IExec->DropInterface((struct Interface*)ILabel);
    ILabel = NULL;
    if (LabelBase) IExec->CloseLibrary(LabelBase);
    LabelBase = NULL;

    if (ILayout) IExec->DropInterface((struct Interface*)ILayout);
    ILayout = NULL;
    if (LayoutBase) IExec->CloseLibrary(LayoutBase);
    LayoutBase = NULL;

    if (ISlider) IExec->DropInterface((struct Interface*)ISlider);
    ISlider = NULL;
    if (SliderBase) IExec->CloseLibrary(SliderBase);
    SliderBase = NULL;

    if (ISpace) IExec->DropInterface((struct Interface*)ISpace);
    ISpace = NULL;
    if (SpaceBase) IExec->CloseLibrary(SpaceBase);
    SpaceBase = NULL;
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

                /* Speed Slider and Integer */
                LAYOUT_AddChild,
                LayoutObject,
                    LAYOUT_Orientation, LAYOUT_HORIZONTAL,
                    /* Speed Slider */
                    LAYOUT_AddChild, speed_slider =
                    (struct Gadget*)SliderObject,
                        GA_RelVerify, TRUE,
                        GA_ID, GA_Speed,
                        SLIDER_Level, 40,
                        SLIDER_Min, 0,
                        SLIDER_Max, 100,
                        SLIDER_Orientation, SLIDER_HORIZONTAL,
                        ICA_MAP, sl_2_int_map,
                    SliderEnd,

                    /* Speed Integer */
                    LAYOUT_AddChild, speed_integer =
                    (struct Gadget*)IntegerObject,
                        GA_RelVerify, TRUE,
                        GA_ID, GA_Speed_Integer,
                        INTEGER_Number, 40,
                        INTEGER_Minimum, 0,
                        INTEGER_Maximum, 100,
                        INTEGER_Arrows, FALSE,
                        ICA_MAP, int_2_sl_map,
                    IntegerEnd,
                LayoutEnd,

                CHILD_Label,
                LabelObject,
                    LABEL_Justification, LABEL_LEFT,
                    LABEL_Text, "Speed: ",
                LabelEnd,

                LAYOUT_AddChild, SpaceObject, SpaceEnd,

                /* Balls Slider and Integer */
                LAYOUT_AddChild,
                LayoutObject,
                    LAYOUT_Orientation, LAYOUT_HORIZONTAL,
                    /* Balls Slider */
                    LAYOUT_AddChild, balls_slider =
                    (struct Gadget*)SliderObject,
                        GA_RelVerify, TRUE,
                        GA_ID, GA_Balls,
                        SLIDER_Level, 25,
                        SLIDER_Min, 2,
                        SLIDER_Max, 100,
                        SLIDER_Orientation, SLIDER_HORIZONTAL,
                        ICA_MAP, sl_2_int_map,
                    SliderEnd,

                    /* Balls Integer */
                    LAYOUT_AddChild, balls_integer =
                    (struct Gadget*)IntegerObject,
                        GA_RelVerify, TRUE,
                        GA_ID, GA_Balls_Integer,
                        INTEGER_Number, 25,
                        INTEGER_Minimum, 2,
                        INTEGER_Maximum, 100,
                        INTEGER_Arrows, FALSE,
                        ICA_MAP, int_2_sl_map,
                    IntegerEnd,
                LayoutEnd,

                CHILD_Label,
                LabelObject,
                    LABEL_Justification, LABEL_LEFT,
                    LABEL_Text, "Balls: ",
                LabelEnd,

                LAYOUT_AddChild, SpaceObject, SpaceEnd,

                /* BallSize Slider and Integer */
                LAYOUT_AddChild,
                LayoutObject,
                    LAYOUT_Orientation, LAYOUT_HORIZONTAL,
                    /* Balls Slider */
                    LAYOUT_AddChild, ballsize_slider =
                    (struct Gadget*)SliderObject,
                        GA_RelVerify, TRUE,
                        GA_ID, GA_BallSize,
                        SLIDER_Level, 20,
                        SLIDER_Min, 10,
                        SLIDER_Max, 50,
                        SLIDER_Orientation, SLIDER_HORIZONTAL,
                        ICA_MAP, sl_2_int_map,
                    SliderEnd,

                    /* BallSize Integer */
                    LAYOUT_AddChild, ballsize_integer =
                    (struct Gadget*)IntegerObject,
                        GA_RelVerify, TRUE,
                        GA_ID, GA_BallSize_Integer,
                        INTEGER_Number, 20,
                        INTEGER_Minimum, 10,
                        INTEGER_Maximum, 50,
                        INTEGER_Arrows, FALSE,
                        ICA_MAP, int_2_sl_map,
                    IntegerEnd,
                LayoutEnd,

                CHILD_Label,
                LabelObject,
                    LABEL_Justification, LABEL_LEFT,
                    LABEL_Text, "BallSize: ",
                LabelEnd,

                LAYOUT_AddChild, SpaceObject, SpaceEnd,

                /* Explosion Slider and Integer */
                LAYOUT_AddChild,
                LayoutObject,
                    LAYOUT_Orientation, LAYOUT_HORIZONTAL,
                    /* Explosion Slider */
                    LAYOUT_AddChild, explosion_slider =
                    (struct Gadget*)SliderObject,
                        GA_RelVerify, TRUE,
                        GA_ID, GA_Explosion,
                        SLIDER_Level, 15,
                        SLIDER_Min, 10,
                        SLIDER_Max, 100,
                        SLIDER_Orientation, SLIDER_HORIZONTAL,
                        ICA_MAP, sl_2_int_map,
                    SliderEnd,

                    /* Balls Integer */
                    LAYOUT_AddChild, explosion_integer =
                    (struct Gadget*)IntegerObject,
                        GA_RelVerify, TRUE,
                        GA_ID, GA_Explosion_Integer,
                        INTEGER_Number, 15,
                        INTEGER_Minimum, 10,
                        INTEGER_Maximum, 100,
                        INTEGER_Arrows, FALSE,
                        ICA_MAP, int_2_sl_map,
                    IntegerEnd,
                LayoutEnd,

                CHILD_Label,
                LabelObject,
                    LABEL_Justification, LABEL_LEFT,
                    LABEL_Text, "Explosion: ",
                LabelEnd,

                LAYOUT_AddChild, SpaceObject, SpaceEnd,

                /* Decay Slider and Integer */
                LAYOUT_AddChild,
                LayoutObject,
                    LAYOUT_Orientation, LAYOUT_HORIZONTAL,
                    /* Decay Slider */
                    LAYOUT_AddChild, decay_slider =
                    (struct Gadget*)SliderObject,
                        GA_RelVerify, TRUE,
                        GA_ID, GA_Decay,
                        SLIDER_Level, 10,
                        SLIDER_Min, 1,
                        SLIDER_Max, 100,
                        SLIDER_Orientation, SLIDER_HORIZONTAL,
                        ICA_MAP, sl_2_int_map,
                    SliderEnd,

                    /* Decay Integer */
                    LAYOUT_AddChild, decay_integer =
                    (struct Gadget*)IntegerObject,
                        GA_RelVerify, TRUE,
                        GA_ID, GA_Decay_Integer,
                        INTEGER_Number, 10,
                        INTEGER_Minimum, 1,
                        INTEGER_Maximum, 100,
                        INTEGER_Arrows, FALSE,
                        ICA_MAP, int_2_sl_map,
                    IntegerEnd,
                LayoutEnd,

                CHILD_Label,
                LabelObject,
                    LABEL_Justification, LABEL_LEFT,
                    LABEL_Text, "Decay: ",
                LabelEnd,

                LAYOUT_AddChild, SpaceObject, SpaceEnd,

                /* Screen mode requester */
                LAYOUT_AddChild, screenmode_requester =
                (struct Gadget*)GetScreenModeObject,
                    GA_RelVerify, TRUE,
                    GA_ID, GA_ScreenMode,
                    GETSCREENMODE_FilterFunc, ScreenmodeHook,
                GetScreenModeEnd,

                CHILD_Label,
                LabelObject,
                    LABEL_Justification, LABEL_LEFT,
                    LABEL_Text, "Screen Mode: ",
                LabelEnd,

                LAYOUT_AddChild, SpaceObject, SpaceEnd,
             LayoutEnd; /* End Main Layout */

        if (bpws->rootLayout != NULL)
        {
            ica_targets[0].ti_Data = (ULONG)speed_integer;
            IIntuition->SetAttrsA((Object*)speed_slider, ica_targets);
            ica_targets[0].ti_Data = (ULONG)speed_slider;
            IIntuition->SetAttrsA((Object*)speed_integer, ica_targets);

            ica_targets[0].ti_Data = (ULONG)balls_integer;
            IIntuition->SetAttrsA((Object*)balls_slider, ica_targets);
            ica_targets[0].ti_Data = (ULONG)balls_slider;
            IIntuition->SetAttrsA((Object*)balls_integer, ica_targets);

            ica_targets[0].ti_Data = (ULONG)ballsize_integer;
            IIntuition->SetAttrsA((Object*)ballsize_slider, ica_targets);
            ica_targets[0].ti_Data = (ULONG)ballsize_slider;
            IIntuition->SetAttrsA((Object*)ballsize_integer, ica_targets);

            ica_targets[0].ti_Data = (ULONG)explosion_integer;
            IIntuition->SetAttrsA((Object*)explosion_slider, ica_targets);
            ica_targets[0].ti_Data = (ULONG)explosion_slider;
            IIntuition->SetAttrsA((Object*)explosion_integer, ica_targets);

            ica_targets[0].ti_Data = (ULONG)decay_integer;
            IIntuition->SetAttrsA((Object*)decay_slider, ica_targets);
            ica_targets[0].ti_Data = (ULONG)decay_slider;
            IIntuition->SetAttrsA((Object*)decay_integer, ica_targets);
        }

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
            case GA_Speed:
            case GA_Speed_Integer:
                IIntuition->GetAttr(INTEGER_Number, (Object*)speed_integer, &attr);
                bd->speed = attr / 100.0f;
                refetch |= TRUE;
                break;
            case GA_Balls:
            case GA_Balls_Integer:
                IIntuition->GetAttr(INTEGER_Number, (Object*)balls_integer, &attr);
                bd->balls = attr;
                refetch |= TRUE;
                break;
            case GA_BallSize:
            case GA_BallSize_Integer:
                IIntuition->GetAttr(INTEGER_Number, (Object*)ballsize_integer, &attr);
                bd->ballSize = attr / 10.0f;
                refetch |= TRUE;
                break;
            case GA_Explosion:
            case GA_Explosion_Integer:
                IIntuition->GetAttr(INTEGER_Number, (Object*)explosion_integer, &attr);
                bd->explosion = attr / 1.0f;
                refetch |= TRUE;
                break;
            case GA_Decay:
            case GA_Decay_Integer:
                IIntuition->GetAttr(INTEGER_Number, (Object*)decay_integer, &attr);
                bd->decay = attr / 100.0f;
                refetch |= TRUE;
                break;
            case GA_ScreenMode:
                RequestScreenMode((Object*)screenmode_requester, bd->WinInfo.window);
                IIntuition->GetAttr(GETSCREENMODE_DisplayID, (Object*)screenmode_requester, &attr);
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

    tags[0].ti_Tag  = INTEGER_Number;
    tags[0].ti_Data = (int)(bd->speed * 100.0f);
    IIntuition->SetGadgetAttrsA(speed_integer, gui_window, NULL, tags);
    tags[0].ti_Data = bd->balls;
    IIntuition->SetGadgetAttrsA(balls_integer, gui_window, NULL, tags);
    tags[0].ti_Data = (int)(bd->ballSize * 10.0f);
    IIntuition->SetGadgetAttrsA(ballsize_integer, gui_window, NULL, tags);
    tags[0].ti_Data = (int)(bd->explosion * 1.0f);
    IIntuition->SetGadgetAttrsA(explosion_integer, gui_window, NULL, tags);
    tags[0].ti_Data = (int)(bd->decay * 100.0f);
    IIntuition->SetGadgetAttrsA(decay_integer, gui_window, NULL, tags);

    tags[0].ti_Tag  = SLIDER_Level;
    tags[0].ti_Data = (int)(bd->speed * 100.0f);
    IIntuition->SetGadgetAttrsA(speed_slider, gui_window, NULL, tags);
    tags[0].ti_Data = bd->balls;
    IIntuition->SetGadgetAttrsA(balls_slider, gui_window, NULL, tags);
    tags[0].ti_Data = (int)(bd->ballSize * 10.0f);
    IIntuition->SetGadgetAttrsA(ballsize_slider, gui_window, NULL, tags);
    tags[0].ti_Data = (int)(bd->explosion * 1.0f);
    IIntuition->SetGadgetAttrsA(explosion_slider, gui_window, NULL, tags);
    tags[0].ti_Data = (int)(bd->decay * 100.0f);
    IIntuition->SetGadgetAttrsA(decay_slider, gui_window, NULL, tags);

    tags[0].ti_Tag = GETSCREENMODE_DisplayID;
    tags[0].ti_Data = bd->screenmodeID;
    IIntuition->RefreshSetGadgetAttrsA(screenmode_requester, gui_window, NULL, tags);
}
