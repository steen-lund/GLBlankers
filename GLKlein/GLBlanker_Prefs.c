#include "blanker.h"
#include <GLBlanker_Prefs.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	GET_PREFS_LONG("ScreenMode", bd->screenmodeID);
	GET_PREFS_BOOL("Random", bd->random);
	GET_PREFS_LONG("Speed", bd->speed);
	GET_PREFS_BOOL("DoSpin", bd->do_spin);
	GET_PREFS_BOOL("DoWander", bd->do_wander);
}

void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	SET_PREFS_LONG(bd->screenmodeID, "ScreenMode");
	SET_PREFS_LONG(bd->speed, "Speed");
	SET_PREFS_BOOL(bd->random, "Random");
	SET_PREFS_BOOL(bd->do_spin, "DoSpin");
	SET_PREFS_BOOL(bd->do_wander, "DoWander");
}
