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
#include <gadgets/checkbox.h>
#include <gadgets/layout.h>
#include <gadgets/chooser.h>
#include <gadgets/integer.h>
#include <gadgets/getscreenmode.h>
#include <gadgets/slider.h>
#include <gadgets/space.h>
#include <images/label.h>
#include <intuition/icclass.h>
#include <libraries/gadtools.h> /* for PLACETEXT_LEFT */
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

#define GA_Radius	       0x1001
#define GA_Radius_Integer  0x1002
#define GA_Speed		   0x1003
#define GA_Speed_Integer   0x1004
#define GA_Waves		   0x1005
#define GA_Waves_Integer   0x1006
#define GA_Spin            0x1007
#define GA_Texture         0x1008
#define GA_Wander          0x1009
#define GA_TextureSize     0x100a
#define GA_ScreenMode      0x100b

struct Library *            ButtonBase     	    = NULL;
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

struct Gadget *radius_slider = NULL, *radius_integer = NULL;
struct Gadget *speed_slider = NULL, *speed_integer = NULL;
struct Gadget *waves_slider = NULL, *waves_integer = NULL;
struct Gadget *texture_size_chooser = NULL;
struct Gadget *spin_chooser = NULL;
struct Gadget *screenmode_requester = NULL;
struct Gadget *texture_checkbox = NULL, *wander_checkbox = NULL;
struct List *spin_list = NULL;
struct List *texture_size_list = NULL;

char* spins[] = {"NONE", "X", "Y", "Z", "XY", "XZ", "YZ", "XYZ"};
char* texture_sizes[] = {"16x16", "32x32", "64x64", "128x128", "256x256"};

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
const struct Hook GUIIdcmpHook = {{ NULL, NULL }, (HOOKFUNC)GUIIdcmpFunc, NULL, NULL };

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

/// free_chooser_list
void free_chooser_list(struct List *list)
{
    struct Node *node, *nextnode;

    node = list->lh_Head;
	while ((nextnode = node->ln_Succ))
    {
        IChooser->FreeChooserNode(node);
        node = nextnode;
    }
}
///

