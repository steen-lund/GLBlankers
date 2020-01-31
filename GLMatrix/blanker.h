#ifndef _BLANKER_H__
#define _BLANKER_H__
/*------------------------------------------------------------------------*/

#include <libraries/blankermodule.h>
#include <proto/exec.h>
#include <proto/blankermodule.h>

#ifdef __GNUC__
    #ifdef __PPC__
        #pragma pack(2)
    #endif
#elif defined(__VBCC__)
    #pragma amiga-align
#endif

/*------------------------------------------------------------------------*/

struct BlankerBase {
	APTR		lib_Succ;        // Pointer to next (successor)
	APTR    	lib_Pred;        // Pointer to previous (predecessor)
	uint8   	lib_Type;
	int8    	lib_Pri;         // Priority, for sorting
	STRPTR  	lib_Name;        // ID string, null terminated
	uint8   	lib_Flags;       // see below
	uint8   	lib_ABIVersion;  // ABI exported by library
	uint16  	lib_NegSize;     // number of bytes before LIB
	uint16  	lib_PosSize;     // number of bytes after LIB
	uint16  	lib_Version;     // major
	uint16  	lib_Revision;    // minor
	STRPTR  	lib_IdString;    // ASCII identification
	uint32  	lib_Sum;         // the system-calculated checksum
	uint16  	lib_OpenCnt;     // number of current opens
	APTR    	lib_SegmentList;
};

struct BlankerData {
	struct BlankerRender *			blankerRender;
	struct BlankerPrefsWindowInfo	WinInfo;
	uint32 							refetchSettings;
	uint32							currentBlankingMode;
	uint32							density;
	uint32							speed;
	uint32							encoding;
	uint32							screenmodeID;
	BOOL							fog;
	BOOL							wave;
	BOOL							rotate;
	BOOL							showTime;
};

int32				_start( 				STRPTR argstring, int32 arglen, APTR SysBase );

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

void 				FixValues( 				uint32 *min, int32 minLimit, int32 maxLimit );
void 				RenderPreview( 			struct BlankerData *bd );
void 				RenderScreen( 			struct BlankerData *bd );

void				SetBlankingMode(		struct Screen *screen, uint32 mode );

/*------------------------------------------------------------------------*/

#ifdef __GNUC__
    #ifdef __PPC__
        #pragma pack()
    #endif
#elif defined(__VBCC__)
    #pragma default-align
#endif

/*------------------------------------------------------------------------*/
#endif
