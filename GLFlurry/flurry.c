/* -*- Mode: C; tab-width: 4 c-basic-offset: 4 indent-tabs-mode: t -*- */
/*
 * vim: ts=8 sw=4 noet
 */

/*

Copyright (c) 2002, Calum Robinson
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the author nor the names of its contributors may be used
  to endorse or promote products derived from this software without specific
  prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/* flurry */

#define DEF_PRESET     "random"
#define DEF_BRIGHTNESS "8"

#ifdef USE_GL

#include "flurry.h"
#include "blanker.h"
#include <exec/exectags.h>
#include <proto/exec.h>
#include <proto/timer.h>
#include <string.h>

//#define DRAW_SPARKS 1
static char *preset_str = NULL;

global_info_t flurry_info;
static double gTimeCounter = 0.0;

static double currentTime(void) {
	struct TimeVal tv;
	ITimer->GetSysTime(&tv);
	return (double)tv.Seconds + (double)tv.Microseconds / 1000000.0;
}

void OTSetup (void) {
    if (gTimeCounter == 0.0) {
        gTimeCounter = currentTime();
    }
}

double TimeInSecondsSinceStart (void) {
    return currentTime() - gTimeCounter;
}

#ifdef __VEC__
static int IsAltiVecAvailable()
{
	uint32 vectorUnit;
	IExec->GetCPUInfoTags(GCIT_VectorUnit, &vectorUnit, TAG_DONE);
	return vectorUnit != VECTORTYPE_NONE;
}
#endif

static
void delete_flurry_info(flurry_info_t *flurry)
{
    int i;

	IExec->FreeVec(flurry->s);
	IExec->FreeVec(flurry->star);
    for (i=0;i<MAX_SPARKS;i++)
    {
		IExec->FreeVec(flurry->spark[i]);
    }
	IExec->FreeVec(flurry);
}

static
flurry_info_t *new_flurry_info(global_info_t *global, int streams, ColorModes colour, float thickness, float speed, double bf)
{
    int i,k;
	flurry_info_t *flurry = (flurry_info_t *)IExec->AllocVecTags(sizeof(flurry_info_t), AVT_ClearWithValue, 0, TAG_DONE);

    if (!flurry) return NULL;

    flurry->flurryRandomSeed = RandFlt(0.0, 300.0);

	flurry->fOldTime = 0;
	flurry->fTime = TimeInSecondsSinceStart() + flurry->flurryRandomSeed;
 	flurry->fDeltaTime = flurry->fTime - flurry->fOldTime;

    flurry->numStreams = streams;
    flurry->streamExpansion = thickness;
    flurry->currentColorMode = colour;
    flurry->briteFactor = bf;

	flurry->s = IExec->AllocVecTags(sizeof(SmokeV), AVT_ClearWithValue, 0, TAG_DONE);
    InitSmoke(flurry->s);

	flurry->star = IExec->AllocVecTags(sizeof(Star), AVT_ClearWithValue, 0, TAG_DONE);
    InitStar(flurry->star);
    flurry->star->rotSpeed = speed;

    for (i = 0;i < MAX_SPARKS; i++)
    {
		flurry->spark[i] = IExec->AllocVecTags(sizeof(Spark), AVT_ClearWithValue, 0, TAG_DONE);
		InitSpark(flurry->spark[i]);
		flurry->spark[i]->mystery = 1800 * (i + 1) / 13; /* 100 * (i + 1) / (flurry->numStreams + 1); */
		UpdateSpark(global, flurry, flurry->spark[i]);
    }

    for (i=0;i<NUMSMOKEPARTICLES/4;i++) {
	for(k=0;k<4;k++) {
	    flurry->s->p[i].dead.i[k] = 1;
	}
    }

    flurry->next = NULL;

    return flurry;
}

static
void GLSetupRC(global_info_t *global)
{
    /* setup the defaults for OpenGL */
    glDisable(GL_DEPTH_TEST);
    glAlphaFunc(GL_GREATER,0.0f);
    glEnable(GL_ALPHA_TEST);
    glShadeModel(GL_FLAT);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

	//glViewport(0,0,(int) global->sys_glWidth,(int) global->sys_glHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,global->sys_glWidth,0,global->sys_glHeight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0,0.0,0.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnableClientState(GL_COLOR_ARRAY);	
    glEnableClientState(GL_VERTEX_ARRAY);	
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
#ifdef __PPC__
    global->optMode = OPT_MODE_SCALAR_FRSQRTE;

#ifdef __VEC__
	if (IsAltiVecAvailable())
	{
		global->optMode = OPT_MODE_VECTOR_UNROLLED;
	}
#endif

#else
    global->optMode = OPT_MODE_SCALAR_BASE;
#endif
}

