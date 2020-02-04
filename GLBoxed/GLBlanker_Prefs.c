#include "blanker.h"
#include <GLBlanker_Prefs.h>
#include <proto/graphics.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
    GET_PREFS_LONG("ScreenMode", bd->screenmodeID);
    GET_PREFS_FLOAT("Speed", bd->speed);
    GET_PREFS_LONG("Balls", bd->balls);
    GET_PREFS_FLOAT("BallSize", bd->ballSize);
    GET_PREFS_FLOAT("Explosion", bd->explosion);
    GET_PREFS_FLOAT("Decay", bd->decay);
}

void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
    SET_PREFS_LONG(bd->screenmodeID, "ScreenMode");
    SET_PREFS_FLOAT(bd->speed, "Speed");
    SET_PREFS_LONG(bd->balls, "Balls");
    SET_PREFS_FLOAT(bd->ballSize, "BallSize");
    SET_PREFS_FLOAT(bd->explosion, "Explosion");
    SET_PREFS_FLOAT(bd->decay, "Decay");
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
    bd->speed           = 0.4f;
    bd->balls           = 25;
    bd->ballSize        = 2.0f;
    bd->explosion       = 15.0f;
    bd->decay           = 0.1f;
}
