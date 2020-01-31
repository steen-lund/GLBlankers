// Common startup code
#include <classes/window.h>
#include <cybergraphx/cybergraphics.h>
#include <datatypes/pictureclass.h>
#include <devices/timer.h>
#include <intuition/icclass.h>
#include <intuition/pointerclass.h>
#include <libraries/application.h>
#include <libraries/blankermodule.h>
#include <libraries/screenblanker.h>
#include <utility/tagitem.h>
#include <proto/application.h>
#include <proto/asl.h>
#include <proto/blankermodule.h>
#include <proto/screenblanker.h>
#include <proto/cybergraphics.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/picasso96api.h>
#include <proto/utility.h>
#include <proto/window.h>
#include <proto/timer.h>
#include <string.h>
#include <stdio.h>

#define MINIGL 1
#include <GL/gl.h>
#include <proto/minigl.h>


/* -- Global Constants -- */
/// Libraries and Interfaces
struct Library *			ApplicationBase		= NULL;
struct Library *			CyberGfxBase		= NULL;
struct Library *        	DOSBase      		= NULL;
struct Library *        	GfxBase      		= NULL;
struct Library *        	IntuitionBase   	= NULL;
struct Library *        	P96Base     		= NULL;
struct Library *        	UtilityBase     	= NULL;
struct Library *			MiniGLBase			= NULL;
struct Device *             TimerBase           = NULL;
struct Library *			ScreenBlankerBase	= NULL;
struct Library *            SysBase             = NULL;
struct Library *            NewlibBase          = NULL;

struct MiniGLIFace *		IMiniGL				= NULL;
struct ApplicationIFace *	IApplication 		= NULL;
struct CyberGfxIFace *		ICyberGfx			= NULL;
struct DOSIFace *    		IDOS            	= NULL;
struct ExecIFace *      	IExec           	= NULL;
struct GraphicsIFace *      IGraphics       	= NULL;
struct IntuitionIFace * 	IIntuition      	= NULL;
struct P96IFace *   		IP96	        	= NULL;
struct PrefsObjectsIFace *	IPrefsObjects 		= NULL;
struct UtilityIFace *   	IUtility        	= NULL;
struct TimerIFace *			ITimer				= NULL;
struct ScreenBlankerIFace *	IScreenBlanker		= NULL;
struct Interface *          INewlib             = NULL;

extern int *** __reent_magic __attribute__((__alias__("SysBase")));
///

/// Structs
struct GLContextIFace *PreviewContext = NULL;
struct GLContextIFace *ScreenContext = NULL;

const char __attribute__((used)) verstag[] = VERSTAG;

struct Library *	_manager_Init( 			struct BlankerBase *libBase, APTR seglist, struct ExecIFace *myIExec );
uint32 				_manager_Obtain( 		struct LibraryManagerInterface *Self );
uint32 				_manager_Release(		struct LibraryManagerInterface *Self );
struct Library *	_manager_Open(			struct LibraryManagerInterface *Self, uint32 version );
APTR 				_manager_Close( 		struct LibraryManagerInterface *Self );
APTR 				_manager_Expunge( 		struct LibraryManagerInterface *Self );

uint32 				_blanker_Obtain( 		struct BlankerModuleIFace *Self );
uint32 				_blanker_Release( 		struct BlankerModuleIFace *Self );
uint32				_blanker_Expunge( 		struct BlankerModuleIFace *Self );
struct Interface *	_blanker_Clone( 		struct BlankerModuleIFace *Self );
BOOL 				_blanker_Get( 			struct BlankerModuleIFace *Self, uint32 msgType, uint32 *msgData );
BOOL 				_blanker_Set(			struct BlankerModuleIFace *Self, uint32 msgType, uint32 msgData );
void 				_blanker_Blank(			struct BlankerModuleIFace *Self );

uint32 				OpenLibraries( 			void );
void 				CloseLibraries( 		void );

void 				ResetSettingsToDefault( struct BlankerData *bd );
void 				UpdateWindowSettings( 	struct BlankerData *bd );

void 				RenderPreview( 			struct BlankerData *bd );
void 				RenderScreen( 			struct BlankerData *bd );

const APTR Manager_Vectors[] =
{
    _manager_Obtain,
    _manager_Release,
    NULL,
    NULL,
    _manager_Open,
    _manager_Close,
    _manager_Expunge,
    NULL,
    (APTR)-1,
};

