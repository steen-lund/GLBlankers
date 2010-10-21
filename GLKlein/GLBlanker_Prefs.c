#include "blanker.h"
#include "../Common/GLBlanker_Prefs.h"
#include <proto/application.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	PrefsObject* obj = NULL;
	GET_PREFS_LONG("ScreenMode", bd->screenmodeID)
	GET_PREFS_BOOL("Random", bd->random)
	GET_PREFS_LONG("Speed", bd->speed)
	GET_PREFS_BOOL("DoSpin", bd->do_spin)
	GET_PREFS_BOOL("DoWander", bd->do_wander)
}


void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	SET_PREFS_LONG(bd->screenmodeID, "ScreenMode")
	SET_PREFS_LONG(bd->speed, "Speed")
	SET_PREFS_BOOL(bd->random, "Random")
	SET_PREFS_BOOL(bd->do_spin, "DoSpin")
	SET_PREFS_BOOL(bd->do_wander, "DoWander")
}


