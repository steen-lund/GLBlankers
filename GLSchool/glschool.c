/* glschool.c, Copyright (c) 2005-2006 David C. Lambert <dcl@panix.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */
#include "blanker.h"
#include "glschool.h"
#include <stdlib.h>
#include "colors.h"
#include <proto/exec.h>

static int			NFish = 100;
static Bool			DoFog = FALSE;
static Bool			DoDrawBBox = TRUE;
static Bool			DoDrawGoal = FALSE;
static int			GoalChgFreq = 50;
static float		MinVel = 1.0f;
static float		MaxVel = 7.0f;
static float		DistExp = 2.2f;
static float		AccLimit = 8.0f;
static float		AvoidFact = 1.5f;
static float		MatchFact = 0.15f;
static float		TargetFact = 80.0f;
static float		CenterFact = 0.1f;
static float		MinRadius = 30.0f;
static float		Momentum = 0.9f;
static float		DistComp = 10.0f;

/*
static XrmOptionDescRec opts[] = {
	{ "-nfish",		".nfish",		XrmoptionSepArg, 0 },
	{ "-fog",		".fog",			XrmoptionNoArg, "True" },
	{ "+fog",		".fog",			XrmoptionNoArg, "False" },
	{ "-drawgoal",	".drawgoal",	XrmoptionNoArg, "True" },
	{ "+drawgoal",	".drawgoal",	XrmoptionNoArg, "False" },
	{ "-drawbbox",	".drawbbox",	XrmoptionNoArg, "True" },
	{ "+drawbbox",	".drawbbox",	XrmoptionNoArg, "False" },
	{ "-goalchgf",	".goalchgf",	XrmoptionSepArg, 0 },
	{ "-maxvel",	".maxvel",		XrmoptionSepArg, 0 },
	{ "-minvel",	".minvel",		XrmoptionSepArg, 0 },
	{ "-acclimit",	".acclimit",	XrmoptionSepArg, 0 },
	{ "-distexp",	".distexp",		XrmoptionSepArg, 0 },
	{ "-avoidfact",	".avoidfact",	XrmoptionSepArg, 0 },
	{ "-matchfact",	".matchfact",	XrmoptionSepArg, 0 },
	{ "-centerfact",".centerfact",	XrmoptionSepArg, 0 },
	{ "-targetfact",".targetfact",	XrmoptionSepArg, 0 },
	{ "-minradius",	".minradius",	XrmoptionSepArg, 0 },
	{ "-distcomp",	".distcomp",	XrmoptionSepArg, 0 },
	{ "-momentum",	".momentum",	XrmoptionSepArg, 0 },
};

static argtype vars[] = {
	{&NFish,		"nfish",		"NFish",		"100", t_Int},
	{&DoFog,		"fog",			"DoFog",		"False", t_Bool},
	{&DoDrawBBox,	"drawbbox",		"DoDrawBBox",	"True", t_Bool},
	{&DoDrawGoal,	"drawgoal",		"DoDrawGoal",	"False", t_Bool},
	{&GoalChgFreq,	"goalchgf",		"GoalChgFreq",	"50",  t_Int},
	{&MaxVel,		"maxvel",		"MaxVel",		"7.0",  t_Float},
	{&MinVel,		"minvel",		"MinVel",		"1.0",	t_Float},
	{&AccLimit,		"acclimit",		"AccLimit",		"8.0",  t_Float},
	{&DistExp,		"distexp",		"DistExp",		"2.2",  t_Float},
	{&AvoidFact,	"avoidfact",	"AvoidFact",	"1.5",  t_Float},
	{&MatchFact,	"matchfact",	"MatchFact",	"0.15",  t_Float},
	{&CenterFact,	"centerfact",	"CenterFact",	"0.1",  t_Float},
	{&TargetFact,	"targetfact",	"TargetFact",	"80",  t_Float},
	{&MinRadius,	"minradius",	"MinRadius",	"30.0",  t_Float},
	{&Momentum,		"momentum",		"Momentum",		"0.9",  t_Float},
	{&DistComp,		"distcomp",		"DistComp",		"10.0",  t_Float},
};

ENTRYPOINT ModeSpecOpt glschool_opts = {countof(opts), opts, countof(vars), vars, NULL};
*/
typedef struct {
	int			nColors;
	int			rotCounter;
	int			goalCounter;
	Bool		drawGoal;
	Bool		drawBBox;			
	GLuint		bboxList;
	GLuint		goalList;
	GLuint		fishList;
	XColor		*colors;
	School		*school;
} glschool_configuration;

static glschool_configuration glSchoolConf;

void reshape_glschool(int width, int height)
{
	double					aspect = (double)width/(double)height;
	glschool_configuration	*sc = &glSchoolConf;

	if (sc->school != (School *)0) {
		setBBox(sc->school, -aspect*160, aspect*160, -130, 130, -450, -50.0);
		glDeleteLists(sc->bboxList, 1);
		createBBoxList(&SCHOOL_BBOX(sc->school), &sc->bboxList, 0);
	}

	reshape(width, height);
}

void init_glschool(struct BlankerData* bd)
{
	glschool_configuration	*sc = &glSchoolConf;

	DoDrawGoal = bd->drawgoal;
	DoDrawBBox = bd->drawbbox;
	DoFog = bd->fog;
	NFish = bd->fish;
	GoalChgFreq = bd->goalchgfreq;
	MinVel = bd->minvel;
	MaxVel = bd->maxvel;
	AccLimit = bd->acclimit;
	DistExp = bd->distcomp;
	AvoidFact = bd->avoidfact;
	MatchFact = bd->matchfact;
	TargetFact = bd->targetfact;
	CenterFact = bd->centerfact;
	MinRadius = bd->minradius;
	Momentum = bd->momentum;
	DistComp = bd->distcomp;

	sc->drawGoal = DoDrawGoal;
	sc->drawBBox = DoDrawBBox;

	sc->nColors = 360;
	sc->colors = (XColor *)IExec->AllocVecTags(sc->nColors * sizeof(XColor), AVT_ClearWithValue, 0, AVT_Type, MEMF_PRIVATE, TAG_DONE);
	make_color_ramp(0, 0,
					0.0, 1.0, 1.0,
					359.0, 1.0, 1.0,
					sc->colors, &sc->nColors,
					False, 0, False);

	sc->school = initSchool(NFish, AccLimit, MaxVel, MinVel, DistExp, Momentum,
							MinRadius, AvoidFact, MatchFact, CenterFact, TargetFact,
							DistComp);
	if (sc->school == (School *)0) {
		//fprintf(stderr, "couldn't initialize TheSchool, exiting\n");
		exit(1);
	}

	initGLEnv(DoFog);
	initFishes(sc->school);
	createDrawLists(&SCHOOL_BBOX(sc->school), &sc->bboxList, &sc->goalList, &sc->fishList, 0);
	computeAccelerations(sc->school);
}

void draw_glschool()
{
	glschool_configuration	*sc = &glSchoolConf;

	if ((sc->goalCounter % GoalChgFreq) == 0)
		newGoal(sc->school);
	sc->goalCounter++;

	sc->rotCounter++;
	sc->rotCounter = (sc->rotCounter%360);

	applyMovements(sc->school);
	drawSchool(sc->colors, sc->school, sc->bboxList, sc->goalList, sc->fishList, sc->rotCounter, sc->drawGoal, sc->drawBBox);
	computeAccelerations(sc->school);
}

void release_glschool()
{
	freeSchool(glSchoolConf.school);
	IExec->FreeVec(glSchoolConf.colors);
}

