/* blocktube, Copyright (c) 2003 Lars Damerow <lars@oddment.org>
 *
 * Based on Jamie Zawinski's original dangerball code.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 */

#define DEBUG 0

#define MAX_ENTITIES     400
#define DEF_TEXTURE     "True"
#define DEF_FOG         "True"

#include "xi_image.h"
#include "yarandom.h"
#include "blanker.h"
#include <math.h>
#include <ctype.h>
#include <stdio.h>

#include <gl/gl.h>
#include <proto/utility.h>

typedef struct {
    int id, r, g, b;
    GLfloat tVal;
    int age;
    int lifetime;
    GLfloat position[3];
    GLfloat angle;
    GLfloat angularVelocity;
} entity;

typedef struct {
  GLuint  block_dlist;
  int nextID;

  entity entities[MAX_ENTITIES];
  float targetR, targetG, targetB,
    currentR, currentG, currentB,
    deltaR, deltaG, deltaB;
  int counter;
  int changing;
  GLfloat zoom;
  GLfloat tilt;
  GLuint envTexture;
  XImage *texti;

  GLfloat tunnelLength;
  GLfloat tunnelWidth;

} blocktube_configuration;

static blocktube_configuration lps;

/*
static int counter = 0;
static int changing = 0;
static GLfloat zoom = 30.0f;
static GLfloat tilt = 4.5f;
static GLuint loop;
static GLuint envTexture;
static XImage *texti = NULL;
GLfloat tunnelLength=200;
GLfloat tunnelWidth=5;*/

static GLint holdtime = 0;
static GLint changetime = 0;
static BOOL do_texture = TRUE;
static BOOL do_fog = FALSE;

extern unsigned char blocktube[];

static BOOL LoadGLTextures(void)
{
	blocktube_configuration *lp = &lps;
    BOOL status;

    status = TRUE;
    glGenTextures(1, &lp->envTexture);
    glBindTexture(GL_TEXTURE_2D, lp->envTexture);

	lp->texti = inmemory_png_to_ximage(blocktube);
	if (!lp->texti)
    {
        status = FALSE;
    }
    else
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, lp->texti->width, lp->texti->height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, lp->texti->data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	}

    return status;
}

static void newTargetColor(blocktube_configuration *lp)
{
    int luminance = 0;

    while (luminance <= 150) {
        lp->targetR = random() % 256;
        lp->targetG = random() % 256;
        lp->targetB = random() % 256;
        lp->deltaR = (lp->targetR - lp->currentR) / changetime;
        lp->deltaG = (lp->targetG - lp->currentG) / changetime;
        lp->deltaB = (lp->targetB - lp->currentB) / changetime;
        luminance = 0.3 * lp->targetR + 0.59 * lp->targetG + 0.11 * lp->targetB;
    }
}

static void randomize_entity(blocktube_configuration *lp, entity *ent)
{
	ent->id = lp->nextID++;
    ent->tVal = 1 - ((float)random() / RAND_MAX / 1.5);
    ent->age = 0;
    ent->lifetime = 100;
    ent->angle = random() % 360;
    ent->angularVelocity = 0.5-((float)(random()) / RAND_MAX);
	ent->position[0] = (float)(random()) / RAND_MAX + lp->tunnelWidth;
    ent->position[1] = (float)(random()) / RAND_MAX * 2;
	ent->position[2] = -(float)(random()) / RAND_MAX * lp->tunnelLength;
}

static void entityTick(blocktube_configuration *lp, entity *ent)
{
    ent->angle += ent->angularVelocity;
    ent->position[2] += 0.1;
	if (ent->position[2] > lp->zoom)
    {
		ent->position[2] = -lp->tunnelLength + ((float)(random()) / RAND_MAX) * 20;
    }
    ent->age += 0.1;
}

