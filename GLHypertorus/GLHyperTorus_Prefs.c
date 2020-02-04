#include "blanker.h"
#include "GLBlanker_Prefs.h"
#include <proto/graphics.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
    GET_PREFS_LONG("ScreenMode", bd->screenmodeID);
    GET_PREFS_LONG("DisplayMode", bd->displaymode);
    GET_PREFS_LONG("Appearance", bd->appearance);
    GET_PREFS_LONG("Colors", bd->colors);
    GET_PREFS_LONG("3DProjection", bd->projection3d);
    GET_PREFS_LONG("4DProjection", bd->projection4d);
}

void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
    SET_PREFS_LONG(bd->screenmodeID, "ScreenMode");
    SET_PREFS_LONG(bd->displaymode, "DisplayMode");
    SET_PREFS_LONG(bd->appearance, "Appearance");
    SET_PREFS_LONG(bd->colors, "Colors");
    SET_PREFS_LONG(bd->projection3d, "3DProjection");
    SET_PREFS_LONG(bd->projection4d, "4DProjection");
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
    bd->displaymode     = 1;
    bd->appearance      = 1;
    bd->colors          = 1;
    bd->projection3d    = 0;
    bd->projection4d    = 0;
}
