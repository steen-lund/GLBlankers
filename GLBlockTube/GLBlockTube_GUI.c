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
#include "GLBlockTube_GUI.h"

#define GA_HoldTime		    	0x1001
#define GA_HoldTime_Integer		0x1002
#define GA_ChangeTime			0x1003
#define GA_ChangeTime_Integer	0x1004
#define GA_ScreenMode	   		0x1005
#define GA_Fog			   		0x1006
#define GA_Texture				0x1007

struct Library *        	ButtonBase     		= NULL;
struct Library *			CheckBoxBase		= NULL;
struct Library *			GetScreenModeBase	= NULL;
struct Library *			IntegerBase			= NULL;
struct Library *        	LabelBase   		= NULL;
struct Library *        	LayoutBase   		= NULL;
struct Library *        	SliderBase     		= NULL;
struct Library *			SpaceBase			= NULL;

struct ButtonIFace *    	IButton         	= NULL;
struct CheckBoxIFace *		ICheckBox			= NULL;
struct GetScreenModeIFace * IGetScreenMode		= NULL;
struct IntegerIFace *		IInteger			= NULL;
struct LabelIFace *    		ILabel          	= NULL;
struct LayoutIFace *    	ILayout         	= NULL;
struct SliderIFace *		ISlider				= NULL;
struct SpaceIFace *			ISpace				= NULL;

struct Gadget *holdtime_slider = NULL, *holdtime_integer = NULL;
struct Gadget *changetime_slider = NULL, *changetime_integer = NULL;
struct Gadget *screenmode_requester = NULL;
struct Gadget *fog_checkbox = NULL, *texture_checkbox = NULL;

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
	if (IButton) IExec->DropInterface((struct Interface*)IButton);
	IButton = NULL;
	if (ButtonBase) IExec->CloseLibrary(ButtonBase);
	ButtonBase = NULL;

	if (ICheckBox) IExec->DropInterface((struct Interface*)ICheckBox);
	ICheckBox = NULL;
	if (CheckBoxBase) IExec->CloseLibrary(CheckBoxBase);
	CheckBoxBase = NULL;

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

BOOL MakeGUI( struct BlankerData *bd, struct BlankerPrefsWindowSetup *bpws )
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

				/* Density Slider and Integer */
				LAYOUT_AddChild,
				LayoutObject,
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,
					/* Density Slider */
					LAYOUT_AddChild, holdtime_slider =
					(struct Gadget*)SliderObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_HoldTime,
						SLIDER_Level, 20,
						SLIDER_Min, 0,
						SLIDER_Max, 50,
						SLIDER_Orientation, SLIDER_HORIZONTAL,
						ICA_MAP, sl_2_int_map,
					SliderEnd,

					/* Density Integer */
					LAYOUT_AddChild, holdtime_integer =
					(struct Gadget*)IntegerObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_HoldTime_Integer,
						INTEGER_Number, 20,
						INTEGER_Minimum, 0,
						INTEGER_Maximum, 50,
						INTEGER_Arrows, FALSE,
						ICA_MAP, int_2_sl_map,
					IntegerEnd,
				LayoutEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Hold Time: ",
				LabelEnd,

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

				/* Speed Slider and Integer */
				LAYOUT_AddChild,
				LayoutObject,
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,
					/* Speed Slider */
					LAYOUT_AddChild, changetime_slider =
					(struct Gadget*)SliderObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_ChangeTime,
						SLIDER_Level, 20,
						SLIDER_Min, 0,
						SLIDER_Max, 50,
						SLIDER_Orientation, SLIDER_HORIZONTAL,
						ICA_MAP, sl_2_int_map,
					SliderEnd,

					/* Speed Integer */
					LAYOUT_AddChild, changetime_integer =
					(struct Gadget*)IntegerObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_ChangeTime_Integer,
						INTEGER_Number, 20,
						INTEGER_Minimum, 0,
						INTEGER_Maximum, 50,
						INTEGER_Arrows, FALSE,
						ICA_MAP, int_2_sl_map,
					IntegerEnd,
				LayoutEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Change Time: ",
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

				/* Check boxes */
				LAYOUT_AddChild,
				LayoutObject,
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,
					LAYOUT_HorizAlignment, LALIGN_CENTER,
					LAYOUT_EvenSize, TRUE,

					/* Fog */
					LAYOUT_AddChild, fog_checkbox =
					(struct Gadget*)CheckBoxObject,
						GA_ID, GA_Fog,
						GA_Text, "_Fog",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

					/* Wave */
					LAYOUT_AddChild, texture_checkbox =
					(struct Gadget*)CheckBoxObject,
						GA_ID, GA_Texture,
						GA_Text, "_Texture",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

				LayoutEnd, /* End Check Boxes */
			LayoutEnd; /* End Main Layout */

		if (bpws->rootLayout != NULL)
		{
			ica_targets[0].ti_Data = (ULONG)holdtime_integer;
			IIntuition->SetAttrsA((Object*)holdtime_slider, ica_targets);
			ica_targets[0].ti_Data = (ULONG)holdtime_slider;
			IIntuition->SetAttrsA((Object*)holdtime_integer, ica_targets);

			ica_targets[0].ti_Data = (ULONG)changetime_integer;
			IIntuition->SetAttrsA((Object*)changetime_slider, ica_targets);
			ica_targets[0].ti_Data = (ULONG)changetime_slider;
			IIntuition->SetAttrsA((Object*)changetime_integer, ica_targets);
		}

		result = TRUE;
	}

	return( result );
}

