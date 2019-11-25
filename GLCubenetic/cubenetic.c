/* cubenetic, Copyright (c) 2002-2006 Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#include <GL/gl.h>
#include "utils.h"
#include "colors.h"
#include "rotator.h"
#include "gltrackball.h"
#include "yarandom.h"
#include <ctype.h>
#include "blanker.h"

#include <exec/exectags.h>
#include <proto/exec.h>
#include <utility/tagitem.h>

#ifdef USE_GL /* whole file */

#define DEF_SPIN        "XYZ"
#define DEF_WANDER      "True"
#define DEF_TEXTURE     "True"

#define DEF_WAVE_COUNT  "3"
#define DEF_WAVE_SPEED  "80"
#define DEF_WAVE_RADIUS "512"

extern char* spins[];

typedef struct {
  int color;
  GLfloat x, y, z;
  GLfloat w, h, d;
  int frame;
  GLfloat dx, dy, dz;
  GLfloat dw, dh, dd;
} cube;

typedef struct {
  int x, y;
  double xth, yth;
} wave_src;

typedef struct {
  int nwaves;
  int radius;
  int sqr_radius;
  int speed;
  wave_src *srcs;
  int *heights;
} waves;


typedef struct {
  rotator *rot;
  trackball_state *trackball;
  Bool button_down_p;

  GLuint cube_list;
  GLuint texture_id;
  int ncubes;
  cube *cubes;
  waves *waves;

  int texture_width, texture_height;
  unsigned char *texture;

  int ncolors;
  XColor *cube_colors;
  XColor *texture_colors;

} cube_configuration;

static cube_configuration ccs;

static BOOL do_wander = TRUE;
static BOOL do_texture = TRUE;

static int wave_count = 4;
static int wave_speed = 40;
static int wave_radius = 64;
static int texture_size = 128;

static void
unit_cube (Bool wire)
{
  glBegin (wire ? GL_LINE_LOOP : GL_QUADS);	/* front */
  glNormal3f (0, 0, 1);
  glTexCoord2f(1, 0); glVertex3f ( 0.5, -0.5,  0.5);
  glTexCoord2f(0, 0); glVertex3f ( 0.5,  0.5,  0.5);
  glTexCoord2f(0, 1); glVertex3f (-0.5,  0.5,  0.5);
  glTexCoord2f(1, 1); glVertex3f (-0.5, -0.5,  0.5);
  glEnd();

  glBegin (wire ? GL_LINE_LOOP : GL_QUADS);	/* back */
  glNormal3f (0, 0, -1);
  glTexCoord2f(0, 0); glVertex3f (-0.5, -0.5, -0.5);
  glTexCoord2f(0, 1); glVertex3f (-0.5,  0.5, -0.5);
  glTexCoord2f(1, 1); glVertex3f ( 0.5,  0.5, -0.5);
  glTexCoord2f(1, 0); glVertex3f ( 0.5, -0.5, -0.5);
  glEnd();

  glBegin (wire ? GL_LINE_LOOP : GL_QUADS);	/* left */
  glNormal3f (-1, 0, 0);
  glTexCoord2f(1, 1); glVertex3f (-0.5,  0.5,  0.5);
  glTexCoord2f(1, 0); glVertex3f (-0.5,  0.5, -0.5);
  glTexCoord2f(0, 0); glVertex3f (-0.5, -0.5, -0.5);
  glTexCoord2f(0, 1); glVertex3f (-0.5, -0.5,  0.5);
  glEnd();

  glBegin (wire ? GL_LINE_LOOP : GL_QUADS);	/* right */
  glNormal3f (1, 0, 0);
  glTexCoord2f(1, 1); glVertex3f ( 0.5, -0.5, -0.5);
  glTexCoord2f(1, 0); glVertex3f ( 0.5,  0.5, -0.5);
  glTexCoord2f(0, 0); glVertex3f ( 0.5,  0.5,  0.5);
  glTexCoord2f(0, 1); glVertex3f ( 0.5, -0.5,  0.5);
  glEnd();

  if (wire) return;

  glBegin (wire ? GL_LINE_LOOP : GL_QUADS);	/* top */
  glNormal3f (0, 1, 0);
  glTexCoord2f(0, 0); glVertex3f ( 0.5,  0.5,  0.5);
  glTexCoord2f(0, 1); glVertex3f ( 0.5,  0.5, -0.5);
  glTexCoord2f(1, 1); glVertex3f (-0.5,  0.5, -0.5);
  glTexCoord2f(1, 0); glVertex3f (-0.5,  0.5,  0.5);
  glEnd();

  glBegin (wire ? GL_LINE_LOOP : GL_QUADS);	/* bottom */
  glNormal3f (0, -1, 0);
  glTexCoord2f(1, 0); glVertex3f (-0.5, -0.5,  0.5);
  glTexCoord2f(0, 0); glVertex3f (-0.5, -0.5, -0.5);
  glTexCoord2f(0, 1); glVertex3f ( 0.5, -0.5, -0.5);
  glTexCoord2f(1, 1); glVertex3f ( 0.5, -0.5,  0.5);
  glEnd();
}



