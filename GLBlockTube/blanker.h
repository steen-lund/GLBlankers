#pragma once
#define __BLANKER_H__

#include <libraries/blankermodule.h>

#ifdef __GNUC__
    #ifdef __PPC__
        #pragma pack(2)
    #endif
#elif defined(__VBCC__)
    #pragma amiga-align
#endif

struct BlankerData {
    struct BlankerRender*           blankerRender;
    struct BlankerPrefsWindowInfo   WinInfo;
    uint32                          refetchSettings;
    uint32                          currentBlankingMode;
    uint32                          screenmodeID;
    uint32                          holdtime;
    uint32                          changetime;
    BOOL                            dotexture;
    BOOL                            dofog;
};

#ifdef __GNUC__
    #ifdef __PPC__
        #pragma pack()
    #endif
#elif defined(__VBCC__)
    #pragma default-align
#endif
