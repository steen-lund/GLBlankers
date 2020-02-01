#include "blanker.h"
#include <GLBlanker_Prefs.h>
#include <proto/graphics.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
    GET_PREFS_LONG("ScreenMode", bd->screenmodeID);
    GET_PREFS_LONG("HoldTime", bd->holdtime);
    GET_PREFS_LONG("ChangeTime", bd->changetime);

    GET_PREFS_BOOL("Fog", bd->dofog);
    GET_PREFS_BOOL("Texture", bd->dotexture);
}

void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
    SET_PREFS_LONG(bd->screenmodeID, "ScreenMode");
    SET_PREFS_LONG(bd->holdtime, "HoldTime");
    SET_PREFS_LONG(bd->changetime, "ChangeTime");

    SET_PREFS_BOOL(bd->dofog, "Fog");
    SET_PREFS_BOOL(bd->dotexture, "Texture");
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
    bd->holdtime        = 0;
    bd->changetime      = 0;
    bd->dotexture       = TRUE;
    bd->dofog           = FALSE;
    bd->refetchSettings = TRUE;
}