/* Window management, etc
 */
void
reshape_cube (int width, int height)
{
  GLfloat h = (GLfloat) height / (GLfloat) width;

  glViewport (0, 0, (GLint) width, (GLint) height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective (30.0, 1/h, 1.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt( 0.0, 0.0, 30.0,
             0.0, 0.0, 0.0,
             0.0, 1.0, 0.0);

  glClear(GL_COLOR_BUFFER_BIT);
}


/* Waves.
   Adapted from ../hacks/interference.c by Hannu Mallat.
 */

static void
init_wave ()
{
  cube_configuration *cc = &ccs;
  waves *ww;
  int i;
  cc->waves = ww = (waves *) IExec->AllocVecTags(sizeof(*cc->waves), AVT_ClearWithValue, 0, TAG_DONE);
  ww->nwaves = wave_count;
  ww->radius = wave_radius;
  ww->sqr_radius = wave_radius * wave_radius;
  ww->speed  = wave_speed;
  ww->heights = (int *) IExec->AllocVecTags(sizeof(*ww->heights) * ww->radius, AVT_ClearWithValue, 0, TAG_DONE);
  ww->srcs = (wave_src *) IExec->AllocVecTags(sizeof(*ww->srcs) * ww->nwaves, AVT_ClearWithValue, 0, TAG_DONE);

  for (i = 0; i < ww->radius; i++)
    {
      float max = (cc->ncolors * (ww->radius - i) / (float) ww->radius);
      ww->heights[i] = ((max + max * cos(i / 50.0)) / 2.0);
    }

  for (i = 0; i < ww->nwaves; i++)
    {
      ww->srcs[i].xth = frand(2.0) * M_PI;
      ww->srcs[i].yth = frand(2.0) * M_PI;
    }
}

static void
free_wave ()
{
	cube_configuration *cc = &ccs;
	waves *ww = cc->waves;
	IExec->FreeVec(ww->heights);
	IExec->FreeVec(ww->srcs);
	IExec->FreeVec(cc->waves);
	ww->heights = NULL;
	ww->srcs = NULL;
	cc->waves = NULL;
}

static void
interference ()
{
  cube_configuration *cc = &ccs;
  waves *ww = cc->waves;
  int x, y, i;

  /* Move the wave origins around
   */
  for (i = 0; i < ww->nwaves; i++)
    {
      ww->srcs[i].xth += (ww->speed / 1000.0);
      if (ww->srcs[i].xth > 2*M_PI)
        ww->srcs[i].xth -= 2*M_PI;
      ww->srcs[i].yth += (ww->speed / 1000.0);
      if (ww->srcs[i].yth > 2*M_PI)
        ww->srcs[i].yth -= 2*M_PI;

      ww->srcs[i].x = (cc->texture_width/2 +
                       (cos (ww->srcs[i].xth) *
                        cc->texture_width / 2));
      ww->srcs[i].y = (cc->texture_height/2 +
                       (cos (ww->srcs[i].yth) *
                        cc->texture_height / 2));
    }

  /* Compute the effect of the waves on each pixel,
     and generate the output map.
   */
  for (y = 0; y < cc->texture_height; y++)
    for (x = 0; x < cc->texture_width; x++)
      {
        int result = 0;
        unsigned char *o;
        for (i = 0; i < ww->nwaves; i++)
          {
            int dx = x - ww->srcs[i].x;
            int dy = y - ww->srcs[i].y;
			int dist = dx*dx + dy*dy;
			if (dist < ww->sqr_radius)
			{
				result += ww->heights[(int)sqrt(dist)];
			}
          }
        result %= cc->ncolors;

        o = cc->texture + (((y * cc->texture_width) + x) << 2);
        o[0] = (cc->texture_colors[result].red   >> 8);
        o[1] = (cc->texture_colors[result].green >> 8);
        o[2] = (cc->texture_colors[result].blue  >> 8);
	 /* o[3] = 0xFF; */
	  }
}


/* Textures
 */

static void
init_texture ()
{
  cube_configuration *cc = &ccs;
  int i;

  glEnable(GL_TEXTURE_2D);

  /*clear_gl_error();*/
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glGenTextures (1, &cc->texture_id);
  glBindTexture (GL_TEXTURE_2D, cc->texture_id);
  /*check_gl_error("texture binding");*/

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  /*check_gl_error("texture initialization");*/

  cc->texture_width  = texture_size;
  cc->texture_height = texture_size;

  i = texture_size * texture_size * 4;
  cc->texture = (unsigned char *) IExec->AllocVecTags (i, AVT_Clear, 0xFF, TAG_DONE);
}

static void
free_texture ()
{
  cube_configuration *cc = &ccs;
  IExec->FreeVec(cc->texture);
  cc->texture = NULL;
  glDeleteTextures(1, &cc->texture_id);
  cc->texture_id = 0;
  glDisable(GL_TEXTURE_2D);
}


static void
shuffle_texture ()
{
  cube_configuration *cc = &ccs;
  interference ();
  /*clear_gl_error();*/
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA,
                cc->texture_width, cc->texture_height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE,
                cc->texture);
  /*check_gl_error("texture");*/
}

void
init_cube (struct BlankerData* bd)
{
  int i;
  cube_configuration *cc;
  int wire = False;

  cc = &ccs;

  wave_count = bd->waves;
  wave_radius = bd->radius;
  wave_speed = bd->speed;
  do_texture = bd->texture;
  texture_size = 16 * (1<<bd->texture_size);
  do_wander = bd->wander;

  if (!wire)
    {
      static const GLfloat pos[4] = {1.0, 0.5, 1.0, 0.0};
      static const GLfloat amb[4] = {0.2, 0.2, 0.2, 1.0};
      static const GLfloat dif[4] = {1.0, 1.0, 1.0, 1.0};

      glLightfv(GL_LIGHT0, GL_POSITION, pos);
      glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
      glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);

      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);
    }


  {
    Bool spinx=False, spiny=False, spinz=False;
    double spin_speed   = 1.0;
    double wander_speed = 0.05;

	if (bd->spin > 0)
	{
		char *s = spins[bd->spin];
		while (*s)
		{
			if      (*s == 'x' || *s == 'X') spinx = True;
			else if (*s == 'y' || *s == 'Y') spiny = True;
			else if (*s == 'z' || *s == 'Z') spinz = True;
			else if (*s == '0') ;
			else
			{
				exit (1);
			  }
			s++;
		  }
	}
    cc->rot = make_rotator (spinx ? spin_speed : 0,
                            spiny ? spin_speed : 0,
                            spinz ? spin_speed : 0,
                            1.0,
                            do_wander ? wander_speed : 0,
                            (spinx && spiny && spinz));
    cc->trackball = gltrackball_init ();
  }

  cc->ncolors = 256;
  cc->texture_colors = (XColor *) IExec->AllocVecTags(cc->ncolors * sizeof(XColor), AVT_ClearWithValue, 0, TAG_DONE);
  cc->cube_colors    = (XColor *) IExec->AllocVecTags(cc->ncolors * sizeof(XColor), AVT_ClearWithValue, 0, TAG_DONE);

  {
    double H[3], S[3], V[3];
    int shift = 60;
    H[0] = frand(360.0); 
    H[1] = ((H[0] + shift) < 360) ? (H[0]+shift) : (H[0] + shift - 360);
    H[2] = ((H[1] + shift) < 360) ? (H[1]+shift) : (H[1] + shift - 360);
    S[0] = S[1] = S[2] = 1.0;
    V[0] = V[1] = V[2] = 1.0;
    make_color_loop(0, 0,
		    H[0], S[0], V[0], 
		    H[1], S[1], V[1], 
		    H[2], S[2], V[2], 
		    cc->texture_colors, &cc->ncolors,
		    False, False);

    make_smooth_colormap (0, 0, 0,
                          cc->cube_colors, &cc->ncolors, 
                          False, 0, False);
  }

  cc->ncubes = 1; /*MI_COUNT (mi);*/
  cc->cubes = (cube *) IExec->AllocVecTags(sizeof(cube) * cc->ncubes, AVT_ClearWithValue, 0, TAG_DONE);
  for (i = 0; i < cc->ncubes; i++)
    {
      cube *cube = &cc->cubes[i];
      cube->color = random() % cc->ncolors;
      cube->w = 1.0;
      cube->h = 1.0;
      cube->d = 1.0;
      cube->dx = frand(0.1);
      cube->dy = frand(0.1);
      cube->dz = frand(0.1);
      cube->dw = frand(0.1);
      cube->dh = frand(0.1);
      cube->dd = frand(0.1);
    }

  if (wire)
    do_texture = False;
    
  if (do_texture)
    {
	  init_texture ();
	  init_wave ();
	  shuffle_texture ();
    }

  cc->cube_list = glGenLists (1);
  glNewList (cc->cube_list, GL_COMPILE);
  unit_cube (wire);
  glEndList ();
}

