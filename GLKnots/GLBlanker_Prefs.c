#include "blanker.h"
#include <GLBlanker_Prefs.h>

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
