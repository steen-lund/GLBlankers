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
#include "GLHyperTorus_GUI.h"

#define GA_DisplayMode		0x1001 /* chooser */
#define GA_Appearance		0x1002 /* chooser */
#define GA_Colors		    0x1003 /* chooser */
#define GA_3D_Projection   	0x1004 /* chooser */
#define GA_4D_Projection   	0x1005 /* Chooser */
#define GA_Speed_wx		   	0x1006 /* integer */
#define GA_Speed_wy		   	0x1007 /* integer */
#define GA_Speed_wz		   	0x1008 /* integer */
#define GA_Speed_xy		   	0x1009 /* integer */
#define GA_Speed_xz		   	0x1010 /* integer */
#define GA_Speed_yz		   	0x1011 /* integer */
#define GA_ScreenMode	   	0x1012 /* Screenmode requester */

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

struct Gadget* displaymode_chooser = NULL;
struct Gadget* appearance_chooser = NULL;
struct Gadget* color_chooser = NULL;
struct Gadget* projection_3d_chooser = NULL;
struct Gadget* projection_4d_chooser = NULL;
struct Gadget* wx_integer = NULL, *wy_integer = NULL, *wz_integer = NULL;
struct Gadget* xy_integer = NULL, *xz_integer = NULL, *yz_integer = NULL;
struct Gadget* screenmode_requester = NULL;

struct List *displaymode_list = NULL;
struct List *appearance_list = NULL;
struct List *color_list = NULL;
struct List *projection_3d_list = NULL;
struct List *projection_4d_list = NULL;

char *displaymodes[] = {"Wireframe", "Surface", "Transparent", NULL};
char *appearances[] = {"Solid", "Bands", "Spirals-1", "Spirals-2", "Spirals-4", "Spirals-8", "Spirals-16", NULL};
char *colors[] = {"Two-sided", "Colorwheel", NULL};
char *projection3d[] = {"Perspective 3D", "Orthographics 3D", NULL};
char *projection4d[] = {"Perspective 4D", "Orthographics 4D", NULL};

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

/// create_chooser_list
struct List* create_chooser_list(char *array[])
{
	struct List* list = (struct List *)IExec->AllocVec(sizeof(struct List), MEMF_CLEAR);

	if (list != NULL)
	{
		int i = 0;
		struct Node *node = NULL;

		IExec->NewList(list);
		while(array[i] != NULL)
		{
			node = (struct Node*)(IChooser->AllocChooserNode(CNA_Text, (ULONG)array[i], TAG_DONE));
			IExec->AddTail(list, node);
			++i;
		}
	}

	return list;
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
    IExec->NewList(list);
}
///

/// MakeGUI(struct BlankerDate* bd, struct BlankerPrefsWindowSetup* bpws)
BOOL MakeGUI(struct BlankerData* bd, struct BlankerPrefsWindowSetup* bpws)
{
	BOOL result = FALSE;

	if ( bd && bpws )
	{
		displaymode_list = create_chooser_list(displaymodes);
		appearance_list = create_chooser_list(appearances);
		color_list = create_chooser_list(colors);
		projection_3d_list = create_chooser_list(projection3d);
		projection_4d_list = create_chooser_list(projection4d);
		
		ScreenmodeHook = IExec->AllocSysObjectTags(ASOT_HOOK, ASOHOOK_Entry, ScreenmodeCallback, TAG_DONE);

		bpws->winWidth      = 300;
		bpws->winHeight     = 140;
		bpws->eventHook     = (APTR)&GUIEventHook;

		bpws->rootLayout = LayoutObject,
                LAYOUT_Orientation, LAYOUT_VERTICAL,
                LAYOUT_SpaceInner, TRUE,
                LAYOUT_SpaceOuter, TRUE,
                LAYOUT_DeferLayout, TRUE,

				/* Display mode Chooser */
				LAYOUT_AddChild, displaymode_chooser =
				ChooserObject,
					GA_RelVerify, TRUE,
					GA_ID, GA_DisplayMode,
					CHOOSER_PopUp, TRUE,
					CHOOSER_Labels, displaymode_list,
				ChooserEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Display mode: ",
				LabelEnd,

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

				/* Appearance chooser */
				LAYOUT_AddChild, appearance_chooser =
				ChooserObject,
					GA_RelVerify, TRUE,
					GA_ID, GA_Appearance,
					CHOOSER_PopUp, TRUE,
					CHOOSER_Labels, appearance_list,
				ChooserEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Appearance: ",
				LabelEnd,

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

				/* Color chooser */
				LAYOUT_AddChild, color_chooser =
				ChooserObject,
					GA_RelVerify, TRUE,
					GA_ID, GA_Colors,
					CHOOSER_PopUp, TRUE,
					CHOOSER_Labels, color_list,
				ChooserEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "Colors: ",
				LabelEnd,

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

				/* 3D Projection chooser */
				LAYOUT_AddChild, projection_3d_chooser =
				ChooserObject,
					GA_RelVerify, TRUE,
					GA_ID, GA_3D_Projection,
					CHOOSER_PopUp, TRUE,
					CHOOSER_Labels, projection_3d_list,
				ChooserEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "3D Projection: ",
				LabelEnd,

				LAYOUT_AddChild, SpaceObject, SpaceEnd,

				/* 4D Projection chooser */
				LAYOUT_AddChild, projection_4d_chooser =
				ChooserObject,
					GA_RelVerify, TRUE,
					GA_ID, GA_4D_Projection,
					CHOOSER_PopUp, TRUE,
					CHOOSER_Labels, projection_4d_list,
				ChooserEnd,

				CHILD_Label,
				LabelObject,
					LABEL_Justification, LABEL_LEFT,
					LABEL_Text, "4D Projection: ",
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
             LayoutEnd; /* End Main Layout */

		result = TRUE;
	}

	return( result );
}
///

