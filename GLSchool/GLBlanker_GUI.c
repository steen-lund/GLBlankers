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

#define GA_NFish				0x1001
#define GA_NFish_Integer		0x1002
#define GA_Fog					0x1003
#define GA_DrawBBox				0x1004
#define GA_DrawGoal				0x1005
#define GA_GoalChgFreq			0x1006
#define GA_GoalChgFreq_Integer	0x1007
#define GA_MaxVel				0x1008
#define GA_MaxVel_Integer		0x1009
#define GA_MinVel				0x100a
#define GA_MinVel_Integer		0x100b
#define GA_AccLimit				0x100c
#define GA_AccLimit_Integer		0x100d
#define GA_DistExp				0x100e
#define GA_DistExp_Integer		0x100f
#define GA_AvoidFact			0x1010
#define GA_AvoidFact_Integer	0x1011
#define GA_MatchFact			0x1012
#define GA_MatchFact_Integer	0x1013
#define GA_CenterFact			0x1014
#define GA_CenterFact_Integer	0x1015
#define GA_TargetFact			0x1016
#define GA_TargetFact_Integer	0x1017
#define GA_MinRadius			0x1018
#define GA_MinRadius_Integer	0x1019
#define GA_Momentum				0x101a
#define GA_Momentum_Integer		0x101b
#define GA_DistComp				0x101c
#define GA_DistComp_Integer		0x101d
#define GA_ScreenMode			0x101e

/// Gadgets
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
///

#define DECLARE_SI(NAME) struct Gadget *NAME##_slider=NULL, *NAME##_integer = NULL;
DECLARE_SI(fish)
DECLARE_SI(goalchg)
DECLARE_SI(maxvel)
DECLARE_SI(minvel)
DECLARE_SI(acclimit)
DECLARE_SI(distexp)
DECLARE_SI(avoidfact)
DECLARE_SI(matchfact)
DECLARE_SI(centerfact)
DECLARE_SI(targetfact)
DECLARE_SI(minradius)
DECLARE_SI(momentum)
DECLARE_SI(distcomp)
#undef DECLARE_SI

struct Gadget *screenmode_requester = NULL;
struct Gadget *fog_checkbox = NULL, *drawbbox_checkbox = NULL, *drawgoal_checkbox = NULL;

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

#define SLIDERINTEGER(GAD, GADID, LEVEL, MIN, MAX, LABEL) \
				LAYOUT_AddChild,                              \
				LayoutObject,                                 \
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,    \
					LAYOUT_AddChild, GAD##_slider =           \
					SliderObject,                             \
						GA_RelVerify, TRUE,                   \
						GA_ID, GADID,                         \
						SLIDER_Level, LEVEL,                  \
						SLIDER_Min, MIN,                      \
						SLIDER_Max, MAX,                      \
						SLIDER_Orientation, SLIDER_HORIZONTAL,\
						ICA_MAP, sl_2_int_map,                \
					SliderEnd,                                \
					LAYOUT_AddChild, GAD##_integer =          \
					IntegerObject,                            \
						GA_RelVerify, TRUE,                   \
						GA_ID, GADID##_Integer,               \
						INTEGER_Number, LEVEL,                \
						INTEGER_Minimum, MIN,                 \
						INTEGER_Maximum, MAX,                 \
						INTEGER_Arrows, FALSE,                \
						ICA_MAP, int_2_sl_map,                \
					IntegerEnd,                               \
				LayoutEnd,                                    \
				CHILD_Label,                                  \
				LabelObject,                                  \
					LABEL_Justification, LABEL_LEFT,          \
					LABEL_Text, LABEL ": ",                   \
				LabelEnd,                                     \
				LAYOUT_AddChild, SpaceObject, SpaceEnd,


				/* Fish Slider and Integer */
				SLIDERINTEGER(fish, GA_NFish, 100, 50, 200, "Fish")

				/* GoalChgFreq Slider and Integer */
				SLIDERINTEGER(goalchg, GA_GoalChgFreq, 50, 2, 200, "GoalChgFreq")

				/* MaxVel Slider and Integer */
				SLIDERINTEGER(maxvel, GA_MaxVel, 70, 50, 100, "MaxVel")

				/* MinVel Slider and Integer */
				SLIDERINTEGER(minvel, GA_MinVel, 10, 1, 50, "MinVel")

				/* AccLimit */
				SLIDERINTEGER(acclimit, GA_AccLimit, 80, 10, 100, "AccLimit")

				SLIDERINTEGER(distexp, GA_DistExp, 22, 0, 100, "DistExp")
				SLIDERINTEGER(avoidfact, GA_AvoidFact, 15, 0, 100, "AvoidFact")
				SLIDERINTEGER(matchfact, GA_MatchFact, 15, 0, 100, "MatchFact")
				SLIDERINTEGER(centerfact, GA_CenterFact, 10, 0, 100, "CenterFact")
				SLIDERINTEGER(targetfact, GA_TargetFact, 80, 0, 100, "TargetFact")
				SLIDERINTEGER(minradius, GA_MinRadius, 30, 0, 100, "MinRadius")
				SLIDERINTEGER(momentum, GA_Momentum, 90, 0, 100, "Momentum")
				SLIDERINTEGER(distcomp, GA_DistComp, 10, 0, 100, "DistComp")