void release_cube ()
{
	cube_configuration *cc = &ccs;
	if (do_texture)
	{
		free_wave();
		free_texture();
	}
	IExec->FreeVec(cc->cubes);
	IExec->FreeVec(cc->cube_colors);
	IExec->FreeVec(cc->texture_colors);
	cc->cubes = NULL;
	cc->cube_colors = NULL;
	cc->texture_colors = NULL;
}

static void
shuffle_cubes ()
{
  cube_configuration *cc = &ccs;
  int i;
  for (i = 0; i < cc->ncubes; i++)
    {
#     define SINOID(SCALE,FRAME,SIZE) \
        ((((1 + sin((FRAME * (SCALE)) / 2 * M_PI)) / 2.0) * (SIZE)) - (SIZE)/2)

      cube *cube = &cc->cubes[i];
      cube->x = SINOID(cube->dx, cube->frame, 0.5);
      cube->y = SINOID(cube->dy, cube->frame, 0.5);
      cube->z = SINOID(cube->dz, cube->frame, 0.5);
      cube->w = SINOID(cube->dw, cube->frame, 0.9) + 1.0;
      cube->h = SINOID(cube->dh, cube->frame, 0.9) + 1.0;
	  cube->d = SINOID(cube->dd, cube->frame, 0.9) + 1.0;
      cube->frame++;
#     undef SINOID
    }
}


