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
#include "GLMatrixModule_GUI.h"

#define GA_Density		   0x1001
#define GA_Density_Integer 0x1002
#define GA_Speed		   0x1003
#define GA_Speed_Integer   0x1004
#define GA_Encoding		   0x1005
#define GA_ScreenMode	   0x1006
#define GA_Fog			   0x1007
#define GA_Wave			   0x1008
#define GA_Rotate		   0x1009
#define GA_Invert		   0x1010

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

struct Gadget *density_slider = NULL, *density_integer = NULL;
struct Gadget *speed_slider = NULL, *speed_integer = NULL;
struct Gadget *encoding_chooser = NULL;
struct Gadget *screenmode_requester = NULL;
struct Gadget *fog_checkbox = NULL, *wave_checkbox = NULL, *rotate_checkbox = NULL, *invert_checkbox = NULL;
struct List *encodings_list;

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

char *encodings[] = {"Matrix", "DNA", "Binary", "Hexadecimal", "decimal", NULL };

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

void free_chooser_list(struct List *list)
{
	struct Node *node, *nextnode;

	node = list->lh_Head;
	while ((nextnode = node->ln_Succ))
	{
		IChooser->FreeChooserNode(node);
		node = nextnode;
	}
	IExec->NewList(list);
}

BOOL MakeGUI( struct BlankerData *bd, struct BlankerPrefsWindowSetup *bpws )
{
	BOOL result = FALSE;

	if ( bd && bpws )
	{
		struct Node *node;
		int i;

		encodings_list = (struct List *)IExec->AllocVecTags(sizeof(struct List), AVT_ClearWithValue, 0, TAG_DONE);

		if (encodings_list != NULL)
		{
			IExec->NewList(encodings_list);
			for (i = 0; i < 5; i++)
			{
				node = (struct Node*)(IChooser->AllocChooserNode(CNA_Text, (ULONG)encodings[i], TAG_DONE));
				IExec->AddTail(encodings_list, node);
			}
		}

		ScreenmodeHook = IExec->AllocSysObjectTags(ASOT_HOOK, ASOHOOK_Entry, ScreenmodeCallback, TAG_DONE);

		bpws->winWidth      = 200;
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
					LAYOUT_AddChild, density_slider =
					(struct Gadget*)SliderObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Density,
						SLIDER_Level, 20,
						SLIDER_Min, 2,
						SLIDER_Max, 200,
						SLIDER_Orientation, SLIDER_HORIZONTAL,
						ICA_MAP, sl_2_int_map,
					SliderEnd,

					/* Density Integer */
					LAYOUT_AddChild, density_integer =
					(struct Gadget*)IntegerObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Density_Integer,
						INTEGER_Number, 20,
						INTEGER_Minimum, 2,
						INTEGER_Maximum, 200,
						INTEGER_Arrows, FALSE,
						ICA_MAP, int_2_sl_map,
					IntegerEnd,
				LayoutEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Density: ",
				LabelEnd,

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

				/* Speed Slider and Integer */
				LAYOUT_AddChild,
				LayoutObject,
					LAYOUT_Orientation, LAYOUT_HORIZONTAL,
					/* Speed Slider */
					LAYOUT_AddChild, speed_slider =
					(struct Gadget*)SliderObject,
						GA_RelVerify, TRUE,
						GA_ID, GA_Speed,
						SLIDER_Level, 100,
						SLIDER_Min, 0,
						SLIDER_Max, 300,
						SLIDER_Orientation, SLIDER_HORIZONTAL,
						ICA_MAP, sl_2_int_map,
					SliderEnd,

					/* Speed Integer */
					LAYOUT_AddChild, speed_integer =
					(struct Gadget*)IntegerObject,
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

				/* Encodings Chooser */
				LAYOUT_AddChild, encoding_chooser =
				(struct Gadget*)ChooserObject,
					GA_RelVerify, TRUE,
					GA_ID, GA_Encoding,
					CHOOSER_PopUp, TRUE,
					CHOOSER_Labels, encodings_list,
				ChooserEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Encoding: ",
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
					LAYOUT_AddChild, wave_checkbox =
					(struct Gadget*)CheckBoxObject,
						GA_ID, GA_Wave,
						GA_Text, "_Wave",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

					/* Rotate */
					LAYOUT_AddChild, rotate_checkbox =
					(struct Gadget*)CheckBoxObject,
						GA_ID, GA_Rotate,
						GA_Text, "_Rotate",
						GA_Selected, TRUE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

					/* Invert alpha */
					LAYOUT_AddChild, invert_checkbox =
					(struct Gadget*)CheckBoxObject,
						GA_ID, GA_Invert,
						GA_Text, "_Invert alpha",
						GA_Selected, FALSE,
						GA_RelVerify, TRUE,
					CheckBoxEnd,

				LayoutEnd, /* End Check Boxes */
			LayoutEnd; /* End Main Layout */

		if (bpws->rootLayout != NULL)
		{
			ica_targets[0].ti_Data = (ULONG)density_integer;
			IIntuition->SetAttrsA((Object*)density_slider, ica_targets);
			ica_targets[0].ti_Data = (ULONG)density_slider;
			IIntuition->SetAttrsA((Object*)density_integer, ica_targets);

			ica_targets[0].ti_Data = (ULONG)speed_integer;
			IIntuition->SetAttrsA((Object*)speed_slider, ica_targets);
			ica_targets[0].ti_Data = (ULONG)speed_slider;
			IIntuition->SetAttrsA((Object*)speed_integer, ica_targets);
		}

		result = TRUE;
	}

	return( result );
}