static void tick(blocktube_configuration *lp)
{
	lp->counter--;
	if (!lp->counter)
    {
		if (!lp->changing)
        {
			newTargetColor(lp);
			lp->counter = changetime;
        }
        else
        {
			lp->counter = holdtime;
        }
		lp->changing = (!lp->changing);
    }
    else
    {
		if (lp->changing)
        {
			lp->currentR += lp->deltaR;
			lp->currentG += lp->deltaG;
			lp->currentB += lp->deltaB;
        }
    }
}

static void cube_vertices(float x, float y, float z, int wire);

void reshape_blocktube(int width, int height)
{
    GLfloat h = (GLfloat) height / (GLfloat) width;

    glViewport(0, 0, (GLint) width, (GLint) height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1/h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void init_blocktube(struct BlankerData* bd)
{
	int loop;
	blocktube_configuration *lp;
	GLfloat fogColor[4] = {0,0,0,1};

	IUtility->SetMem(&lps, 0, sizeof(blocktube_configuration));

	lp = &lps;
    lp->zoom = 30;
    lp->tilt = 4.5;
    lp->tunnelLength = 200;
    lp->tunnelWidth = 5;

    lp->block_dlist = glGenLists (1);
    glNewList (lp->block_dlist, GL_COMPILE);
	cube_vertices(0.15, 1.2, 5.25, 0);
    glEndList ();

	do_fog = bd->dofog;
	do_texture = bd->dotexture;

	glDisable(GL_TEXTURE_2D);
	if (do_texture)
    {
		if (LoadGLTextures())
        {
			glEnable(GL_TEXTURE_2D);
        }
    }

    /* kick on the fog machine */
	if (do_fog)
    {
      glEnable(GL_FOG);
      glFogi(GL_FOG_MODE, GL_LINEAR);
      glHint(GL_FOG_HINT, GL_NICEST);
      glFogf(GL_FOG_START, 0);
	  glFogf(GL_FOG_END, lp->tunnelLength/1.8);
      glFogfv(GL_FOG_COLOR, fogColor);
      glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    }

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

	if (!do_texture) {
/*		 If there is no texture, the boxes don't show up without a light.
         Though I don't understand why all the blocks come out gray.
*/
      GLfloat pos[4] = {0.0, 1.0, 1.0, 0.0};
      GLfloat amb[4] = {0.2, 0.2, 0.2, 1.0};
      GLfloat dif[4] = {1.0, 1.0, 1.0, 1.0};
      GLfloat spc[4] = {1.0, 1.0, 1.0, 1.0};
      glLightfv(GL_LIGHT0, GL_POSITION, pos);
      glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
      glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);
      glLightfv(GL_LIGHT0, GL_SPECULAR, spc);
      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);
    }

	holdtime = bd->holdtime;
	changetime = bd->changetime;
	do_texture = bd->dotexture;
	do_fog = bd->dofog;

	lp->counter = holdtime;
	lp->currentR = random() % 256;
	lp->currentG = random() % 256;
	lp->currentB = random() % 256;
	newTargetColor(lp);
    for (loop = 0; loop < MAX_ENTITIES; loop++)
    {
		randomize_entity(lp, &lp->entities[loop]);
    }
    glFlush();
}

void release_blocktube(void)
{
	blocktube_configuration *lp = &lps;
	if (do_texture)
	{
		glDeleteTextures(1, &lp->envTexture);
		XDestroyImage(lp->texti);
	}
}