const struct TagItem Manager_Tags[] =
{
    { MIT_Name,         (uint32)"__library"     },
    { MIT_VectorTable,  (uint32)Manager_Vectors },
    { MIT_Version,      (uint32)1               },
    { TAG_DONE,         (uint32)0               }
};

const APTR Blanker_Vectors[] =
{
	_blanker_Obtain,
	_blanker_Release,
	_blanker_Expunge,
	_blanker_Clone,
	_blanker_Get,
	_blanker_Set,
	_blanker_Blank,
    (APTR)-1
};

const struct TagItem Blanker_Tags[] =
{
	{ MIT_Name,         (uint32)"blankermodule" },
	{ MIT_VectorTable,  (uint32)Blanker_Vectors },
	{ MIT_DataSize,		(uint32)sizeof(struct BlankerData)},
	{ MIT_Flags,		(uint32)IFLF_PRIVATE	},
    { MIT_Version,      (uint32)1               },
    { TAG_DONE,         (uint32)0               }
};

const APTR LibInterfaces[] =
{
	(APTR)Manager_Tags,
	(APTR)Blanker_Tags,
    NULL
};

const struct TagItem libCreateTags[] =
{
	{ CLT_DataSize,     (uint32)sizeof(struct BlankerBase)  },
    { CLT_InitFunc,     (uint32)_manager_Init               },
    { CLT_Interfaces,   (uint32)LibInterfaces               },
    { CLT_NoLegacyIFace,(uint32)TRUE                        },
    { TAG_DONE,         (uint32)0                           }
};

const struct Resident lib_res =
{
    RTC_MATCHWORD,              // rt_MatchWord
	(APTR)&lib_res,             // rt_MatchTag
	(APTR)&lib_res+1,           // rt_EndSkip
    RTF_NATIVE|RTF_AUTOINIT,    // rt_Flags
    VERSION,                    // rt_Version
    NT_LIBRARY,                 // rt_Type
    LIBPRI,                     // rt_Pri
    LIBNAME,                    // rt_Name
    VSTRING,                    // rt_IdString
	(APTR)libCreateTags         // rt_Init
};

const struct BitMap PointerBitmap =
{
	0,
	0,
	0,
	0,
	0,
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
};

struct TimeRequest* TimeIO;
///

/* -- Library -- */

/// _start
/*
 * The system (and compiler) rely on a symbol named _start which marks
 * the beginning of execution of an ELF file. To prevent others from
 * executing this library, and to keep the compiler/linker happy, we
 * define an empty _start symbol here.
 *
 * On the classic system (pre-AmigaOS4) this was usually done by
 * moveq #0,d0
 * rts
 *
 */

int32 _start(STRPTR argstring, int32 arglen, APTR SysBase)
{
	struct ScreenBlankerNotificationTask info;
	struct Process *proc;
	struct Message *wbmsg;
	uint32 actionSignal;
	uint32 waitsignals;
	uint32 signals;
	uint32 cnt;
	int8 actionSignalBit;

	IExec = ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface));
	IExec->Obtain();

	proc = (struct Process *)IExec->FindTask( NULL );
	wbmsg= NULL;

	if ( proc->pr_CLI == 0 )
	{
		IExec->WaitPort( &proc->pr_MsgPort );
		wbmsg = IExec->GetMsg( &proc->pr_MsgPort );
	}

	ScreenBlankerBase = IExec->OpenLibrary( "screenblanker.library", 52 );
	IScreenBlanker = (struct ScreenBlankerIFace *)IExec->GetInterface( ScreenBlankerBase, "main", 1, NULL );

	if ( IScreenBlanker )
	{
		actionSignalBit = IExec->AllocSignal( -1 );

		if ( actionSignalBit != -1 )
		{
			actionSignal= 1<<actionSignalBit;
			waitsignals	= SIGBREAKF_CTRL_C | actionSignal;

			info.task	= (struct Task *)proc;
			info.signal	= actionSignal;

			IScreenBlanker->SetScreenBlankerAttrs(
				SBATTR_ActionNotificationTask, &info,
				TAG_END
			);

			if ( IScreenBlanker->OpenBlankerModule( "progdir:" LIBNAME, TAG_END ))
			{
				IScreenBlanker->SetBlankingMode( SBBM_Blanking );
				cnt	= 3;

				while( TRUE )
				{
					signals = IExec->Wait( waitsignals );

					if ( signals & SIGBREAKF_CTRL_C )
					{
						break;
					}

					if ( signals & actionSignal )
					{
						if ( cnt-- == 0 )
						{
							break;
						}
					}
				}
				IScreenBlanker->CloseBlankerModule();
			}
			IExec->FreeSignal( actionSignalBit );
		}
	}

	if ( IScreenBlanker )
	{
		IExec->DropInterface( (struct Interface *)IScreenBlanker );
	}

	if ( ScreenBlankerBase )
	{
		IExec->CloseLibrary( ScreenBlankerBase );
	}

	if ( wbmsg )
	{
		IExec->ReplyMsg( wbmsg );
	}

	IExec->Release();

    return( RETURN_OK );
}
///
/* -- Manager Interface -- */

