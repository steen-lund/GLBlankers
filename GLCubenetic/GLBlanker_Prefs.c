#include "blanker.h"
#include "../Common/GLBlanker_Prefs.h"
#include <proto/application.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	PrefsObject* obj = NULL;
	GET_PREFS_LONG("ScreenMode", bd->screenmodeID)
	GET_PREFS_LONG("Waves", bd->waves)
	GET_PREFS_LONG("Radius", bd->radius)
	GET_PREFS_LONG("Speed", bd->speed)
	GET_PREFS_LONG("Spin", bd->spin)
	GET_PREFS_LONG("TextureSize", bd->texture_size)
	GET_PREFS_BOOL("Texture", bd->texture)
	GET_PREFS_BOOL("Wander", bd->wander)
}

void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	SET_PREFS_LONG(bd->screenmodeID, "ScreenMode")
	SET_PREFS_LONG(bd->waves, "Waves")
	SET_PREFS_LONG(bd->radius, "Radius")
	SET_PREFS_LONG(bd->speed, "Speed")
	SET_PREFS_LONG(bd->spin, "Spin")
	SET_PREFS_BOOL(bd->texture, "Texture")
	SET_PREFS_LONG(bd->texture_size, "TextureSize")
	SET_PREFS_BOOL(bd->wander, "Wander")
}


