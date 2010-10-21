#include "../Common/GLBlanker_Prefs.h"

#include <proto/application.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	PrefsObject* obj = NULL;
	GET_PREFS_LONG("ScreenMode", bd->screenmodeID)
}


void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	SET_PREFS_LONG(bd->screenmodeID, "ScreenMode")
}