/// _manager_Init
/* The ROMTAG Init Function */
struct Library *_manager_Init(struct BlankerBase *libBase, APTR seglist, struct ExecIFace *myIExec)
{
    IExec = myIExec;
    IExec->Obtain();
	SysBase = IExec->Data.LibBase;

#if DebugLevel > 0
	IExec->DebugPrintF( "--> _manager_Init( LibBase = %lx, seglist = %lx, IExec = %lx )\n", libBase, seglist, myIExec );
#endif

    libBase->lib_Type       = NT_LIBRARY;
    libBase->lib_Pri        = LIBPRI;
    libBase->lib_Name       = LIBNAME;
    libBase->lib_Flags      = LIBF_SUMUSED|LIBF_CHANGED;
	libBase->lib_Version    = VERSION;
    libBase->lib_Revision   = REVISION;
    libBase->lib_IdString   = VSTRING;
    libBase->lib_SegmentList= seglist;

	if ( OpenLibraries() )
	{
		ya_rand_init(0);
		return( (struct Library *)libBase );
	}
	else
	{
		CloseLibraries();

		IExec->Release();

		return( NULL );
	}
}
///

/// _manager_Obtain
uint32 _manager_Obtain(struct LibraryManagerInterface *Self)
{

#if DebugLevel > 0
    IExec->DebugPrintF( "--> _manager_Obtain()\n" );
#endif

    Self->Data.RefCount++;

    return( Self->Data.RefCount );
}
///

/// _manager_Release
uint32 _manager_Release(struct LibraryManagerInterface *Self)
{
#if DebugLevel > 0
    IExec->DebugPrintF( "--> _manager_Release()\n" );
#endif

    Self->Data.RefCount--;

    return( Self->Data.RefCount );
}
///

/// _manager_Open

/* Open the library */
struct Library *_manager_Open(struct LibraryManagerInterface *Self, uint32 version)
{
	struct BlankerBase *libBase;

	libBase = (struct BlankerBase *)Self->Data.LibBase;

#if DebugLevel > 0
    IExec->DebugPrintF( "--> _manager_Open( Version = %ld )\n", version );
#endif

    /* Add any specific open code here
       Return 0 before incrementing OpenCnt to fail opening */

    /* Add up the open count */
    libBase->lib_OpenCnt++;
    libBase->lib_Flags &= ~LIBF_DELEXP;

    return( (struct Library *)libBase );
}
///

/// _manager_Close
/* Close the library */
APTR _manager_Close(struct LibraryManagerInterface *Self)
{
	struct BlankerBase *libBase;
	APTR seglist;

	libBase = (struct BlankerBase *)Self->Data.LibBase;

#if DebugLevel > 0
    IExec->DebugPrintF( "--> _manager_Close()\n" );
#endif

    /* Make sure to undo what open did */

    /* Make the close count */
    seglist = NULL;

    libBase->lib_OpenCnt--;

    if ( libBase->lib_OpenCnt == 0 )
    {
        if ( libBase->lib_Flags & LIBF_DELEXP )
        {
//			  seglist = _manager_Expunge( Self );
		}
    }

    return( seglist );
}
///

