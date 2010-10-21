#define MINIGL 1
#include <GL/gl.h>
#include <utility/tagitem.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/minigl.h>

// Reaction Stuff
#include <classes/window.h>
#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>
#include <gadgets/button.h>
#include <gadgets/layout.h>
#include <gadgets/chooser.h>
#include <gadgets/integer.h>
#include <gadgets/getscreenmode.h>
#include <gadgets/slider.h>
#include <gadgets/space.h>
#include <images/label.h>
#include <intuition/icclass.h>
#include <proto/Picasso96API.h>
#include <proto/button.h>
#include <proto/checkbox.h>
#include <proto/chooser.h>
#include <proto/getscreenmode.h>
#include <proto/integer.h>
#include <proto/label.h>
#include <proto/layout.h>
#include <proto/slider.h>
#include <proto/space.h>

#include "blanker.h"
#include "GLBlanker_GUI.h"

#define GA_Speed		   0x1003
#define GA_Speed_Integer   0x1004
#define GA_ScreenMode	   0x1006
#define GA_Random		      0x1007
#define GA_Wander		     0x1008
#define GA_Spin		     0x1009

struct Library *        	ButtonBase     		= NULL;
struct Library *			CheckBoxBase		= NULL;
struct Library *			ChooserBase			= NULL;
struct Library *			GetScreenModeBase	= NULL;
struct Library *			IntegerBase			= NULL;
struct Library *        	LabelBase   		= NULL;
struct Library *        	LayoutBase   		= NULL;
struct Library *        	SliderBase     		= NULL;
struct Library *			SpaceBase			= NULL;

struct ButtonIFace *    	IButton         	= NULL;
struct CheckBoxIFace *		ICheckBox			= NULL;
struct ChooserIFace *		IChooser			= NULL;
struct GetScreenModeIFace * IGetScreenMode		= NULL;
struct IntegerIFace *		IInteger			= NULL;
struct LabelIFace *    		ILabel          	= NULL;
struct LayoutIFace *    	ILayout         	= NULL;
struct SliderIFace *		ISlider				= NULL;
struct SpaceIFace *			ISpace				= NULL;

struct Gadget *speed_slider = NULL, *speed_integer = NULL;
struct Gadget *screenmode_requester = NULL;
struct Gadget *random_checkbox = NULL, *wander_checkbox = NULL, *spin_checkbox = NULL;

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

const struct Hook GUIEventHook = {{ NULL, NULL }, (HOOKFUNC)GUIEventFunc, NULL, NULL };

/// OpenGUILibraries()
uint32 OpenGUILibraries()
{
	ButtonBase = IExec->OpenLibrary( "gadgets/button.gadget", 51 );
	IButton = (struct ButtonIFace *)IExec->GetInterface( ButtonBase, "main", 1, NULL );
	if ( IButton == NULL )
	{
		return FALSE;
	}

	CheckBoxBase = IExec->OpenLibrary("gadgets/checkbox.gadget", 51);
	ICheckBox = (struct CheckBoxIFace *)IExec->GetInterface( CheckBoxBase, "main", 1, NULL);
	if (ICheckBox == NULL)
	{
		return FALSE;
	}

	ChooserBase = IExec->OpenLibrary("gadgets/chooser.gadget", 51);
	IChooser = (struct ChooserIFace *)IExec->GetInterface( ChooserBase, "main", 1, NULL);
	if (IChooser == NULL)
	{
		return FALSE;
	}

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
///

/// CloseGUILibraries()
void CloseGUILibraries()
{
	if (IButton) IExec->DropInterface((struct Interface*)IButton);
	IButton = NULL;
	if (ButtonBase) IExec->CloseLibrary(ButtonBase);
	ButtonBase = NULL;

	if (ICheckBox) IExec->DropInterface((struct Interface*)ICheckBox);
	ICheckBox = NULL;
	if (CheckBoxBase) IExec->CloseLibrary(CheckBoxBase);
	CheckBoxBase = NULL;

	if (IChooser) IExec->DropInterface((struct Interface*)IChooser);
	IChooser = NULL;
	if (ChooserBase) IExec->CloseLibrary(ChooserBase);
	ChooserBase = NULL;

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
///

/// MGLCallBack
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
///

/// MakeGUI(struct BlankerDate* bd, struct BlankerPrefsWindowSetup* bpws)
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
					SliderObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Speed,
						SLIDER_Level, 100,
						SLIDER_Min, 20,
						SLIDER_Max, 300,
						SLIDER_Orientation, SLIDER_HORIZONTAL,
						ICA_MAP, sl_2_int_map,
					SliderEnd,

                    /* Speed Integer */
					LAYOUT_AddChild, speed_integer =
					IntegerObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Speed_Integer,
						INTEGER_Number, 100,
						INTEGER_Minimum, 0,
						INTEGER_Maximum, 300,
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

                /* Check boxes */
				LAYOUT_AddChild,
				LayoutObject,
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,
					LAYOUT_HorizAlignment, LALIGN_CENTER,
					LAYOUT_EvenSize, TRUE,

					/* Random */
					LAYOUT_AddChild, random_checkbox =
					CheckBoxObject,
						GA_ID, GA_Random,
						GA_Text, "_Random",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

					/* Wander */
					LAYOUT_AddChild, wander_checkbox =
					CheckBoxObject,
						GA_ID, GA_Wander,
						GA_Text, "_Wander",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

					/* Spin */
					LAYOUT_AddChild, spin_checkbox =
					CheckBoxObject,
						GA_ID, GA_Spin,
						GA_Text, "_Spin",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

				LayoutEnd, /* End Check Boxes */

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

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

		if (bpws->rootLayout != NULL)
		{
			ica_targets[0].ti_Data = (ULONG)speed_integer;
			IIntuition->SetAttrsA(speed_slider, ica_targets);
			ica_targets[0].ti_Data = (ULONG)speed_slider;
			IIntuition->SetAttrsA(speed_integer, ica_targets);
		}

		result = TRUE;
	}

	return( result );
}
///

