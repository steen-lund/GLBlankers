#include "GLHyperTorus_Prefs.h"

#include <proto/application.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	PrefsObject* obj = NULL;

	obj = IPrefsObjects->DictGetObjectForKey( dict, "ScreenMode" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &bd->screenmodeID, TAG_DONE );

	obj = IPrefsObjects->DictGetObjectForKey( dict, "DisplayMode" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &bd->displaymode, TAG_DONE );

	obj = IPrefsObjects->DictGetObjectForKey( dict, "Appearance" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &bd->appearance, TAG_DONE );

	obj = IPrefsObjects->DictGetObjectForKey( dict, "Colors" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &bd->colors, TAG_DONE );

	obj = IPrefsObjects->DictGetObjectForKey( dict, "3DProjection" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &bd->projection3d, TAG_DONE );

	obj = IPrefsObjects->DictGetObjectForKey( dict, "4DProjection" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &bd->projection4d, TAG_DONE );
}


void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->screenmodeID, TAG_DONE ),
		"ScreenMode");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->displaymode, TAG_DONE ),
		"DisplayMode");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->appearance, TAG_DONE ),
		"Appearance");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->colors, TAG_DONE ),
		"Colors");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->projection3d, TAG_DONE ),
		"3DProjection");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->projection4d, TAG_DONE ),
		"4DProjection");
}