/// _manager_Expunge
/* Expunge the library */
void DeletePreviewContext();
APTR _manager_Expunge(struct LibraryManagerInterface *Self)
{
	struct BlankerBase *libBase;
	APTR result;

	libBase = (struct BlankerBase *)Self->Data.LibBase;

#if DebugLevel > 0
    IExec->DebugPrintF( "--> _manager_Expunge()\n" );
#endif

    if ( libBase->lib_OpenCnt == 0 )
    {
        result = libBase->lib_SegmentList;
        /* Undo what the init code did */

		DeletePreviewContext();
		CloseLibraries();

        IExec->Remove( (struct Node *)libBase );
        IExec->DeleteLibrary( (struct Library *)libBase );

		IExec->Release();
		IExec = NULL;
    }
    else
    {
        result = NULL;
        libBase->lib_Flags |= LIBF_DELEXP;
    }

    return( result );
}
///

/* -- Blanker Interface -- */

/// _blanker_Obtain
uint32 _blanker_Obtain(struct BlankerModuleIFace *Self)
{

#if DebugLevel > 0
	IExec->DebugPrintF( "--> _blanker_Obtain()\n" );
#endif

    Self->Data.RefCount++;

    return( Self->Data.RefCount );
}
///

/// _blanker_Release
uint32 _blanker_Release(struct BlankerModuleIFace *Self)
{

#if DebugLevel > 0
	IExec->DebugPrintF( "--> _blanker_Release()\n" );
#endif

    Self->Data.RefCount--;

	if ( Self->Data.RefCount == 0 )
	{
		Self->Expunge();
	}

    return( Self->Data.RefCount );
}
///

/// _blanker_Expunge
uint32 _blanker_Expunge(struct BlankerModuleIFace *Self)
{

#if DebugLevel > 0
	IExec->DebugPrintF( "--> _blanker_Expunge()\n" );
#endif

	/*
	** Expunge just frees up the memory of the interface.
	** If the instance data contains more resources, like other libraries
	** or memory areas, they must be freed here as well...
	*/

	IExec->FreeVec( (void *)((uint32)Self-Self->Data.NegativeSize) );

	return( TRUE );
}
///

/// _blanker_Clone
struct Interface *_blanker_Clone(struct BlankerModuleIFace *Self )
{
	struct Interface *IFace;
	struct BlankerData *bd;

#if DebugLevel > 0
	IExec->DebugPrintF( "--> _blanker_Clone()\n" );
#endif

    /*
    ** The Clone method will create a copy of itself.
    ** The instance data should be initialized here.
    */

	IFace = IExec->MakeInterface( Self->Data.LibBase, Blanker_Tags );

	if ( IFace )
	{
		bd = (struct BlankerData *)((uint32)IFace-IFace->Data.NegativeSize);

		IFace->Data.Flags |= IFLF_CLONED;

		memset( bd, 0, sizeof( struct BlankerData ));

		bd->currentBlankingMode = SBBM_NoBlanking;

		ResetSettingsToDefault( bd );
	}

	return( IFace );
}
///

/// _blanker_Get
BOOL _blanker_Get(struct BlankerModuleIFace *Self, uint32 msgType, uint32 *msgData)
{
	struct BlankerData *bd = (struct BlankerData *)((uint32)Self - Self->Data.NegativeSize);
	BOOL result = TRUE;

#if DebugLevel > 0
	IExec->DebugPrintF( "--> _blanker_Get()\n" );
#endif

	switch( msgType )
	{
		case BMGET_Version:
		{
#if DebugLevel > 0
	IExec->DebugPrintF( "    BMGET_Version\n" );
#endif
			*msgData = BLANKERMODULEVERSION;
			break;
		}

		case BMGET_ModuleAuthorInfo:
		{
#if DebugLevel > 0
	IExec->DebugPrintF( "    BMGET_ModuleAuthorInfo\n" );
#endif
			*msgData = (uint32)"Steen Lund <steen@munk-lund.dk>";
			break;
		}

		case BMGET_ModuleVersionInfo:
		{
#if DebugLevel > 0
	IExec->DebugPrintF( "    BMGET_ModuleVersionInfo\n" );
#endif
			*msgData = (uint32)LIBNAME" "LIBVERS" ("__AMIGADATE__")";
			break;
		}

		case BMGET_RenderMode:
		{
#if DebugLevel > 0
	IExec->DebugPrintF( "    BMGET_RenderMode\n" );
#endif
			*msgData = BMRM_CustomScreen;
			break;
		}

	    case BMGET_PreferencesType:
	    {
#if DebugLevel > 0
	IExec->DebugPrintF( "    BMGET_PreferenceType\n" );
#endif
			*msgData = BMPT_Custom;
		    break;
	    }

	    case BMGET_Preferences:
	    {
#if DebugLevel > 0
	IExec->DebugPrintF( "    BMGET_Preferences\n" );
#endif
		    PrefsObject *dict = (PrefsObject *)msgData;

		    if ( dict )
		    {
				// Set Settings i Application libraries
				SetBlankerPrefs(bd, dict);
		    }
		    break;
	    }

		default:
		{
			result = FALSE;
			break;
		}
	}

	return( result );
}
///

