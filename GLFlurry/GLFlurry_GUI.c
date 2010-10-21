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
#include "GLFlurry_GUI.h"

#define GA_Style	    0x1001
#define GA_ScreenMode	   0x1002

struct Library *			ChooserBase			= NULL;
struct Library *			GetScreenModeBase	= NULL;
struct Library *        	LabelBase   		= NULL;
struct Library *        	LayoutBase   		= NULL;
struct Library *			SpaceBase			= NULL;

struct ChooserIFace *		IChooser			= NULL;
struct GetScreenModeIFace * IGetScreenMode		= NULL;
struct LabelIFace *    		ILabel          	= NULL;
struct LayoutIFace *    	ILayout         	= NULL;
struct SpaceIFace *			ISpace				= NULL;

struct Gadget *style_chooser = NULL;
struct Gadget *screenmode_requester = NULL;
struct List *styles_list;

char *styles[] = {"Random", "Water", "Fire", "Psychedelic", "RGB", "Binary", "Classic", "insane", NULL };

GLint GUIModeID = -1;
ULONG temp_mode_ID = 0xffffffff;
struct Hook* ScreenmodeHook = NULL;

const struct Hook GUIEventHook = {{ NULL, NULL }, (HOOKFUNC)GUIEventFunc, NULL, NULL };
const struct Hook GUIIdcmpHook = {{ NULL, NULL }, (HOOKFUNC)GUIIdcmpFunc, NULL, NULL };

uint32 OpenGUILibraries()
{
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
	if (IChooser) IExec->DropInterface((struct Interface*)IChooser);
	IChooser = NULL;
	if (ChooserBase) IExec->CloseLibrary(ChooserBase);
	ChooserBase = NULL;

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

		styles_list = (struct List *)IExec->AllocVec(sizeof(struct List), MEMF_CLEAR);

		if (styles_list != NULL)
		{
			IExec->NewList(styles_list);
			for (i = 0; i < 8; i++)
			{
				node = (struct Node*)(IChooser->AllocChooserNode(CNA_Text, (ULONG)styles[i], TAG_DONE));
				IExec->AddTail(styles_list, node);
			}
		}

		ScreenmodeHook = IExec->AllocSysObjectTags(ASOT_HOOK, ASOHOOK_Entry, ScreenmodeCallback, TAG_DONE);

		bpws->winWidth      = 300;
		bpws->winHeight     = 70;
		bpws->eventHook     = (APTR)&GUIEventHook;

		bpws->rootLayout = LayoutObject,
                LAYOUT_Orientation, LAYOUT_VERTICAL,
                LAYOUT_SpaceInner, TRUE,
                LAYOUT_SpaceOuter, TRUE,
                LAYOUT_DeferLayout, TRUE,

				/* Style Chooser */
				LAYOUT_AddChild, style_chooser =
                ChooserObject,
                    GA_RelVerify, TRUE,
					GA_ID, GA_Style,
                    CHOOSER_PopUp, TRUE,
					CHOOSER_Labels, styles_list,
                ChooserEnd,

                CHILD_Label,
                LabelObject,
                    LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Style: ",
                LabelEnd,

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

		result = TRUE;
	}

	return( result );
}

void DestroyGUI()
{
	if (styles_list != NULL)
	{
		free_chooser_list(styles_list);
		IExec->FreeVec(styles_list);
		styles_list = NULL;
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
			case GA_Style:
				IIntuition->GetAttr(CHOOSER_Selected, style_chooser, &attr);
				bd->style = attr;
				refetch |= TRUE;
				break;
			case GA_ScreenMode:
				RequestScreenMode((Object*)screenmode_requester, bd->WinInfo.window);
				IIntuition->GetAttr((ULONG)GETSCREENMODE_DisplayID, screenmode_requester, &attr);
				bd->screenmodeID = attr;
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

    tags[0].ti_Tag  = CHOOSER_Selected;
	tags[0].ti_Data = bd->style;
	IIntuition->SetGadgetAttrsA(style_chooser, gui_window, NULL, tags);

    tags[0].ti_Tag = GETSCREENMODE_DisplayID;
	tags[0].ti_Data = bd->screenmodeID;
	IIntuition->RefreshSetGadgetAttrsA(screenmode_requester, gui_window, NULL, tags);
}


