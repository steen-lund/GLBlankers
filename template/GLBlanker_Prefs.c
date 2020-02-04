#include "blanker.h"
#include <GLBlanker_Prefs.h>
#include <proto/graphics.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
    GET_PREFS_LONG("ScreenMode", bd->screenmodeID);
}

void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
    SET_PREFS_LONG(bd->screenmodeID, "ScreenMode");
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

    bd->screenmodeID = id;
}