///DestroyGUI()
void DestroyGUI()
{
	if (displaymode_list != NULL)
	{
		free_chooser_list(displaymode_list);
		IExec->FreeVec(displaymode_list);
		displaymode_list = NULL;
	}

	if (appearance_list != NULL)
	{
		free_chooser_list(appearance_list);
		IExec->FreeVec(appearance_list);
		appearance_list = NULL;
	}

	if (color_list != NULL)
	{
		free_chooser_list(color_list);
		IExec->FreeVec(color_list);
		color_list = NULL;
	}

	if (projection_3d_list != NULL)
	{
		free_chooser_list(projection_3d_list);
		IExec->FreeVec(projection_3d_list);
		projection_3d_list = NULL;
	}

	if (projection_4d_list != NULL)
	{
		free_chooser_list(projection_4d_list);
		IExec->FreeVec(projection_4d_list);
		projection_4d_list = NULL;
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
			case GA_DisplayMode:
				IIntuition->GetAttr(CHOOSER_Selected, displaymode_chooser, &attr);
				bd->displaymode = attr;
				refetch |= TRUE;
				break;
			case GA_Appearance:
				IIntuition->GetAttr(CHOOSER_Selected, appearance_chooser, &attr);
				bd->appearance = attr;
				refetch |= TRUE;
				break;
			case GA_Colors:
				IIntuition->GetAttr(CHOOSER_Selected, color_chooser, &attr);
				bd->colors = attr;
				refetch |= TRUE;
				break;
			case GA_3D_Projection:
				IIntuition->GetAttr(CHOOSER_Selected, projection_3d_chooser, &attr);
				bd->projection3d = attr;
				refetch |= TRUE;
				break;
			case GA_4D_Projection:
				IIntuition->GetAttr(CHOOSER_Selected, projection_4d_chooser, &attr);
				bd->projection4d = attr;
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

    tags[0].ti_Tag  = CHOOSER_Selected;
	tags[0].ti_Data = bd->displaymode;
	IIntuition->SetGadgetAttrsA(displaymode_chooser, gui_window, NULL, tags);

	tags[0].ti_Data = bd->appearance;
	IIntuition->SetGadgetAttrsA(appearance_chooser, gui_window, NULL, tags);

	tags[0].ti_Data = bd->colors;
	IIntuition->SetGadgetAttrsA(color_chooser, gui_window, NULL, tags);

	tags[0].ti_Data = bd->projection3d;
	IIntuition->SetGadgetAttrsA(projection_3d_chooser, gui_window, NULL, tags);

	tags[0].ti_Data = bd->displaymode;
	IIntuition->SetGadgetAttrsA(projection_4d_chooser, gui_window, NULL, tags);

    tags[0].ti_Tag = GETSCREENMODE_DisplayID;
	tags[0].ti_Data = bd->screenmodeID;
	IIntuition->RefreshSetGadgetAttrsA(screenmode_requester, gui_window, NULL, tags);
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
	bd->displaymode = 1;
	bd->appearance = 1;
	bd->colors = 1;
	bd->projection3d = 0;
	bd->projection4d = 0;
}
///