///DestroyGUI()
void DestroyGUI()
{
	IExec->FreeSysObject(ASOT_HOOK, ScreenmodeHook);
}
///

/// GUI Event Func(struct Hook* hook, struct BlankerModuleIFace* Self, struct BlankerPrefsWinGUIEvent* event)
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
				IIntuition->GetAttr(INTEGER_Number, speed_integer, &attr);
				bd->speed = attr;
				refetch |= TRUE;
				break;
			case GA_ScreenMode:
				RequestScreenMode((Object*)screenmode_requester, bd->WinInfo.window);
				IIntuition->GetAttr(GETSCREENMODE_DisplayID, screenmode_requester, &attr);
				bd->screenmodeID = attr;
				break;
			case GA_Random:
				IIntuition->GetAttr(GA_Selected, random_checkbox, &attr);
				bd->random = (BOOL)attr;
				refetch |= TRUE;
				break;
			case GA_Wander:
				IIntuition->GetAttr(GA_Selected, wander_checkbox, &attr);
				bd->do_wander = (BOOL)attr;
				refetch |= TRUE;
				break;
			case GA_Spin:
				IIntuition->GetAttr(GA_Selected, spin_checkbox, &attr);
				bd->do_spin = (BOOL)attr;
				refetch |= TRUE;
				break;
			default:
				break;
		} /* switch gadget */

	    bd->refetchSettings = refetch;
    }
}
///

/// UpdateWindowSettings(struct BlankerData* bd)
void UpdateWindowSettings(struct BlankerData* bd)
{
	struct TagItem tags[] = { {0,0}, {TAG_DONE}};

	struct Window* gui_window = bd->WinInfo.window;

	tags[0].ti_Tag  = INTEGER_Number;
	tags[0].ti_Data = bd->speed;
    IIntuition->SetGadgetAttrsA(speed_integer, gui_window, NULL, tags);

    tags[0].ti_Tag  = SLIDER_Level;
	tags[0].ti_Data = bd->speed;
    IIntuition->SetGadgetAttrsA(speed_slider, gui_window, NULL, tags);

	tags[0].ti_Tag = GETSCREENMODE_DisplayID;
	tags[0].ti_Data = bd->screenmodeID;
	IIntuition->RefreshSetGadgetAttrsA(screenmode_requester, gui_window, NULL, tags);

	tags[0].ti_Tag  = GA_Selected;
	tags[0].ti_Data = (ULONG)bd->random;
	IIntuition->RefreshSetGadgetAttrsA(random_checkbox, gui_window, NULL, tags);

	tags[0].ti_Data = (ULONG)bd->do_wander;
	IIntuition->RefreshSetGadgetAttrsA(wander_checkbox, gui_window, NULL, tags);

	tags[0].ti_Data = (ULONG)bd->do_spin;
	IIntuition->RefreshSetGadgetAttrsA(spin_checkbox, gui_window, NULL, tags);
}
///

/// ResetSettingsToDefault(struct BlankerData *bd)
void ResetSettingsToDefault(struct BlankerData *bd)
{
	uint32 id;

    id = IP96->p96BestModeIDTags(
		P96BIDTAG_NominalWidth,	    640,
		P96BIDTAG_NominalHeight,    480,
	    P96BIDTAG_Depth,		    16,
	    P96BIDTAG_FormatsForbidden, RGBFB_CLUT,
	    TAG_END
    );

	bd->screenmodeID	= id;
	bd->random = TRUE;
	bd->do_spin = TRUE;
	bd->do_wander = TRUE;
	bd->speed = 20;
}
///

