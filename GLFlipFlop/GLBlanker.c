/*
 * This example is best viewed with a TabSize of 4 and using GoldEd's Folding methods.
 * Feel free to use/misuse this example as you se fit.
 */

#define VERSION     3
#define REVISION    0
#define LIBPRI      0
#define LIBVERS     "3.0"
#define VSTRING     LIBNAME" "LIBVERS" ("__AMIGADATE__")\r\n"
#define VERSTAG     "\0$VER: "LIBNAME" "LIBVERS" ("__AMIGADATE__")"

#define DebugLevel  0


#include "blanker.h"
#include "yarandom.h"
#include "GLBlanker_GUI.h"
#include "GLBlanker_Prefs.h"
#include "../BlankerStartup/Startup.inl"

extern void init_flipflop(struct BlankerData* bd);
extern void reshape_flipflop(int w, int h);
extern void draw_flipflop();
extern void release_flipflop();

/// RenderPreview
void RenderPreview(struct BlankerData *bd)
{
	uint32 run;
	uint32 w;
	uint32 h;
	static uint32 old_w, old_h;

	run = FALSE;

#if DebugLevel > 0
	IExec->DebugPrintF( "--> RenderPreview\n" );
#endif

	w = bd->blankerRender->blankWidth;
	h = bd->blankerRender->blankHeight;

#if DebugLevel > 0
	IExec->DebugPrintF("W: %ld, H: %ld\n", w, h);
#endif

	if (PreviewContext == NULL || old_w != w || old_h != h)
	{
		if (PreviewContext != NULL)
		{
			DeletePreviewContext();
		}

		if (CreatePreviewContext(bd, w, h))
		{
			run = TRUE;
			old_w = w;
			old_h = h;
		}
	}
	else
	{
		run = TRUE;
	}

	while( run == TRUE )
    {
		uint32 lastBlankingMode;
		BOOL activeRendering;

		activeRendering = FALSE;
		lastBlankingMode = SBBM_NoBlanking;

		// RWO: Lav Init beregninger her
		// n�r refetchSetting �ndres bliver de beregnet igen
		mglMakeCurrent(PreviewContext);
		init_flipflop(bd);
		reshape_flipflop(w, h);

		bd->refetchSettings = FALSE;

	    while(( run == TRUE ) && ( bd->refetchSettings == FALSE ))
	    {
		    if ( IExec->SetSignal( 0, 0 ) & bd->blankerRender->taskSigBreak )
		    {
			    run = FALSE;
			    break;
		    }

			if ( bd->currentBlankingMode != lastBlankingMode )
			{
				// something has changed...

				switch( bd->currentBlankingMode )
				{
					case SBBM_Preview:
					case SBBM_Blanking:
					{
						if ( IGraphics->GetBitMapAttr( bd->blankerRender->rp->BitMap, BMA_DEPTH ) > 8 )
						{
							activeRendering = TRUE;
						}
						break;
					}

					default:
					{
						activeRendering = FALSE;
						break;
					}
				}

				lastBlankingMode = bd->currentBlankingMode;
			}

			if ( activeRendering )
			{


				// RWO: Lav din main beregninger her
				mglMakeCurrent(PreviewContext);
				mglLockDisplay();
				draw_flipflop();
				mglUnlockDisplay();

				IGraphics->BltBitMapRastPort(previewBitMap, 0, 0, bd->blankerRender->rp, 0, 0, w, h, 0xc0);

				if ( bd->blankerRender->renderHook )
				{
					IUtility->CallHookPkt( bd->blankerRender->renderHook, (APTR)BMRHM_FrameRendered, (APTR)0 );
				}
			}

			// S� frem provakere vi et task skift og dermed er der ingen busywaiting
			IDOS->Delay( 1 );
	    }
    }

//bailout:
	release_flipflop();
}
///

/// RenderScreen
void RenderScreen(struct BlankerData* bd)
{
	struct Screen *scr;
	struct Window *win;
	Object *MouseObject;
	uint32 run;
	uint32 w;
	uint32 h;

	MouseObject = NULL;
    run = TRUE;
    win = NULL;

#if DebugLevel > 0
	IExec->DebugPrintF( "--> RenderScreen\n" );
#endif

	w = IP96->p96GetModeIDAttr( bd->screenmodeID, P96IDA_WIDTH );
	h = IP96->p96GetModeIDAttr( bd->screenmodeID, P96IDA_HEIGHT );

#if DebugLevel > 0
	IExec->DebugPrintF( "    width: %ld, height: %ld\n", w, h);
#endif

	
	ScreenContext = IMiniGL->CreateContextTags( MGLCC_ScreenMode, bd->screenmodeID,
											MGLCC_Width, w,
											MGLCC_Height, h,
											TAG_END);
	if (ScreenContext != NULL)
	{
		mglMakeCurrent(ScreenContext);
		mglLockMode(MGL_LOCK_SMART);
		mglEnableSync(GL_TRUE);
	}
	else
	{
		goto bailout;
	}

	win = ScreenContext->GetWindowHandle();
	scr = win->WScreen;

	MouseObject = IIntuition->NewObject( NULL, "pointerclass",
		POINTERA_BitMap,    &PointerBitmap,
		POINTERA_XOffset,   0,
		POINTERA_YOffset,   0,
		TAG_END
	);

	if ( MouseObject == NULL )
	{
	    goto bailout;
	}

	while( run == TRUE )
    {
		uint32 lastBlankingMode;
		BOOL activeRendering;

		activeRendering = FALSE;
		lastBlankingMode = SBBM_NoBlanking;

		// RWO: Lav Init beregninger her
		// n�r refetchSetting �ndres bliver de beregnet igen
		mglMakeCurrent(ScreenContext);
		init_flipflop(bd);
		reshape_flipflop(w, h);

		bd->refetchSettings = FALSE;
	    while(( run == TRUE ) && ( bd->refetchSettings == FALSE ))
	    {
		    if ( IExec->SetSignal( 0, 0 ) & bd->blankerRender->taskSigBreak )
		    {
			    run = FALSE;
			    break;
		    }

			if ( bd->currentBlankingMode != lastBlankingMode )
			{
				// something has changed...

				SetBlankingMode( scr, bd->currentBlankingMode );

				switch( bd->currentBlankingMode )
				{
					case SBBM_Preview:
					case SBBM_Blanking:
					{
						if ( IGraphics->GetBitMapAttr( win->RPort->BitMap, BMA_DEPTH ) > 8 )
						{
							activeRendering = TRUE;
						}
						break;
					}

					default:
					{
						activeRendering = FALSE;
						break;
					}
				}

				lastBlankingMode = bd->currentBlankingMode;
			}

			if ( activeRendering )
			{
				// RWO: Lav din main beregninger her

				mglMakeCurrent(ScreenContext);
				draw_flipflop();
				mglSwitchDisplay();

				if ( bd->blankerRender->renderHook )
				{
					IUtility->CallHookPkt( bd->blankerRender->renderHook, (APTR)BMRHM_FrameRendered, (APTR)0 );
				}
			}

			// S� frem provakere vi et task skift og dermed er der ingen busywaiting
			IDOS->Delay( 1 );
	    }

		release_flipflop();
    }

	SetBlankingMode( scr, SBBM_NoBlanking );

bailout:


	if ( MouseObject )
	{
		IIntuition->DisposeObject( MouseObject );
	}

	if (ScreenContext != NULL)
	{
		mglMakeCurrent(ScreenContext);
		mglDeleteContext();
		ScreenContext = NULL;
	}
}
///
/* -- The End -- */

