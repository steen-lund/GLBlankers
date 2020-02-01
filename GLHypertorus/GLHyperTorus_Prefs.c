#include "blanker.h"
#include "GLBlanker_Prefs.h"

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