/// _blanker_Set
BOOL _blanker_Set(struct BlankerModuleIFace *Self, uint32 msgType, uint32 msgData)
{
	struct BlankerData *bd = (struct BlankerData *)((uint32)Self - Self->Data.NegativeSize);
	BOOL result = TRUE;

#if DebugLevel > 0
	IExec->DebugPrintF( "--> _blanker_Set()\n" );
#endif

	switch( msgType )
	{
		case BMSET_BlankerRender:
		{
#if DebugLevel > 0
	IExec->DebugPrintF( "    BMSET_BlankerRender\n" );
#endif
			// only store a pointer!
			// we are running in the provided task
			bd->blankerRender = (struct BlankerRender *)msgData;
			break;
		}

		case BMSET_BlankingMode:
		{
#if DebugLevel > 0
	IExec->DebugPrintF( "    BMSET_BlankingMode\n" );
#endif
			switch( msgData )
			{
				case SBBM_NoBlanking:
				case SBBM_Preview:
				case SBBM_Blanking:
				case SBBM_DPMS_Standby:
				case SBBM_DPMS_Suspend:
				case SBBM_DPMS_PowerOff:
				{
					bd->currentBlankingMode = msgData;
					result = TRUE;
					break;
				}

				default:
				{
					result = FALSE;
					break;
				}
			}
			break;
		}

		case BMSET_OpenPrefsWindow:
		{
#if DebugLevel > 0
	IExec->DebugPrintF( "    BMSET_OpenPrefsWindow\n" );
#endif
			result = MakeGUI(bd , (struct BlankerPrefsWindowSetup *)msgData );
			UpdateWindowSettings(bd);
			break;
		}

		case BMSET_ClosePrefsWindow:
		{
#if DebugLevel > 0
	IExec->DebugPrintF( "    BMSET_ClosePrefsWindow\n" );
#endif
			DestroyGUI();
			memset( &bd->WinInfo, 0, sizeof( bd->WinInfo ));
			break;
		}

		case BMSET_PrefsWindowInfo:
		{
#if DebugLevel > 0
	IExec->DebugPrintF( "    BMSET_PrefsWindowInfo\n" );
#endif
			bd->WinInfo = *((struct BlankerPrefsWindowInfo *)msgData);
			break;
		}

		case BMSET_Preferences:
		{
#if DebugLevel > 0
	IExec->DebugPrintF( "    BMSET_Preferences\n" );
#endif
			PrefsObject *dict = (PrefsObject *)msgData;

		    if ( dict )
		    {
				// Her henter vi Settings fra Application libraryet
				GetBlankerPrefs(bd, dict);

				// Set Refresh bit'en s� Blankeren genindl�ser ops�tningen
			    bd->refetchSettings = TRUE;
		    }
			break;
		}

		case BMSET_UseDefaultSettings:
		{
#if DebugLevel > 0
	IExec->DebugPrintF( "    BMSET_UseDefaultSettings\n" );
#endif
			ResetSettingsToDefault( bd );
			UpdateWindowSettings( bd );
			bd->refetchSettings = TRUE;
			break;
		}

		default:
		{
			result = FALSE;
			break;
		}
	}

	return( result );
}
///

/// _blanker_Blank
void DeletePreviewContext();
void _blanker_Blank(struct BlankerModuleIFace *Self )
{
	struct BlankerData *bd;

#if DebugLevel > 0
	IExec->DebugPrintF( "--> _blanker_Blank()\n" );
#endif

    bd = (struct BlankerData *)((uint32)Self - Self->Data.NegativeSize);

    if ( bd->blankerRender->isPreview )
    {
	    RenderPreview( bd );
		DeletePreviewContext();
    }
    else
    {
	    RenderScreen( bd );
    }
}
///

