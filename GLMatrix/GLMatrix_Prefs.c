#include "GLMatrix_Prefs.h"

#include <proto/application.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	PrefsObject* obj = NULL;

	obj = IPrefsObjects->DictGetObjectForKey(dict, "Density");
	IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetLong, &bd->density, TAG_DONE);

	obj = IPrefsObjects->DictGetObjectForKey(dict, "Speed");
	IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetLong, &bd->speed, TAG_DONE);

	obj = IPrefsObjects->DictGetObjectForKey(dict, "Encoding");
	IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetLong, &bd->encoding, TAG_DONE);

	obj = IPrefsObjects->DictGetObjectForKey( dict, "ScreenMode" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &bd->screenmodeID, TAG_DONE );

	obj = IPrefsObjects->DictGetObjectForKey(dict, "Fog");
	IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetBool, &bd->fog, TAG_DONE);

	obj = IPrefsObjects->DictGetObjectForKey(dict, "Wave");
	IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetBool, &bd->wave, TAG_DONE);

	obj = IPrefsObjects->DictGetObjectForKey(dict, "Rotate");
	IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetBool, &bd->rotate, TAG_DONE);

	obj = IPrefsObjects->DictGetObjectForKey(dict, "ShowClock");
	IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetBool, &bd->clock, TAG_DONE);
}


void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->density, TAG_DONE ),
		"Density");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->speed, TAG_DONE ),
		"Speed");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->encoding, TAG_DONE ),
		"Encoding");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->screenmodeID, TAG_DONE ),
		"ScreenMode");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetBool, bd->fog, TAG_DONE ),
		"Fog");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetBool, bd->wave, TAG_DONE ),
		"Wave");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetBool, bd->rotate, TAG_DONE ),
		"Rotate");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetBool, bd->clock, TAG_DONE ),
		"ShowClock");
}


