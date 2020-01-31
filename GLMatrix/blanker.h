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
