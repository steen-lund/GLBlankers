#ifndef __BLANKER_PREFS_H__
#define __BLANKER_PREFS_H__

#ifndef __BLANKER_H__
#error Include blanker.h before this
#endif

#include <libraries/application.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict);
void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict);

#define GET_PREFS_LONG(NAME, VAR)	{ obj = IPrefsObjects->DictGetObjectForKey( dict, NAME ); \
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetLong, &VAR, TAG_DONE );}

#define GET_PREFS_DOUBLE(NAME, VAR)	  { obj = IPrefsObjects->DictGetObjectForKey( dict, NAME ); \
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetDouble, &VAR, TAG_DONE );}

#define GET_PREFS_FLOAT(NAME, VAR)	 {double dummy = 0.0; \
	 obj = IPrefsObjects->DictGetObjectForKey( dict, NAME ); \
	if (obj != NULL) {IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetDouble, &dummy, TAG_DONE ); \
	VAR = (float)dummy;} }

#define GET_PREFS_BOOL(NAME, VAR)	{ obj = IPrefsObjects->DictGetObjectForKey( dict, NAME ); \
	IPrefsObjects->PrefsNumber( obj, NULL, ALPONUM_GetBool, &VAR, TAG_DONE );}

#define SET_PREFS_LONG(VAR, NAME) {\
	IPrefsObjects->DictSetObjectForKey( dict, \
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetLong, VAR, TAG_DONE ), \
		NAME);}

#define SET_PREFS_FLOAT(VAR, NAME) {double dummy = (double)VAR ; \
	IPrefsObjects->DictSetObjectForKey( dict, \
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetDouble, &dummy, TAG_DONE ), \
		NAME);}

#define SET_PREFS_DOUBLE(VAR, NAME) {\
	IPrefsObjects->DictSetObjectForKey( dict, \
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetDouble, &VAR, TAG_DONE ), \
		NAME);}

#define SET_PREFS_BOOL(VAR, NAME) {\
	IPrefsObjects->DictSetObjectForKey( dict, \
		IPrefsObjects->PrefsNumber( NULL, NULL, ALPONUM_AllocSetBool, VAR, TAG_DONE ), \
		NAME);}

#endif