/* -- Internal Functions -- */
/// Open Libraries
uint32 OpenLibraries( void )
{
	// Open Libraries and Interfaces
	NewlibBase = IExec->OpenLibrary("newlib.library", 3);
	INewlib = IExec->GetInterface(NewlibBase, "main", 1, NULL);
	if (INewlib == NULL)
	{
		return FALSE;
	}

	ApplicationBase = IExec->OpenLibrary( "application.library", 53 );
	IApplication = (struct ApplicationIFace *)IExec->GetInterface(ApplicationBase, "application", 2, NULL);
	if (IApplication == NULL)
	{
		return FALSE;
	}

	IPrefsObjects = (struct PrefsObjectsIFace *)IExec->GetInterface(ApplicationBase, "prefsobjects", 2, NULL);
	if (IPrefsObjects == NULL)
	{
		return FALSE;
	}

	CyberGfxBase = IExec->OpenLibrary("cybergraphics.library", 42);
	ICyberGfx = (struct CyberGfxIFace *)IExec->GetInterface(CyberGfxBase, "main", 1, NULL);
	if (ICyberGfx == NULL)
	{
		return FALSE;
	}

	DOSBase = IExec->OpenLibrary( "dos.library", 51 );
	IDOS = (struct DOSIFace *)IExec->GetInterface( DOSBase, "main", 1, NULL );
	if ( IDOS == NULL )
	{
		return( FALSE );
	}

	GfxBase = IExec->OpenLibrary( "graphics.library", 51 );
	IGraphics = (struct GraphicsIFace *)IExec->GetInterface( GfxBase, "main", 1, NULL );
	if ( IGraphics == NULL )
	{
		return( FALSE );
	}

	IntuitionBase = IExec->OpenLibrary( "intuition.library", 51 );
	IIntuition = (struct IntuitionIFace *)IExec->GetInterface( IntuitionBase, "main", 1, NULL );
	if ( IIntuition == NULL )
	{
		return( FALSE );
	}

	P96Base = IExec->OpenLibrary( "Picasso96API.library", 2 );
	IP96 = (struct P96IFace *)IExec->GetInterface( P96Base, "main", 1, NULL );

	if ( IP96 == NULL )
	{
		return( FALSE );
	}

	UtilityBase = IExec->OpenLibrary( "utility.library", 51 );
	IUtility = (struct UtilityIFace *)IExec->GetInterface( UtilityBase, "main", 1, NULL );

	if ( IUtility == NULL )
	{
		return( FALSE );
	}

	MiniGLBase = IExec->OpenLibrary("minigl.library", 0L);
	IMiniGL =(struct MiniGLIFace*)IExec->GetInterface(MiniGLBase, "main", 1, NULL);
	if (IMiniGL == NULL) return FALSE;

	TimeIO = IExec->AllocVecTags(sizeof(struct TimeRequest), AVT_ClearWithValue, 0, TAG_DONE);
	IExec->OpenDevice(TIMERNAME, UNIT_MICROHZ, (struct IORequest*)TimeIO, 0);
	TimerBase = TimeIO->Request.io_Device;
	ITimer = (struct TimerIFace *)IExec->GetInterface((struct Library*)TimerBase, "main", 1, NULL);
	if (ITimer == NULL)
	{
		return FALSE;
	}

	return OpenGUILibraries();
}
///

