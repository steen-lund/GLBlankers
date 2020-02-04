#include "blanker.h"
#include <GLBlanker_Prefs.h>
#include <proto/graphics.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
    GET_PREFS_LONG("ScreenMode", bd->screenmodeID);
    GET_PREFS_LONG("Segments", bd->segments);
    GET_PREFS_LONG("Duration", bd->duration);
    GET_PREFS_BOOL("SpinX", bd->spinX);
    GET_PREFS_BOOL("SpinY", bd->spinY);
    GET_PREFS_BOOL("SpinZ", bd->spinZ);
    GET_PREFS_BOOL("Wander", bd->wander);
}

void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
    SET_PREFS_LONG(bd->screenmodeID, "ScreenMode");
    SET_PREFS_LONG(bd->segments, "Segments");
    SET_PREFS_LONG(bd->duration, "Duration");
    SET_PREFS_BOOL(bd->spinX, "SpinX");
    SET_PREFS_BOOL(bd->spinY, "SpinY");
    SET_PREFS_BOOL(bd->spinZ, "SpinZ");
    SET_PREFS_BOOL(bd->wander, "Wander");
}

void ResetSettingsToDefault(struct BlankerData *bd)
{
    uint32 id;

    id = IGraphics->BestModeID(
        BIDTAG_NominalWidth,    640,
        BIDTAG_NominalHeight,   480,
        BIDTAG_Depth,           16,
        TAG_END
    );

    bd->screenmodeID    = id;
    bd->segments        = 800;
    bd->duration        = 8;
    bd->spinX           = TRUE;
    bd->spinY           = TRUE;
    bd->spinZ           = TRUE;
    bd->wander          = TRUE;
    bd->refetchSettings = TRUE;
}