void DestroyGUI()
{
	IExec->FreeSysObject(ASOT_HOOK, ScreenmodeHook);
}

///
/// GUI Event Func
void GUIEventFunc( struct Hook *hook, struct BlankerModuleIFace *Self, struct BlankerPrefsWinGUIEvent *event )
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
			case GA_HoldTime:
			case GA_HoldTime_Integer:
				IIntuition->GetAttr(INTEGER_Number, (Object*)holdtime_integer, &attr);
				bd->holdtime = attr;
				refetch |= TRUE;
				break;
			case GA_ChangeTime:
			case GA_ChangeTime_Integer:
				IIntuition->GetAttr(INTEGER_Number, (Object*)changetime_integer, &attr);
				bd->changetime = attr;
				refetch |= TRUE;
				break;
			case GA_ScreenMode:
				RequestScreenMode((Object*)screenmode_requester, bd->WinInfo.window);
				IIntuition->GetAttr(GETSCREENMODE_DisplayID, (Object*)screenmode_requester, &attr);
				bd->screenmodeID = attr;
				break;
			case GA_Fog:
				IIntuition->GetAttr(GA_Selected, (Object*)fog_checkbox, &attr);
				bd->dofog = (BOOL)attr;
				refetch |= TRUE;
				break;
			case GA_Texture:
				IIntuition->GetAttr(GA_Selected, (Object*)texture_checkbox, &attr);
				bd->dotexture = (BOOL)attr;
				refetch |= TRUE;
				break;
			default:
				break;
		} /* switch gadget */

	    bd->refetchSettings = refetch;
    }
}

/// GUI Idcmp Func
void GUIIdcmpFunc( struct Hook *hook, struct BlankerModuleIFace *Self, struct BlankerPrefsWinIDCMPEvent *event )
{
//struct BlankerData *bd;
//struct TagItem *tags, *tag;
//uint32 interestingEvent;
//uint32 gadgetID;
//uint32 refetch;


// Bliver ikke brugt i dette eksemble, men gemmer det hvis du vil udvide GUIen

//    if (( Self != NULL ) && ( event != NULL ) && ( event->msg->Class == IDCMP_IDCMPUPDATE ))
//    {
//	      tags = (struct TagItem *)event->msg->IAddress;
//	      gadgetID = 0;
//	      interestingEvent = FALSE;
//
//	      while(( tag = IUtility->NextTagItem( &tags )))
//	      {
//		      switch( tag->ti_Tag )
//		      {
//			      case SLIDER_Level:
//			      {
//				      interestingEvent = TRUE;
//				      break;
//			      }
//
//			      case GA_ID:
//			      {
//				      gadgetID = tag->ti_Data;
//				      break;
//			      }
//
//			      default:
//			      {
//				      break;
//			      }
//		      }
//	      }
//
//	      if ( interestingEvent )
//	      {
//		      bd = (struct BlankerData *)((uint32)Self - Self->Data.NegativeSize);
//
//			  refetch = bd->refetchSettings;
//
//		      switch( gadgetID )
//		      {
//			      case 0x1001:
//			      {
//					  IIntuition->GetAttr( SLIDER_Level, bd->Gad_Speed, &bd->Speed );
//
//					  refetch |= TRUE;
//				      break;
//			      }
//
//				  case 0x1002:
//				  {
//					  IIntuition->GetAttr( SLIDER_Level, bd->Gad_MatrixWidth, &bd->MatrixWidth );
//
//					  refetch |= TRUE;
//				      break;
//			      }
//
//				  case 0x1003:
//				  {
//					  IIntuition->GetAttr( SLIDER_Level, bd->Gad_MatrixHeight, &bd->MatrixHeight );
//
//					  refetch |= TRUE;
//				      break;
//			      }
//
//			      default:
//			      {
//				      break;
//			      }
//		      }
//
//		      bd->refetchSettings = refetch;
//	      }
//    }
}


/// UpdateWindowSettings
void UpdateWindowSettings( struct BlankerData *bd )
{
	struct TagItem tags[] = { {0,0}, {TAG_DONE}};

	struct Window* gui_window = bd->WinInfo.window;

    tags[0].ti_Tag  = INTEGER_Number;
	tags[0].ti_Data = bd->holdtime;
	IIntuition->SetGadgetAttrsA(holdtime_integer, gui_window, NULL, tags);

	tags[0].ti_Data = bd->changetime;
	IIntuition->SetGadgetAttrsA(changetime_integer, gui_window, NULL, tags);

    tags[0].ti_Tag  = SLIDER_Level;
	tags[0].ti_Data = bd->holdtime;
	IIntuition->SetGadgetAttrsA(holdtime_slider, gui_window, NULL, tags);

	tags[0].ti_Data = bd->changetime;
	IIntuition->SetGadgetAttrsA(changetime_slider, gui_window, NULL, tags);

    tags[0].ti_Tag = GETSCREENMODE_DisplayID;
	tags[0].ti_Data = bd->screenmodeID;
	IIntuition->RefreshSetGadgetAttrsA(screenmode_requester, gui_window, NULL, tags);

    tags[0].ti_Tag  = GA_Selected;
	tags[0].ti_Data = (ULONG)bd->dofog;
	IIntuition->RefreshSetGadgetAttrsA(fog_checkbox, gui_window, NULL, tags);

	tags[0].ti_Data = (ULONG)bd->dotexture;
	IIntuition->RefreshSetGadgetAttrsA(texture_checkbox, gui_window, NULL, tags);
}


