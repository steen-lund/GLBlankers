#include "blanker.h"
#include <GLBlanker_Prefs.h>
#include <proto/graphics.h>

void GetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
    GET_PREFS_LONG("ScreenMode", bd->screenmodeID);
    GET_PREFS_BOOL("Fog", bd->fog);
    GET_PREFS_BOOL("DrawBBox", bd->drawbbox);
    GET_PREFS_BOOL("DrawGoal", bd->drawgoal);
    GET_PREFS_LONG("Fish", bd->fish);
    GET_PREFS_LONG("GoalChgFreq", bd->goalchgfreq);
    GET_PREFS_FLOAT("MaxVel", bd->maxvel);
    GET_PREFS_FLOAT("MinVel", bd->minvel);
    GET_PREFS_FLOAT("AccLimit", bd->acclimit);
    GET_PREFS_FLOAT("DistExp", bd->distexp);
    GET_PREFS_FLOAT("AvoidFact", bd->avoidfact);
    GET_PREFS_FLOAT("MatchFact", bd->matchfact);
    GET_PREFS_FLOAT("CenterFact", bd->centerfact);
    GET_PREFS_FLOAT("TargetFact", bd->targetfact);
    GET_PREFS_FLOAT("MinRadius", bd->minradius);
    GET_PREFS_FLOAT("Momentum", bd->momentum);
    GET_PREFS_FLOAT("DistComp", bd->distcomp);
}

void SetBlankerPrefs(struct BlankerData* bd, PrefsObject* dict)
{
    SET_PREFS_LONG(bd->screenmodeID, "ScreenMode");
    SET_PREFS_BOOL(bd->fog, "Fog");
    SET_PREFS_BOOL(bd->drawbbox, "DrawBBox");
    SET_PREFS_BOOL(bd->drawgoal, "DrawGoal");
    SET_PREFS_LONG(bd->fish, "Fish");
    SET_PREFS_LONG(bd->goalchgfreq, "GoalChgFreq");
    SET_PREFS_FLOAT(bd->maxvel, "MaxVel");
    SET_PREFS_FLOAT(bd->minvel, "MinVel");
    SET_PREFS_FLOAT(bd->acclimit, "AccLimit");
    SET_PREFS_FLOAT(bd->distexp, "DistExp");
    SET_PREFS_FLOAT(bd->avoidfact, "AvoidFact");
    SET_PREFS_FLOAT(bd->matchfact, "MatchFact");
    SET_PREFS_FLOAT(bd->centerfact, "CenterFact");
    SET_PREFS_FLOAT(bd->targetfact, "TargetFact");
    SET_PREFS_FLOAT(bd->minradius, "MinRadius");
    SET_PREFS_FLOAT(bd->momentum, "Momentum");
    SET_PREFS_FLOAT(bd->distcomp, "DistComp");
}

void ResetSettingsToDefault(struct BlankerData *bd)
{
    uint32 id;

    id = IGraphics->BestModeID(
        BIDTAG_NominalWidth,    640,
        BIDTAG_NominalHeight,   480,
        BIDTAG_Depth,           16,
        TAG_END
    );

    bd->screenmodeID    = id;
    bd->fog             = TRUE;
    bd->drawbbox        = TRUE;
    bd->drawgoal        = FALSE;
    bd->fish            = 100;
    bd->goalchgfreq     = 50;
    bd->maxvel          = 7.0f;
    bd->minvel          = 1.0f;
    bd->acclimit        = 8.0f;
    bd->distexp         = 2.2f;
    bd->avoidfact       = 1.5f;
    bd->matchfact       = 0.15f;
    bd->centerfact      = 0.1f;
    bd->targetfact      = 80.0f;
    bd->minradius       = 30.0f;
    bd->momentum        = 0.9f;
    bd->distcomp        = 10.0f;
}