void DestroyGUI()
{
	if (encodings_list != NULL)
	{
		free_chooser_list(encodings_list);
		IExec->FreeVec(encodings_list);
		encodings_list = NULL;
	}
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
			case GA_Density:
			case GA_Density_Integer:
				IIntuition->GetAttr(INTEGER_Number, (Object*)density_integer, &attr);
				bd->density = attr;
				refetch |= TRUE;
				break;
			case GA_Speed:
			case GA_Speed_Integer:
				IIntuition->GetAttr(INTEGER_Number, (Object*)speed_integer, &attr);
				bd->speed = attr;
				refetch |= TRUE;
				break;
			case GA_Encoding:
				IIntuition->GetAttr(CHOOSER_Selected, (Object*)encoding_chooser, &attr);
				bd->encoding = attr;
				refetch |= TRUE;
				break;
			case GA_ScreenMode:
				RequestScreenMode((Object*)screenmode_requester, bd->WinInfo.window);
				IIntuition->GetAttr(GETSCREENMODE_DisplayID, (Object*)screenmode_requester, &attr);
				bd->screenmodeID = attr;
				break;
			case GA_Fog:
				IIntuition->GetAttr(GA_Selected, (Object*)fog_checkbox, &attr);
				bd->fog = (BOOL)attr;
				refetch |= TRUE;
				break;
			case GA_Wave:
				IIntuition->GetAttr(GA_Selected, (Object*)wave_checkbox, &attr);
				bd->wave = (BOOL)attr;
				refetch |= TRUE;
				break;
			case GA_Rotate:
				IIntuition->GetAttr(GA_Selected, (Object*)rotate_checkbox, &attr);
				bd->rotate = (BOOL)attr;
				refetch |= TRUE;
				break;
			case GA_Invert:
				IIntuition->GetAttr(GA_Selected, (Object*)invert_checkbox, &attr);
				bd->invertAlpha = (BOOL)attr;
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
	tags[0].ti_Data = bd->density;
	IIntuition->SetGadgetAttrsA(density_integer, gui_window, NULL, tags);

	tags[0].ti_Data = bd->speed;
	IIntuition->SetGadgetAttrsA(speed_integer, gui_window, NULL, tags);

	tags[0].ti_Tag  = SLIDER_Level;
	tags[0].ti_Data = bd->density;
	IIntuition->SetGadgetAttrsA(density_slider, gui_window, NULL, tags);

	tags[0].ti_Data = bd->speed;
	IIntuition->SetGadgetAttrsA(speed_slider, gui_window, NULL, tags);

	tags[0].ti_Tag  = CHOOSER_Selected;
	tags[0].ti_Data = bd->encoding;
	IIntuition->SetGadgetAttrsA(encoding_chooser, gui_window, NULL, tags);

	tags[0].ti_Tag = GETSCREENMODE_DisplayID;
	tags[0].ti_Data = bd->screenmodeID;
	IIntuition->RefreshSetGadgetAttrsA(screenmode_requester, gui_window, NULL, tags);

	tags[0].ti_Tag  = GA_Selected;
	tags[0].ti_Data = (ULONG)bd->fog;
	IIntuition->RefreshSetGadgetAttrsA(fog_checkbox, gui_window, NULL, tags);

	tags[0].ti_Data = (ULONG)bd->wave;
	IIntuition->RefreshSetGadgetAttrsA(wave_checkbox, gui_window, NULL, tags);

	tags[0].ti_Data = (ULONG)bd->rotate;
	IIntuition->RefreshSetGadgetAttrsA(rotate_checkbox, gui_window, NULL, tags);

	tags[0].ti_Data = (ULONG)bd->invertAlpha;
	IIntuition->RefreshSetGadgetAttrsA(invert_checkbox, gui_window, NULL, tags);
}


