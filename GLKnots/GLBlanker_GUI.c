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

#define GA_Segments          0x1001
#define GA_Segments_Integer  0x1002
#define GA_Thickness         0x1003
#define GA_Thickness_Integer 0x1004
#define GA_Duration          0x1005
#define GA_Duration_Integer  0x1006
#define GA_ScreenMode        0x1008
#define GA_SpinX             0x1009
#define GA_SpinY             0x100a
#define GA_SpinZ             0x100b
#define GA_Wander            0x100c

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

struct Gadget *Segments_slider = NULL, *Segments_integer = NULL;
struct Gadget *Thickness_slider = NULL, *Thickness_integer = NULL;
struct Gadget *Duration_slider = NULL, *Duration_integer = NULL;
struct Gadget *screenmode_requester = NULL;
struct Gadget *SpinX_checkbox = NULL, *SpinY_checkbox = NULL, *SpinZ_checkbox = NULL, *Wander_checkbox = NULL;

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

				/* Segments Slider and Integer */
				LAYOUT_AddChild,
				LayoutObject,
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,
					/* Segments Slider */
					LAYOUT_AddChild, Segments_slider =
					(struct Gadget*)SliderObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Segments,
						SLIDER_Level, 400,
						SLIDER_Min, 200,
						SLIDER_Max, 1000,
						SLIDER_Orientation, SLIDER_HORIZONTAL,
						ICA_MAP, sl_2_int_map,
					SliderEnd,

					/* Segments Integer */
					LAYOUT_AddChild, Segments_integer =
					(struct Gadget*)IntegerObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Segments_Integer,
						INTEGER_Number, 400,
						INTEGER_Minimum, 200,
						INTEGER_Maximum, 1000,
						INTEGER_Arrows, FALSE,
						ICA_MAP, int_2_sl_map,
					IntegerEnd,
				LayoutEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Segments: ",
				LabelEnd,

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

				/* Duration Slider and Integer */
				LAYOUT_AddChild,
				LayoutObject,
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,
                    /* Speed Slider */
					LAYOUT_AddChild, Duration_slider =
					(struct Gadget*)SliderObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Duration,
						SLIDER_Level, 8,
						SLIDER_Min, 4,
						SLIDER_Max, 20,
						SLIDER_Orientation, SLIDER_HORIZONTAL,
						ICA_MAP, sl_2_int_map,
					SliderEnd,

					/* Duration Integer */
					LAYOUT_AddChild, Duration_integer =
					(struct Gadget*)IntegerObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Duration_Integer,
						INTEGER_Number, 8,
						INTEGER_Minimum, 4,
						INTEGER_Maximum, 20,
						INTEGER_Arrows, FALSE,
						ICA_MAP, int_2_sl_map,
					IntegerEnd,
				LayoutEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Duration: ",
				LabelEnd,

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

                /* Check boxes */
				LAYOUT_AddChild,
				LayoutObject,
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,
					LAYOUT_HorizAlignment, LALIGN_CENTER,
					LAYOUT_EvenSize, TRUE,

                    /* Fog */
					LAYOUT_AddChild, SpinX_checkbox =
					(struct Gadget*)CheckBoxObject,
						GA_ID, GA_SpinX,
						GA_Text, "Spin_X",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

                    /* Wave */
					LAYOUT_AddChild, SpinY_checkbox =
					(struct Gadget*)CheckBoxObject,
						GA_ID, GA_SpinY,
						GA_Text, "Spin_Y",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

                    /* Rotate */
					LAYOUT_AddChild, SpinZ_checkbox =
					(struct Gadget*)CheckBoxObject,
						GA_ID, GA_SpinZ,
						GA_Text, "Spin_Z",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

                    /* Invert alpha */
					LAYOUT_AddChild, Wander_checkbox =
					(struct Gadget*)CheckBoxObject,
						GA_ID, GA_Wander,
						GA_Text, "_Wander",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

				LayoutEnd, /* End Check Boxes */

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
             LayoutEnd; /* End Main Layout */

		if (bpws->rootLayout != NULL)
		{
			ica_targets[0].ti_Data = (ULONG)Segments_integer;
			IIntuition->SetAttrsA((Object*)Segments_slider, ica_targets);
			ica_targets[0].ti_Data = (ULONG)Segments_slider;
			IIntuition->SetAttrsA((Object*)Segments_integer, ica_targets);

			ica_targets[0].ti_Data = (ULONG)Duration_integer;
			IIntuition->SetAttrsA((Object*)Duration_slider, ica_targets);
			ica_targets[0].ti_Data = (ULONG)Duration_slider;
			IIntuition->SetAttrsA((Object*)Duration_integer, ica_targets);
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
			case GA_Segments:
			case GA_Segments_Integer:
				IIntuition->GetAttr(INTEGER_Number, (Object*)Segments_integer, &attr);
				bd->segments = attr;
				refetch |= TRUE;
				break;
			case GA_Duration:
			case GA_Duration_Integer:
				IIntuition->GetAttr(INTEGER_Number, (Object*)Duration_integer, &attr);
				bd->duration = attr;
				refetch |= TRUE;
				break;
			case GA_ScreenMode:
				RequestScreenMode((Object*)screenmode_requester, bd->WinInfo.window);
				IIntuition->GetAttr(GETSCREENMODE_DisplayID, (Object*)screenmode_requester, &attr);
				bd->screenmodeID = attr;
				break;
			case GA_SpinX:
				IIntuition->GetAttr(GA_Selected, (Object*)SpinX_checkbox, &attr);
				bd->spinX = (BOOL)attr;
				refetch |= TRUE;
				break;
			case GA_SpinY:
				IIntuition->GetAttr(GA_Selected, (Object*)SpinY_checkbox, &attr);
				bd->spinY = (BOOL)attr;
				refetch |= TRUE;
				break;
			case GA_SpinZ:
				IIntuition->GetAttr(GA_Selected, (Object*)SpinZ_checkbox, &attr);
				bd->spinZ = (BOOL)attr;
				refetch |= TRUE;
				break;
			case GA_Wander:
				IIntuition->GetAttr(GA_Selected, (Object*)Wander_checkbox, &attr);
				bd->wander = (BOOL)attr;
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
	tags[0].ti_Data = bd->segments;
	IIntuition->SetGadgetAttrsA(Segments_integer, gui_window, NULL, tags);

	tags[0].ti_Data = bd->duration;
	IIntuition->SetGadgetAttrsA(Duration_integer, gui_window, NULL, tags);

    tags[0].ti_Tag  = SLIDER_Level;
	tags[0].ti_Data = bd->segments;
	IIntuition->SetGadgetAttrsA(Segments_slider, gui_window, NULL, tags);

	tags[0].ti_Data = bd->duration;
	IIntuition->SetGadgetAttrsA(Duration_slider, gui_window, NULL, tags);

    tags[0].ti_Tag = GETSCREENMODE_DisplayID;
	tags[0].ti_Data = bd->screenmodeID;
	IIntuition->RefreshSetGadgetAttrsA(screenmode_requester, gui_window, NULL, tags);

	tags[0].ti_Tag  = GA_Selected;
	tags[0].ti_Data = (ULONG)bd->spinX;
	IIntuition->RefreshSetGadgetAttrsA(SpinX_checkbox, gui_window, NULL, tags);

	tags[0].ti_Data = (ULONG)bd->spinY;
	IIntuition->RefreshSetGadgetAttrsA(SpinY_checkbox, gui_window, NULL, tags);

	tags[0].ti_Data = (ULONG)bd->spinZ;
	IIntuition->RefreshSetGadgetAttrsA(SpinZ_checkbox, gui_window, NULL, tags);

	tags[0].ti_Data = (ULONG)bd->wander;
	IIntuition->RefreshSetGadgetAttrsA(Wander_checkbox, gui_window, NULL, tags);

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
	bd->segments		= 800;
	bd->duration		= 8;
	bd->spinX			= TRUE;
	bd->spinY			= TRUE;
	bd->spinZ			= TRUE;
	bd->wander			= TRUE;
	bd->refetchSettings	= TRUE;
}
///