/// MakeGUI(struct BlankerDate* bd, struct BlankerPrefsWindowSetup* bpws)
BOOL MakeGUI(struct BlankerData* bd, struct BlankerPrefsWindowSetup* bpws)
{
	BOOL result = FALSE;

	if ( bd && bpws )
	{
		struct Node *node;
		int i;

		spin_list = (struct List *)IExec->AllocSysObject(ASOT_LIST, TAG_DONE);

		if (spin_list != NULL)
		{
			for (i = 0; i < 8; i++)
			{
				node = (struct Node*)(IChooser->AllocChooserNode(CNA_Text, (ULONG)spins[i], TAG_DONE));
				IExec->AddTail(spin_list, node);
			}
		}

		texture_size_list = (struct List *)IExec->AllocSysObject(ASOT_LIST, TAG_DONE);

		if (texture_size_list != NULL)
		{
			for (i = 0; i < 5; i++)
			{
				node = (struct Node*)(IChooser->AllocChooserNode(CNA_Text, (ULONG)texture_sizes[i], TAG_DONE));
				IExec->AddTail(texture_size_list, node);
			}
		}

		ScreenmodeHook = IExec->AllocSysObjectTags(ASOT_HOOK, ASOHOOK_Entry, ScreenmodeCallback, TAG_DONE);

		bpws->winWidth      = 300;
		bpws->winHeight     = 140;
		bpws->eventHook     = (APTR)&GUIEventHook;

		bpws->rootLayout = LayoutObject,
                LAYOUT_Orientation, LAYOUT_VERTICAL,
                LAYOUT_SpaceInner, TRUE,
                LAYOUT_SpaceOuter, TRUE,
                LAYOUT_DeferLayout, TRUE,

				/* Waves Slider and Integer */
				LAYOUT_AddChild,
				LayoutObject,
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,
					/* Waves Slider */
					LAYOUT_AddChild, waves_slider =
					SliderObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Waves,
						SLIDER_Level, 2,
						SLIDER_Min, 1,
						SLIDER_Max, 5,
						SLIDER_Orientation, SLIDER_HORIZONTAL,
						ICA_MAP, sl_2_int_map,
					SliderEnd,

                    /* Density Integer */
					LAYOUT_AddChild, waves_integer =
					IntegerObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Waves_Integer,
						INTEGER_Number, 2,
						INTEGER_Minimum, 1,
						INTEGER_Maximum, 5,
						INTEGER_Arrows, FALSE,
						ICA_MAP, int_2_sl_map,
					IntegerEnd,
				LayoutEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Waves: ",
				LabelEnd,

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

				/* Wave radius slider and Integer */
				LAYOUT_AddChild,
				LayoutObject,
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,
					/* Radius Slider */
					LAYOUT_AddChild, radius_slider =
					SliderObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Radius,
						SLIDER_Level, 50,
						SLIDER_Min, 8,
						SLIDER_Max, 128,
						SLIDER_Orientation, SLIDER_HORIZONTAL,
						ICA_MAP, sl_2_int_map,
					SliderEnd,

					/* Radius Integer */
					LAYOUT_AddChild, radius_integer =
					IntegerObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Radius_Integer,
						INTEGER_Number, 50,
						INTEGER_Minimum, 8,
						INTEGER_Maximum, 128,
						INTEGER_Arrows, FALSE,
						ICA_MAP, int_2_sl_map,
					IntegerEnd,
				LayoutEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Wave Radius: ",
				LabelEnd,

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

				/* Speed Slider and Integer */
				LAYOUT_AddChild,
				LayoutObject,
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,
                    /* Speed Slider */
					LAYOUT_AddChild, speed_slider =
					SliderObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Speed,
						SLIDER_Level, 80,
						SLIDER_Min, 0,
						SLIDER_Max, 300,
						SLIDER_Orientation, SLIDER_HORIZONTAL,
						ICA_MAP, sl_2_int_map,
					SliderEnd,

                    /* Speed Integer */
					LAYOUT_AddChild, speed_integer =
					IntegerObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Speed_Integer,
						INTEGER_Number, 80,
						INTEGER_Minimum, 0,
						INTEGER_Maximum, 300,
						INTEGER_Arrows, FALSE,
						ICA_MAP, int_2_sl_map,
					IntegerEnd,
				LayoutEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Wave Speed: ",
				LabelEnd,

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

				/* Spin Chooser */
				LAYOUT_AddChild, spin_chooser =
				ChooserObject,
					GA_RelVerify, TRUE,
					GA_ID, GA_Spin,
					CHOOSER_PopUp, TRUE,
					CHOOSER_Labels, spin_list,
				ChooserEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Spin: ",
				LabelEnd,

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

				/* Texture settings */
				LAYOUT_AddChild,
				LayoutObject,
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,
					LAYOUT_HorizAlignment, LALIGN_CENTER,

					/* Wander */
					LAYOUT_AddChild, wander_checkbox =
					CheckBoxObject,
						GA_ID, GA_Wander,
						CHECKBOX_TextPlace, PLACETEXT_LEFT,
						GA_Text, "_Wander: ",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

					/* Texture */
					LAYOUT_AddChild, texture_checkbox =
					CheckBoxObject,
						GA_ID, GA_Texture,
						CHECKBOX_TextPlace, PLACETEXT_LEFT,
						GA_Text, "_Texture: ",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

					/* Texture size Chooser */
					LAYOUT_AddChild, texture_size_chooser =
					ChooserObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_TextureSize,
						CHOOSER_PopUp, TRUE,
						CHOOSER_Labels, texture_size_list,
					ChooserEnd,

					CHILD_Label,
					LabelObject,
						LABEL_Justification, LABEL_LEFT,
						LABEL_Text, "Texture Size: ",
					LabelEnd,
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

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

             LayoutEnd; /* End Main Layout */

		if (bpws->rootLayout != NULL)
		{
			ica_targets[0].ti_Data = (ULONG)waves_integer;
			IIntuition->SetAttrsA(waves_slider, ica_targets);
			ica_targets[0].ti_Data = (ULONG)waves_slider;
			IIntuition->SetAttrsA(waves_integer, ica_targets);

			ica_targets[0].ti_Data = (ULONG)radius_integer;
			IIntuition->SetAttrsA(radius_slider, ica_targets);
			ica_targets[0].ti_Data = (ULONG)radius_slider;
			IIntuition->SetAttrsA(radius_integer, ica_targets);

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
	if (spin_list != NULL)
	{
		free_chooser_list(spin_list);
		IExec->FreeSysObject(ASOT_LIST, spin_list);
		spin_list = NULL;
	}
	if (texture_size_list != NULL)
	{
		free_chooser_list(texture_size_list);
		IExec->FreeSysObject(ASOT_LIST, texture_size_list);
		texture_size_list = NULL;
	}
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
			case GA_Waves:
			case GA_Waves_Integer:
				IIntuition->GetAttr(INTEGER_Number, waves_integer, &attr);
				bd->waves = attr;
				refetch |= TRUE;
				break;
			case GA_Radius:
			case GA_Radius_Integer:
				IIntuition->GetAttr(INTEGER_Number, radius_integer, &attr);
				bd->radius = attr;
				refetch |= TRUE;
				break;
			case GA_Speed:
			case GA_Speed_Integer:
				IIntuition->GetAttr(INTEGER_Number, speed_integer, &attr);
				bd->speed = attr;
				refetch |= TRUE;
				break;
			case GA_Spin:
				IIntuition->GetAttr(CHOOSER_Selected, spin_chooser, &attr);
				bd->spin = attr;
				refetch |= TRUE;
				break;
			case GA_Wander:
				IIntuition->GetAttr(GA_Selected, wander_checkbox, &attr);
				bd->wander = (BOOL)attr;
				refetch |= TRUE;
				break;
			case GA_Texture:
				IIntuition->GetAttr(GA_Selected, texture_checkbox, &attr);
				bd->texture = (BOOL)attr;
				refetch |= TRUE;
				break;
			case GA_TextureSize:
				IIntuition->GetAttr(CHOOSER_Selected, texture_size_chooser, &attr);
				bd->texture_size = attr;
				refetch |= TRUE;
				break;
		  case GA_ScreenMode:
				RequestScreenMode((Object*)screenmode_requester, bd->WinInfo.window);
				IIntuition->GetAttr(GETSCREENMODE_DisplayID, screenmode_requester, &attr);
				bd->screenmodeID = attr;
				break;
		    default:
				break;
		} /* switch gadget */

	    bd->refetchSettings = refetch;
    }
}
///

