#include "blanker.h"
#include <GLBlanker_Prefs.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	GET_PREFS_LONG("ScreenMode", bd->screenmodeID);
}

void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	SET_PREFS_LONG(bd->screenmodeID, "ScreenMode");
}
