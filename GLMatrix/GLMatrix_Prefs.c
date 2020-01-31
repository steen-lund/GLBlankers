#include "blanker.h"
#include <GLBlanker_Prefs.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	PrefsObject* obj = NULL;

	// obj = IPrefsObjects->DictGetObjectForKey(dict, "Density");
	// IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetLong, &bd->density, TAG_DONE);
	GET_PREFS_LONG("Density", bd->density);

	// obj = IPrefsObjects->DictGetObjectForKey(dict, "Speed");
	// IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetLong, &bd->speed, TAG_DONE);
	GET_PREFS_LONG("Speed", bd->speed);

	// obj = IPrefsObjects->DictGetObjectForKey(dict, "Encoding");
	// IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetLong, &bd->encoding, TAG_DONE);
	GET_PREFS_LONG("Encoding", bd->encoding);

	// obj = IPrefsObjects->DictGetObjectForKey( dict, "ScreenMode" );
	// IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &bd->screenmodeID, TAG_DONE );
	GET_PREFS_LONG("ScreenMode", bd->screenmodeID);

	// obj = IPrefsObjects->DictGetObjectForKey(dict, "Fog");
	// IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetBool, &bd->fog, TAG_DONE);
	GET_PREFS_BOOL("Fog", bd->fog);

	// obj = IPrefsObjects->DictGetObjectForKey(dict, "Wave");
	// IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetBool, &bd->wave, TAG_DONE);
	GET_PREFS_BOOL("Wave", bd->wave);

	// obj = IPrefsObjects->DictGetObjectForKey(dict, "Rotate");
	// IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetBool, &bd->rotate, TAG_DONE);
	GET_PREFS_BOOL("Rotate", bd->rotate);

	// obj = IPrefsObjects->DictGetObjectForKey(dict, "ShowTime");
	// IPrefsObjects->PrefsNumber(obj, NULL, ALPONUM_GetBool, &bd->showTime, TAG_DONE);
	GET_PREFS_BOOL("ShowTime", bd->showTime);
}


void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	// IPrefsObjects->DictSetObjectForKey( dict,
	// 	IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->density, TAG_DONE ),
	// 	"Density");
	SET_PREFS_LONG(bd->density, "Density");

	// IPrefsObjects->DictSetObjectForKey( dict,
	// 	IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->speed, TAG_DONE ),
	// 	"Speed");
	SET_PREFS_LONG(bd->speed, "Speed");

	// IPrefsObjects->DictSetObjectForKey( dict,
	// 	IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->encoding, TAG_DONE ),
	// 	"Encoding");
	SET_PREFS_LONG(bd->encoding, "Encoding");

	// IPrefsObjects->DictSetObjectForKey( dict,
	// 	IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, bd->screenmodeID, TAG_DONE ),
	// 	"ScreenMode");
	SET_PREFS_LONG(bd->screenmodeID, "ScreenMode");

	// IPrefsObjects->DictSetObjectForKey( dict,
	// 	IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetBool, bd->fog, TAG_DONE ),
	// 	"Fog");
	SET_PREFS_BOOL(bd->fog, "Fog");

	// IPrefsObjects->DictSetObjectForKey( dict,
	// 	IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetBool, bd->wave, TAG_DONE ),
	// 	"Wave");
	SET_PREFS_BOOL(bd->wave, "Wave");

	// IPrefsObjects->DictSetObjectForKey( dict,
	// 	IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetBool, bd->rotate, TAG_DONE ),
	// 	"Rotate");
	SET_PREFS_BOOL(bd->rotate, "Rotate");

	// IPrefsObjects->DictSetObjectForKey( dict,
	// 	IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetBool, bd->showTime, TAG_DONE ),
	// 	"ShowTime");
	SET_PREFS_BOOL(bd->showTime, "ShowTime");
}