#undef SLIDERINTEGER

				/* Check boxes */
				LAYOUT_AddChild,
				LayoutObject,
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,
					LAYOUT_HorizAlignment, LALIGN_CENTER,
					LAYOUT_EvenSize, TRUE,

                    /* Fog */
					LAYOUT_AddChild, fog_checkbox =
					CheckBoxObject,
						GA_ID, GA_Fog,
						GA_Text, "_Fog",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

                    /* Wave */
					LAYOUT_AddChild, drawbbox_checkbox =
					CheckBoxObject,
						GA_ID, GA_DrawBBox,
						GA_Text, "_DrawBBox",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

                    /* Rotate */
					LAYOUT_AddChild, drawgoal_checkbox =
					CheckBoxObject,
						GA_ID, GA_DrawGoal,
						GA_Text, "Draw_Goal",
						GA_Selected, FALSE,
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
#define BIND(X) \
			ica_targets[0].ti_Data = (ULONG)X##_integer; \
			IIntuition->SetAttrsA(X##_slider, ica_targets); \
			ica_targets[0].ti_Data = (ULONG)X##_slider; \
			IIntuition->SetAttrsA(X##_integer, ica_targets);

			BIND(fish)
			BIND(goalchg)
			BIND(maxvel)
			BIND(minvel)
			BIND(acclimit)
			BIND(distexp)
			BIND(avoidfact)
			BIND(matchfact)
			BIND(centerfact)
			BIND(targetfact)
			BIND(minradius)
			BIND(momentum)
			BIND(distcomp)
#undef BIND
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
#define READ_UI(GADID, GAD, VAR)                                 \
			case GADID:                                          \
			case GADID##_Integer:                                \
				IIntuition->GetAttr(INTEGER_Number, GAD##_integer, &attr); \
				VAR	= attr;                                      \
				refetch |= TRUE;                                 \
				break;

#define READ_UI_FLOAT(GADID, GAD, VAR, FACTOR)                   \
			case GADID:                                          \
			case GADID##_Integer:                                \
				IIntuition->GetAttr(INTEGER_Number, GAD##_integer, &attr); \
				VAR	= (float)attr / FACTOR;                      \
				refetch |= TRUE;                                 \
				break;

			READ_UI(GA_NFish, fish, bd->fish)
			READ_UI(GA_GoalChgFreq, goalchg, bd->goalchgfreq)

			READ_UI_FLOAT(GA_MaxVel, maxvel, bd->maxvel, 10.0f)
			READ_UI_FLOAT(GA_MinVel, minvel, bd->minvel, 10.0f)
			READ_UI_FLOAT(GA_AccLimit, acclimit, bd->acclimit, 10.0f)
			READ_UI_FLOAT(GA_DistExp, distexp, bd->distexp, 10.0f)
			READ_UI_FLOAT(GA_AvoidFact, avoidfact, bd->avoidfact, 10.0f)
			READ_UI_FLOAT(GA_MatchFact, matchfact, bd->matchfact, 100.0f)
			READ_UI_FLOAT(GA_CenterFact, centerfact, bd->centerfact, 100.0f)
			READ_UI_FLOAT(GA_TargetFact, targetfact, bd->targetfact, 1.0f)
			READ_UI_FLOAT(GA_MinRadius, minradius, bd->minradius, 1.0f)
			READ_UI_FLOAT(GA_Momentum, momentum, bd->momentum, 100.0f)
			READ_UI_FLOAT(GA_DistComp, distcomp, bd->distcomp, 1.0f);

#undef READ_UI_FLOAT
#undef READ_UI


			case GA_ScreenMode:
				RequestScreenMode((Object*)screenmode_requester, bd->WinInfo.window);
				IIntuition->GetAttr(GETSCREENMODE_DisplayID, screenmode_requester, &attr);
				bd->screenmodeID = attr;
				break;
			case GA_Fog:
				IIntuition->GetAttr(GA_Selected, fog_checkbox, &attr);
				bd->fog = (BOOL)attr;
				refetch |= TRUE;
				break;
			case GA_DrawBBox:
				IIntuition->GetAttr(GA_Selected, drawbbox_checkbox, &attr);
				bd->drawbbox = (BOOL)attr;
				refetch |= TRUE;
				break;
			case GA_DrawGoal:
				IIntuition->GetAttr(GA_Selected, drawgoal_checkbox, &attr);
				bd->drawgoal = (BOOL)attr;
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

    tags[0].ti_Tag = GETSCREENMODE_DisplayID;
	tags[0].ti_Data = bd->screenmodeID;
	IIntuition->RefreshSetGadgetAttrsA(screenmode_requester, gui_window, NULL, tags);

	tags[0].ti_Tag  = GA_Selected;
	tags[0].ti_Data = (ULONG)bd->fog;
	IIntuition->RefreshSetGadgetAttrsA(fog_checkbox, gui_window, NULL, tags);

	tags[0].ti_Data = (ULONG)bd->drawbbox;
	IIntuition->RefreshSetGadgetAttrsA(drawbbox_checkbox, gui_window, NULL, tags);

	tags[0].ti_Data = (ULONG)bd->drawgoal;
	IIntuition->RefreshSetGadgetAttrsA(drawgoal_checkbox, gui_window, NULL, tags);

#define UPDATE_UI(VAR, GAD)                                              \
	tags[0].ti_Data = VAR;                                               \
	tags[0].ti_Tag  = INTEGER_Number;                                    \
	IIntuition->SetGadgetAttrsA(GAD##_integer, gui_window, NULL, tags);  \
	tags[0].ti_Tag  = SLIDER_Level;                                      \
	IIntuition->SetGadgetAttrsA(GAD##_slider, gui_window, NULL, tags);

	UPDATE_UI(bd->fish, fish)
	UPDATE_UI(bd->goalchgfreq, goalchg)

	UPDATE_UI((int)(bd->maxvel * 10.0f), maxvel)
	UPDATE_UI((int)(bd->minvel * 10.0f), minvel)
	UPDATE_UI((int)(bd->acclimit * 10.0f), acclimit)
	UPDATE_UI((int)(bd->distexp * 10.0f), distexp)
	UPDATE_UI((int)(bd->avoidfact * 10.0f), avoidfact)
	UPDATE_UI((int)(bd->matchfact * 100.0f), matchfact)
	UPDATE_UI((int)(bd->centerfact * 100.0f), centerfact)
	UPDATE_UI((int)(bd->targetfact * 1.0f), targetfact)
	UPDATE_UI((int)(bd->minradius * 1.0f), minradius)
	UPDATE_UI((int)(bd->momentum * 100.0f), momentum)
	UPDATE_UI((int)(bd->distcomp * 1.0f), distcomp)

#undef UPDATE_UI

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

	bd->screenmodeID = id;
	bd->fog = TRUE;
	bd->drawbbox = TRUE;
	bd->drawgoal = FALSE;
	bd->fish = 100;
	bd->goalchgfreq = 50;
	bd->maxvel = 7.0f;
	bd->minvel = 1.0f;
	bd->acclimit = 8.0f;
	bd->distexp = 2.2f;
	bd->avoidfact = 1.5f;
	bd->matchfact = 0.15f;
	bd->centerfact = 0.1f;
	bd->targetfact = 80.0f;
	bd->minradius = 30.0f;
	bd->momentum = 0.9f;
	bd->distcomp = 10.0f;
}
///

