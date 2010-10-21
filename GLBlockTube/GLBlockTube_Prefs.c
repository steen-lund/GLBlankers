#include "GLBlockTube_Prefs.h"

#include <proto/application.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	PrefsObject* obj = NULL;
	obj = IPrefsObjects->DictGetObjectForKey( dict, "ScreenMode" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &bd->screenmodeID, TAG_DONE );

	obj = IPrefsObjects->DictGetObjectForKey( dict, "HoldTime" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &bd->holdtime, TAG_DONE );

	obj = IPrefsObjects->DictGetObjectForKey( dict, "ChangeTime" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &bd->changetime, TAG_DONE );

	obj = IPrefsObjects->DictGetObjectForKey( dict, "Fog" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetBool, &bd->dofog, TAG_DONE );

	obj = IPrefsObjects->DictGetObjectForKey( dict, "Texture" );
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetBool, &bd->dotexture, TAG_DONE );
}


void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->screenmodeID, TAG_DONE ),
		"ScreenMode");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->holdtime, TAG_DONE ),
		"HoldTime");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->changetime, TAG_DONE ),
		"ChangeTime");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetBool, bd->dofog, TAG_DONE ),
		"Fog");

	IPrefsObjects->DictSetObjectForKey( dict,
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetBool, bd->dotexture, TAG_DONE ),
		"Texture");
}