static
void GLRenderScene(global_info_t *global, flurry_info_t *flurry, double b)
{
    int i;

    flurry->dframe++;

    flurry->fOldTime = flurry->fTime;
    flurry->fTime = TimeInSecondsSinceStart() + flurry->flurryRandomSeed;
    flurry->fDeltaTime = flurry->fTime - flurry->fOldTime;

    flurry->drag = (float) pow(0.9965,flurry->fDeltaTime*85.0);

    UpdateStar(global, flurry, flurry->star);

#ifdef DRAW_SPARKS
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
#endif

    for (i=0;i<flurry->numStreams;i++) {
	flurry->spark[i]->color[0]=1.0;
	flurry->spark[i]->color[1]=1.0;
	flurry->spark[i]->color[2]=1.0;
	flurry->spark[i]->color[2]=1.0;
	UpdateSpark(global, flurry, flurry->spark[i]);
#ifdef DRAW_SPARKS
	DrawSpark(global, flurry, flurry->spark[i]);
#endif
    }
    switch(global->optMode) {
	case OPT_MODE_SCALAR_BASE:
		UpdateSmoke_ScalarBase(global, flurry, flurry->s);
	    break;
#ifdef __PPC__
	case OPT_MODE_SCALAR_FRSQRTE:
		UpdateSmoke_ScalarFrsqrte(global, flurry, flurry->s);
	    break;
#endif
#ifdef __VEC__
	case OPT_MODE_VECTOR_SIMPLE:
		UpdateSmoke_VectorBase(global, flurry, flurry->s);
	    break;
	case OPT_MODE_VECTOR_UNROLLED:
		UpdateSmoke_VectorUnrolled(global, flurry, flurry->s);
	    break;
#endif

	default:
	    break;
    }

    /* glDisable(GL_BLEND); */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glEnable(GL_TEXTURE_2D);

    switch(global->optMode) {
	case OPT_MODE_SCALAR_BASE:
#ifdef __PPC__
	case OPT_MODE_SCALAR_FRSQRTE:
#endif
	    DrawSmoke_Scalar(global, flurry, flurry->s, b);
	    break;
#ifdef __VEC__
	case OPT_MODE_VECTOR_SIMPLE:
	case OPT_MODE_VECTOR_UNROLLED:
	    DrawSmoke_Vector(global, flurry, flurry->s, b);
	    break;
#endif
	default:
	    break;
    }    

    glDisable(GL_TEXTURE_2D);
}

static void GLResize(global_info_t *global, float w, float h)
{
    global->sys_glWidth = w;
    global->sys_glHeight = h;
}