void
draw_cube ()
{
  cube_configuration *cc = &ccs;
  int i;

  glShadeModel(GL_FLAT);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glEnable(GL_CULL_FACE);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix ();

  glScalef(1.1, 1.1, 1.1);

  {
    double x, y, z;
    get_position (cc->rot, &x, &y, &z, !cc->button_down_p);
    glTranslatef((x - 0.5) * 8,
                 (y - 0.5) * 6,
                 (z - 0.5) * 15);

    gltrackball_rotate (cc->trackball);

    get_rotation (cc->rot, &x, &y, &z, !cc->button_down_p);
    glRotatef (x * 360, 1.0, 0.0, 0.0);
    glRotatef (y * 360, 0.0, 1.0, 0.0);
    glRotatef (z * 360, 0.0, 0.0, 1.0);
  }

  glScalef (2.5, 2.5, 2.5);

  for (i = 0; i < cc->ncubes; i++)
    {
      cube *cube = &cc->cubes[i];
      GLfloat color[4];
      color[0] = cc->cube_colors[cube->color].red   / 65536.0;
      color[1] = cc->cube_colors[cube->color].green / 65536.0;
      color[2] = cc->cube_colors[cube->color].blue  / 65536.0;
      color[3] = 1.0;
      cube->color++;
      if (cube->color >= cc->ncolors) cube->color = 0;

      glPushMatrix ();
      glTranslatef (cube->x, cube->y, cube->z);
      glScalef (cube->w, cube->h, cube->d);
      glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	  glCallList (cc->cube_list);
      glPopMatrix ();
    }

  shuffle_cubes ();
  if (do_texture)
	shuffle_texture ();

  glPopMatrix();

  glFinish();
}

#endif /* USE_GL */