/// Close Libraries
void CloseLibraries()
{
	CloseGUILibraries();

	if (ITimer) IExec->DropInterface((struct Interface*)ITimer);
	ITimer = NULL;

	if (TimerBase) IExec->CloseDevice((struct IORequest*)TimeIO);
	TimerBase = NULL;

	if (TimeIO) IExec->FreeVec(TimeIO);
	TimeIO = NULL;

	// Close Libraries
	if (IMiniGL) IExec->DropInterface((struct Interface*)IMiniGL);
	IMiniGL = NULL;

	if (MiniGLBase) IExec->CloseLibrary(MiniGLBase);
	MiniGLBase = NULL;

	if (IUtility) IExec->DropInterface((struct Interface *)IUtility);
	IUtility = NULL;

	if (UtilityBase) IExec->CloseLibrary(UtilityBase);
	UtilityBase = NULL;

	if (IP96) IExec->DropInterface((struct Interface *)IP96);
	IP96 = NULL;

	if (P96Base) IExec->CloseLibrary(P96Base);
	P96Base = NULL;

	if (IIntuition) IExec->DropInterface((struct Interface *)IIntuition);
	IIntuition = NULL;

	if (IntuitionBase) IExec->CloseLibrary(IntuitionBase);
	IntuitionBase = NULL;

	if (IGraphics) IExec->DropInterface((struct Interface *)IGraphics);
	IGraphics = NULL;

	if (GfxBase) IExec->CloseLibrary(GfxBase);
	GfxBase = NULL;

	if (IDOS) IExec->DropInterface((struct Interface *)IDOS);
	IDOS = NULL;

	if (DOSBase) IExec->CloseLibrary(DOSBase);
	DOSBase = NULL;

	if (ICyberGfx) IExec->DropInterface((struct Interface *)ICyberGfx);
	ICyberGfx = NULL;

	if (CyberGfxBase) IExec->CloseLibrary(CyberGfxBase);
	CyberGfxBase = NULL;

	if (IPrefsObjects) IExec->DropInterface((struct Interface *)IPrefsObjects);
	IPrefsObjects = NULL;

	if (IApplication) IExec->DropInterface((struct Interface *)IApplication);
	IApplication = NULL;

	if (ApplicationBase) IExec->CloseLibrary(ApplicationBase);
	ApplicationBase = NULL;

	if (INewlib) IExec->DropInterface(INewlib);
	if (NewlibBase) IExec->CloseLibrary(NewlibBase);
	NewlibBase = NULL;
}
///

/// RenderPreview
struct BitMap* previewBitMap = NULL;
BOOL CreatePreviewContext(struct BlankerData* bd, int w, int h)
{
	int depth = IP96->p96GetBitMapAttr(bd->blankerRender->rp->BitMap, P96BMA_DEPTH);
	previewBitMap = IP96->p96AllocBitMap(w, h, depth, BMF_DISPLAYABLE, bd->blankerRender->rp->BitMap, 0);

	if (previewBitMap != NULL)
	{
		PreviewContext = IMiniGL->CreateContextTags( MGLCC_Bitmap, previewBitMap, TAG_END);

		if (PreviewContext != NULL)
		{
			mglMakeCurrent(PreviewContext);
			mglLockMode(MGL_LOCK_MANUAL);
			return TRUE;
		}
	}

	return FALSE;
}

void DeletePreviewContext()
{
	if (PreviewContext != NULL)
	{
		mglMakeCurrent(PreviewContext);
		mglDeleteContext();
		PreviewContext = NULL;
	}

	if (IP96 != NULL && previewBitMap != NULL)
	{
		IP96->p96FreeBitMap(previewBitMap);
		previewBitMap = NULL;
	}
}
///

/// SetBlankingMode
void SetBlankingMode( struct Screen *screen, uint32 mode )
{
	if ( screen )
	{
		switch ( mode )
		{
			case SBBM_Preview:
			case SBBM_NoBlanking:
			case SBBM_Blanking:
			{
				ICyberGfx->CVideoCtrlTags( (struct ViewPort *)&screen->ViewPort,
					SETVC_DPMSLevel, DPMS_ON,
					TAG_DONE
				);
				break;
			}

			case SBBM_DPMS_Standby:
			{
				ICyberGfx->CVideoCtrlTags( (struct ViewPort *)&screen->ViewPort,
					SETVC_DPMSLevel, DPMS_STANDBY,
					TAG_DONE
				);
				break;
			}

			case SBBM_DPMS_Suspend:
			{
				ICyberGfx->CVideoCtrlTags( (struct ViewPort *)&screen->ViewPort,
					SETVC_DPMSLevel, DPMS_SUSPEND,
					TAG_DONE
				);
				break;
			}

			case SBBM_DPMS_PowerOff:
			{
				ICyberGfx->CVideoCtrlTags( (struct ViewPort *)&screen->ViewPort,
					SETVC_DPMSLevel, DPMS_OFF,
					TAG_DONE
				);
				break;
			}

			default:
			{
				break;
			}
		}
	}
}
///

