#include "blanker.h"
#include "../Common/GLBlanker_Prefs.h"
#include <proto/application.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	PrefsObject* obj = NULL;
	GET_PREFS_LONG("ScreenMode", bd->screenmodeID)
	GET_PREFS_FLOAT("Speed", bd->speed)
	GET_PREFS_LONG("Balls", bd->balls)
	GET_PREFS_FLOAT("BallSize", bd->ballSize)
	GET_PREFS_FLOAT("Explosion", bd->explosion)
	GET_PREFS_FLOAT("Decay", bd->decay)
}


void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	SET_PREFS_LONG(bd->screenmodeID, "ScreenMode")
	SET_PREFS_FLOAT(bd->speed, "Speed")
	SET_PREFS_LONG(bd->balls, "Balls")
	SET_PREFS_FLOAT(bd->ballSize, "BallSize")
	SET_PREFS_FLOAT(bd->explosion, "Explosion")
	SET_PREFS_FLOAT(bd->decay, "Decay")
}