/// GUI Idcmp Func(struct Hook*hook, struct BlankerModuleIFace *Self, struct BlankerPrefsWinIDCMPEvent *event )
void GUIIdcmpFunc(struct Hook* hook, struct BlankerModuleIFace* Self, struct BlankerPrefsWinIDCMPEvent* event)
{
}
///

/// UpdateWindowSettings(struct BlankerData* bd)
void UpdateWindowSettings(struct BlankerData* bd)
{
	struct TagItem tags[] = { {0,0}, {TAG_DONE}};

	struct Window* gui_window = bd->WinInfo.window;

	tags[0].ti_Tag  = INTEGER_Number;
	tags[0].ti_Data = bd->waves;
	IIntuition->SetGadgetAttrsA(waves_integer, gui_window, NULL, tags);

	tags[0].ti_Data = bd->radius;
	IIntuition->SetGadgetAttrsA(radius_integer, gui_window, NULL, tags);

	tags[0].ti_Data = bd->speed;
    IIntuition->SetGadgetAttrsA(speed_integer, gui_window, NULL, tags);

    tags[0].ti_Tag  = SLIDER_Level;
	tags[0].ti_Data = bd->waves;
	IIntuition->SetGadgetAttrsA(waves_slider, gui_window, NULL, tags);

	tags[0].ti_Data = bd->radius;
	IIntuition->SetGadgetAttrsA(radius_slider, gui_window, NULL, tags);

	tags[0].ti_Data = bd->speed;
    IIntuition->SetGadgetAttrsA(speed_slider, gui_window, NULL, tags);

	tags[0].ti_Tag  = CHOOSER_Selected;
	tags[0].ti_Data = bd->spin;
	IIntuition->SetGadgetAttrsA(spin_chooser, gui_window, NULL, tags);

	tags[0].ti_Data = bd->texture_size;
	IIntuition->SetGadgetAttrsA(texture_size_chooser, gui_window, NULL, tags);

    tags[0].ti_Tag = GETSCREENMODE_DisplayID;
	tags[0].ti_Data = bd->screenmodeID;
	IIntuition->RefreshSetGadgetAttrsA(screenmode_requester, gui_window, NULL, tags);

	tags[0].ti_Tag  = GA_Selected;
	tags[0].ti_Data = (ULONG)bd->texture;
	IIntuition->RefreshSetGadgetAttrsA(texture_checkbox, gui_window, NULL, tags);

	tags[0].ti_Data = (ULONG)bd->wander;
	IIntuition->RefreshSetGadgetAttrsA(wander_checkbox, gui_window, NULL, tags);
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
	bd->waves = 2;
	bd->radius = 16;
	bd->speed = 80;
	bd->spin = 7;
	bd->texture = TRUE;
	bd->texture_size = 2;
	bd->wander = TRUE;
}
///

