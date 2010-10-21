#include "GLFlurry_Prefs.h"

#include <proto/application.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	PrefsObject* obj = NULL;
	obj = IPrefsObjects->DictGetObjectForKey( dict, "ScreenMode" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &bd->screenmodeID, TAG_DONE );

	obj = IPrefsObjects->DictGetObjectForKey( dict, "Style" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &bd->style, TAG_DONE );
}


void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->screenmodeID, TAG_DONE ),
		"ScreenMode");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->style, TAG_DONE ),
		"Style");
}