static void cube_vertices(float x, float y, float z, int wire)
{
    float x2, y2, z2, nv = 0.7;
    x2 = x/2;
    y2 = y/2;
    z2 = z/2;

    glFrontFace(GL_CW);

    glNormal3f(0, 0, nv);
    glBegin (wire ? GL_LINE_LOOP : GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(-x2,  y2,  z2);
    glTexCoord2f(1.0, 0.0); glVertex3f( x2,  y2,  z2);
    glTexCoord2f(1.0, 1.0); glVertex3f( x2, -y2,  z2);
    glTexCoord2f(0.0, 1.0); glVertex3f(-x2, -y2,  z2);
    glEnd();

    glNormal3f(0, 0, -nv);
    glBegin (wire ? GL_LINE_LOOP : GL_QUADS);
    glTexCoord2f(1.0, 0.0); glVertex3f(-x2, -y2, -z2);
    glTexCoord2f(1.0, 1.0); glVertex3f( x2, -y2, -z2);
    glTexCoord2f(0.0, 1.0); glVertex3f( x2,  y2, -z2);
    glTexCoord2f(0.0, 0.0); glVertex3f(-x2,  y2, -z2);
    glEnd();

    glNormal3f(0, nv, 0);
    glBegin (wire ? GL_LINE_LOOP : GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f(-x2,  y2, -z2);
    glTexCoord2f(0.0, 0.0); glVertex3f( x2,  y2, -z2);
    glTexCoord2f(1.0, 0.0); glVertex3f( x2,  y2,  z2);
    glTexCoord2f(1.0, 1.0); glVertex3f(-x2,  y2,  z2);
    glEnd();

    glNormal3f(0, -nv, 0);
    glBegin (wire ? GL_LINE_LOOP : GL_QUADS);
    glTexCoord2f(1.0, 1.0); glVertex3f(-x2, -y2, -z2);
    glTexCoord2f(0.0, 1.0); glVertex3f(-x2, -y2,  z2);
    glTexCoord2f(0.0, 0.0); glVertex3f( x2, -y2,  z2);
    glTexCoord2f(1.0, 0.0); glVertex3f( x2, -y2, -z2);
    glEnd();

    if (wire) return;

    glNormal3f(nv, 0, 0);
    glBegin (wire ? GL_LINE_LOOP : GL_QUADS);
    glTexCoord2f(1.0, 0.0); glVertex3f( x2, -y2, -z2);
    glTexCoord2f(1.0, 1.0); glVertex3f( x2, -y2,  z2);
    glTexCoord2f(0.0, 1.0); glVertex3f( x2,  y2,  z2);
    glTexCoord2f(0.0, 0.0); glVertex3f( x2,  y2, -z2);
    glEnd();

    glNormal3f(-nv, 0, 0);
    glBegin (wire ? GL_LINE_LOOP : GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(-x2, -y2, -z2);
    glTexCoord2f(1.0, 0.0); glVertex3f(-x2,  y2, -z2);
    glTexCoord2f(1.0, 1.0); glVertex3f(-x2,  y2,  z2);
    glTexCoord2f(0.0, 1.0); glVertex3f(-x2, -y2,  z2);
    glEnd();
}

static void draw_block()
{
	blocktube_configuration *lp = &lps;
    glCallList (lp->block_dlist);
}

void draw_blocktube()
{
	blocktube_configuration *lp = &lps;
	entity *cEnt = NULL;
    int loop = 0;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (do_texture)
	{
      glEnable(GL_TEXTURE_GEN_S);
      glEnable(GL_TEXTURE_GEN_T);
	  glBindTexture(GL_TEXTURE_2D, lp->envTexture);
    }

	for (loop = 0; loop < MAX_ENTITIES; loop++)
	{
		cEnt = &lp->entities[loop];

        glLoadIdentity();
		glTranslatef(0.0f, 0.0f, lp->zoom);
		glRotatef(lp->tilt, 1.0f, 0.0f, 0.0f);
        glRotatef(cEnt->angle, 0.0f, 0.0f, 1.0f);
        glTranslatef(cEnt->position[0], cEnt->position[1], cEnt->position[2]);
		glColor4ub((int)(lp->currentR * cEnt->tVal),
				   (int)(lp->currentG * cEnt->tVal),
				   (int)(lp->currentB * cEnt->tVal), 255);
        draw_block();
		entityTick(lp, cEnt);
    }
	tick(lp);

    glFinish();
}

