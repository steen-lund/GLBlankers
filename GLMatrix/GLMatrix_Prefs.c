#include "blanker.h"
#include <GLBlanker_Prefs.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	GET_PREFS_LONG("Density", bd->density);
	GET_PREFS_LONG("Speed", bd->speed);
	GET_PREFS_LONG("Encoding", bd->encoding);
	GET_PREFS_LONG("ScreenMode", bd->screenmodeID);
	GET_PREFS_BOOL("Fog", bd->fog);
	GET_PREFS_BOOL("Wave", bd->wave);
	GET_PREFS_BOOL("Rotate", bd->rotate);
	GET_PREFS_BOOL("ShowTime", bd->showTime);
}

void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	SET_PREFS_LONG(bd->density, "Density");
	SET_PREFS_LONG(bd->speed, "Speed");
	SET_PREFS_LONG(bd->encoding, "Encoding");
	SET_PREFS_LONG(bd->screenmodeID, "ScreenMode");
	SET_PREFS_BOOL(bd->fog, "Fog");
	SET_PREFS_BOOL(bd->wave, "Wave");
	SET_PREFS_BOOL(bd->rotate, "Rotate");
	SET_PREFS_BOOL(bd->showTime, "ShowTime");
}
