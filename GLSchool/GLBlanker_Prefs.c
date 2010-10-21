#include "blanker.h"
#include "../Common/GLBlanker_Prefs.h"
#include <proto/application.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	PrefsObject* obj = NULL;
	GET_PREFS_LONG("ScreenMode", bd->screenmodeID)
	GET_PREFS_BOOL("Fog", bd->fog)
	GET_PREFS_BOOL("DrawBBox", bd->drawbbox)
	GET_PREFS_BOOL("DrawGoal", bd->drawgoal)
	GET_PREFS_LONG("Fish", bd->fish)
	GET_PREFS_LONG("GoalChgFreq", bd->goalchgfreq)
	GET_PREFS_FLOAT("MaxVel", bd->maxvel)
	GET_PREFS_FLOAT("MinVel", bd->minvel)
	GET_PREFS_FLOAT("AccLimit", bd->acclimit)
	GET_PREFS_FLOAT("DistExp", bd->distexp)
	GET_PREFS_FLOAT("AvoidFact", bd->avoidfact)
	GET_PREFS_FLOAT("MatchFact", bd->matchfact)
	GET_PREFS_FLOAT("CenterFact", bd->centerfact)
	GET_PREFS_FLOAT("TargetFact", bd->targetfact)
	GET_PREFS_FLOAT("MinRadius", bd->minradius)
	GET_PREFS_FLOAT("Momentum", bd->momentum)
	GET_PREFS_FLOAT("DistComp", bd->distcomp)
}

void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
	SET_PREFS_LONG(bd->screenmodeID, "ScreenMode");
	SET_PREFS_BOOL(bd->fog, "Fog")
	SET_PREFS_BOOL(bd->drawbbox, "DrawBBox")
	SET_PREFS_BOOL(bd->drawgoal, "DrawGoal")
	SET_PREFS_LONG(bd->fish, "Fish")
	SET_PREFS_LONG(bd->goalchgfreq, "GoalChgFreq")
	SET_PREFS_FLOAT(bd->maxvel, "MaxVel")
	SET_PREFS_FLOAT(bd->minvel, "MinVel")
	SET_PREFS_FLOAT(bd->acclimit, "AccLimit")
	SET_PREFS_FLOAT(bd->distexp, "DistExp")
	SET_PREFS_FLOAT(bd->avoidfact, "AvoidFact")
	SET_PREFS_FLOAT(bd->matchfact, "MatchFact")
	SET_PREFS_FLOAT(bd->centerfact, "CenterFact")
	SET_PREFS_FLOAT(bd->targetfact, "TargetFact")
	SET_PREFS_FLOAT(bd->minradius, "MinRadius")
	SET_PREFS_FLOAT(bd->momentum, "Momentum")
	SET_PREFS_FLOAT(bd->distcomp, "DistComp")
}