/* new window size or exposure */
void reshape_flurry(int width, int height)
{
	global_info_t *global = &flurry_info;

    glViewport(0.0, 0.0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glFlush();

    GLResize(global, (float)width, (float)height);
}

void init_flurry(struct BlankerData* bd)
{
    int i;
	global_info_t *global = &flurry_info;

    enum {
	PRESET_INSANE = -1,
        PRESET_WATER = 0,
        PRESET_FIRE,
        PRESET_PSYCHEDELIC,
        PRESET_RGB,
        PRESET_BINARY,
        PRESET_CLASSIC,
        PRESET_MAX
    } preset_num;

	OTSetup();

	switch (bd->style)
    {
        case 0:
			preset_str = "random";
            break;
        case 1:
			preset_str = "water";
            break;
        case 2:
			preset_str = "fire";
            break;
        case 3:
			preset_str = "psychedelic";
            break;
        case 4:
			preset_str = "rgb";
            break;
		case 5:
			preset_str = "binary";
            break;
		case 6:
			preset_str = "classic";
            break;
		case 7:
			preset_str = "insane";
            break;
	}

    if (!preset_str || !*preset_str) preset_str = DEF_PRESET;
    if (!strcmp(preset_str, "random")) {
        preset_num = random() % PRESET_MAX;
    } else if (!strcmp(preset_str, "water")) {
        preset_num = PRESET_WATER;
    } else if (!strcmp(preset_str, "fire")) {
        preset_num = PRESET_FIRE;
    } else if (!strcmp(preset_str, "psychedelic")) {
        preset_num = PRESET_PSYCHEDELIC;
    } else if (!strcmp(preset_str, "rgb")) {
        preset_num = PRESET_RGB;
    } else if (!strcmp(preset_str, "binary")) {
        preset_num = PRESET_BINARY;
    } else if (!strcmp(preset_str, "classic")) {
        preset_num = PRESET_CLASSIC;
    } else if (!strcmp(preset_str, "insane")) {
        preset_num = PRESET_INSANE;
    } else {
		IExec->DebugPrintF("Flurry: unknown preset %s\n", preset_str);
        exit(1);
    }

    global->flurry = NULL;
    switch (preset_num) {
    case PRESET_WATER: {
	for (i = 0; i < 9; i++) {
	    flurry_info_t *flurry;

	    flurry = new_flurry_info(global, 1, blueColorMode, 100.0, 2.0, 2.0);
	    flurry->next = global->flurry;
	    global->flurry = flurry;
	}
        break;
    }
    case PRESET_FIRE: {
	flurry_info_t *flurry;

	flurry = new_flurry_info(global, 12, slowCyclicColorMode, 10000.0, 0.0, 1.0);
	flurry->next = global->flurry;
	global->flurry = flurry;	
        break;
    }
    case PRESET_PSYCHEDELIC: {
	flurry_info_t *flurry;

	flurry = new_flurry_info(global, 10, rainbowColorMode, 200.0, 2.0, 1.0);
	flurry->next = global->flurry;
	global->flurry = flurry;	
        break;
    }
    case PRESET_RGB: {
	flurry_info_t *flurry;

	flurry = new_flurry_info(global, 3, redColorMode, 100.0, 0.8, 1.0);
	flurry->next = global->flurry;
	global->flurry = flurry;	

	flurry = new_flurry_info(global, 3, greenColorMode, 100.0, 0.8, 1.0);
	flurry->next = global->flurry;
	global->flurry = flurry;	

	flurry = new_flurry_info(global, 3, blueColorMode, 100.0, 0.8, 1.0);
	flurry->next = global->flurry;
	global->flurry = flurry;	
        break;
    }
    case PRESET_BINARY: {
	flurry_info_t *flurry;

	flurry = new_flurry_info(global, 16, tiedyeColorMode, 1000.0, 0.5, 1.0);
	flurry->next = global->flurry;
	global->flurry = flurry;

	flurry = new_flurry_info(global, 16, tiedyeColorMode, 1000.0, 1.5, 1.0);
	flurry->next = global->flurry;
	global->flurry = flurry;
        break;
    }
    case PRESET_CLASSIC: {
	flurry_info_t *flurry;

	flurry = new_flurry_info(global, 5, tiedyeColorMode, 10000.0, 1.0, 1.0);
	flurry->next = global->flurry;
	global->flurry = flurry;
        break;
    }
    case PRESET_INSANE: {
	flurry_info_t *flurry;

	flurry = new_flurry_info(global, 64, tiedyeColorMode, 1000.0, 0.5, 0.5);
	flurry->next = global->flurry;
	global->flurry = flurry;

        break;
    }
    default: {
		IExec->DebugPrintF("Flurry: unknown preset %s\n", preset_str);
        exit(1);
    }
    } 

	GLSetupRC(global);
}

int first = 1;
void draw_flurry()
{
    static double oldFrameTime = -1;
	double newFrameTime;
    double deltaFrameTime = 0;
    double brite;
    GLfloat alpha;

	global_info_t *global = &flurry_info;
    flurry_info_t *flurry;

    newFrameTime = currentTime();
    if (oldFrameTime == -1) {
	/* special case the first frame -- clear to black */
	alpha = 1.0;
    } else {
	/* 
	 * this clamps the speed at below 60fps and, here
	 * at least, produces a reasonably accurate 50fps.
	 * (probably part CPU speed and part scheduler).
	 *
	 * Flurry is designed to run at this speed; much higher
	 * than that and the blending causes the display to
	 * saturate, which looks really ugly.
	 */
/*	  if (newFrameTime - oldFrameTime < 1/60.0) {
	    usleep(MAX_(1,(int)(20000 * (newFrameTime - oldFrameTime))));
	    return;
	}*/
		deltaFrameTime = newFrameTime - oldFrameTime;
		alpha = 5.0 * deltaFrameTime;
    }
    oldFrameTime = newFrameTime;

    if (alpha > 0.2) alpha = 0.2;

    if (first) {
	MakeTexture();
	first = 0;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0, 0.0, 0.0, alpha);
    glRectd(0, 0, global->sys_glWidth, global->sys_glHeight);

    brite = pow(deltaFrameTime,0.75) * 10;
    for (flurry = global->flurry; flurry; flurry=flurry->next) {
	GLRenderScene(global, flurry, brite * flurry->briteFactor);
    }

	glFlush();
}

extern int firstTime;
void release_flurry()
{
	global_info_t *global = &flurry_info;
	flurry_info_t *flurry;

	for (flurry = global->flurry; flurry; ) {
		flurry_info_t* temp = flurry->next;
		delete_flurry_info(flurry);
		flurry = temp;
	}

	first = 1;
	firstTime = 1;
	glDeleteTextures(1, &theTexture);
}

#endif
