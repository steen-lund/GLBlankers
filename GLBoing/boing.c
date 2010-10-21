/* boing, Copyright (c) 2005 Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * A clone of the Amiga 1000 "Boing" demo.  This was the first graphics demo
 * for the Amiga, written by Dale Luck and RJ Mical during a break at the 1984
 * Consumer Electronics Show (or so the legend goes.)  The boing ball was
 * briefly the official logo of Amiga Inc., until they were bought by
 * Commodore later that year.
 *
 * With no arguments, this program looks a lot like the original Amiga demo.
 * With "-smooth -lighting", it looks... less old.
 *
 * The amiga version made noise when the ball hit the walls.  This version
 * does not, obviously.
 */

#undef countof
#define countof(x) (sizeof((x))/sizeof((*x)))

#include <GL/gl.h>
#include "gltrackball.h"
#include "utils.h"
#include "yarandom.h"
#include "blanker.h"

#include <ctype.h>

#ifdef USE_GL /* whole file */

#define DEF_SPIN        "True"
#define DEF_LIGHTING    "False"
#define DEF_SMOOTH      "False"
#define DEF_SCANLINES   "True"
#define DEF_SPEED       "1.0"
#define DEF_SIZE        "0.5"
#define DEF_ANGLE       "15"
#define DEF_MERIDIANS   "16"
#define DEF_PARALLELS   "8"
#define DEF_TILES       "12"
#define DEF_THICKNESS   "0.05"

#define DEF_BALL_COLOR1  "#CC1919"
#define DEF_BALL_COLOR2  "#F2F2F2"
#define DEF_GRID_COLOR   "#991999"
#define DEF_SHADOW_COLOR "#303030"
#define DEF_BACKGROUND   "#8C8C8C"

typedef struct { GLfloat x, y, z; } XYZ;

typedef struct {
  trackball_state *trackball;
  Bool button_down_p;

  GLuint ball_list;
  double ball_x,   ball_y,   ball_z,   ball_th;
  double ball_dx,  ball_dy,  ball_dz,  ball_dth;
  double ball_ddx, ball_ddy, ball_ddz;

  GLfloat ball_color1[4], ball_color2[4], grid_color[4];
  GLfloat bg_color[4], shadow_color[4];
  GLfloat lightpos[4];

} boing_configuration;

static boing_configuration bps;

static Bool spin = TRUE;
static Bool lighting_p = FALSE;
static Bool smooth_p = FALSE;
static Bool scanlines_p = TRUE;
static GLfloat speed = 1.0f;
static int angle = 15;
static GLfloat ball_size = 0.5f;
static unsigned int meridians = 16;
static unsigned int parallels = 8;
static unsigned int tiles = 12;
static GLfloat thickness = 0.05f;
static char *ball_color1_str = DEF_BALL_COLOR1, *ball_color2_str = DEF_BALL_COLOR2, *grid_color_str = DEF_GRID_COLOR,
  *shadow_str = DEF_SHADOW_COLOR, *bg_str = DEF_BACKGROUND;

/*static XrmOptionDescRec opts[] = {
  { "-spin",       ".spin",      XrmoptionNoArg,  "True"  },
  { "+spin",       ".spin",      XrmoptionNoArg,  "False" },
  { "-lighting",   ".lighting",  XrmoptionNoArg,  "True"  },
  { "+lighting",   ".lighting",  XrmoptionNoArg,  "False" },
  { "-smooth",     ".smooth",    XrmoptionNoArg,  "True"  },
  { "+smooth",     ".smooth",    XrmoptionNoArg,  "False" },
  { "-scanlines",  ".scanlines", XrmoptionNoArg,  "True"  },
  { "+scanlines",  ".scanlines", XrmoptionNoArg,  "False" },
  { "-speed",      ".speed",     XrmoptionSepArg, 0 },
  { "-angle",      ".angle",     XrmoptionSepArg, 0 },
  { "-size",       ".ballSize",  XrmoptionSepArg, 0 },
  { "-meridians",  ".meridians", XrmoptionSepArg, 0 },
  { "-parallels",  ".parallels", XrmoptionSepArg, 0 },
  { "-tiles",      ".tiles",     XrmoptionSepArg, 0 },
  { "-thickness",  ".thickness", XrmoptionSepArg, 0 },
  { "-ball-color1",".ballColor1",XrmoptionSepArg, 0 },
  { "-ball-color2",".ballColor2",XrmoptionSepArg, 0 },
  { "-grid-color", ".gridColor", XrmoptionSepArg, 0 },
  { "-shadow-color",".shadowColor",XrmoptionSepArg, 0 },
};*/

/*static argtype vars[] = {
  {&spin,      "spin",      "Spin",       DEF_SPIN,      t_Bool},
  {&lighting_p,"lighting",  "Lighting",   DEF_LIGHTING,  t_Bool},
  {&smooth_p,  "smooth",    "Smooth",     DEF_SMOOTH,    t_Bool},
  {&scanlines_p,"scanlines","Scanlines",  DEF_SCANLINES, t_Bool},
  {&speed,     "speed",     "Speed",      DEF_SPEED,     t_Float},
  {&angle,     "angle",     "Angle",      DEF_ANGLE,     t_Int},
  {&ball_size, "ballSize",  "BallSize",   DEF_SIZE,      t_Float},
  {&meridians, "meridians", "meridians",  DEF_MERIDIANS, t_Int},
  {&parallels, "parallels", "parallels",  DEF_PARALLELS, t_Int},
  {&tiles,     "tiles",     "Tiles",      DEF_TILES,     t_Int},
  {&thickness, "thickness", "Thickness",  DEF_THICKNESS, t_Float},
  {&ball_color1_str, "ballColor1", "BallColor1", DEF_BALL_COLOR1, t_String},
  {&ball_color2_str, "ballColor2", "BallColor2", DEF_BALL_COLOR2, t_String},
  {&grid_color_str,  "gridColor",  "GridColor",  DEF_GRID_COLOR,  t_String},
  {&shadow_str,      "shadowColor","ShadowColor",DEF_SHADOW_COLOR,t_String},
*/	/* dammit, -background is too magic... */
/*	{&bg_str,        "boingBackground", "Background", DEF_BACKGROUND, t_String},
};*/

/*ENTRYPOINT ModeSpecOpt boing_opts = {countof(opts), opts, countof(vars), vars, NULL};*/

static void
parse_color (const char *name, const char *s, GLfloat *a)
{
#pragma message Remove this when including a color selector
  a[3] = 1.0;  /* alpha */
  int temps[3];
  int c;
  for (c = 0; c < 3; ++c)
  {
	temps[c] = 0;
	char t = s[1 + c * 2];
	if (t >= '0' && t <= '9')
	{
		temps[c] = (t - '0') * 16;
	}
	else if (t >= 'A' && t <= 'F')
	{
		temps[c] = (t - 'A' + 10) * 16;
	}
	t = s[2 + c * 2];
	if (t >= '0' && t <= '9')
	{
		temps[c] += (t - '0');
	}
	else if (t >= 'A' && t <= 'F')
	{
		temps[c] += t - 'A' + 10;
	}
  }

  a[0] = temps[0] / 255.0;
  a[1] = temps[1] / 255.0;
  a[2] = temps[2] / 255.0;
}


static void
draw_grid ()
{
  boing_configuration *bp = &bps;
  int x, y;
  GLfloat t2  = (GLfloat) tiles / 2;
  GLfloat s = 1.0 / (tiles + thickness);
  GLfloat z = 0;
#pragma fix height
  GLfloat lw = 480 * 0.06 * thickness;

  glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bp->grid_color);
  glColor3fv (bp->grid_color);

  glPushMatrix();
  glScalef(s, s, s);
  glTranslatef (-t2, -t2, 0);

  glLineWidth (lw);
  glBegin (GL_LINES);
  for (y = 0; y <= tiles; y++)
    {
      glVertex3f (0,     y, z);
      glVertex3f (tiles, y, z);
      /*mi->polygon_count++;*/
    }
  for (x = 0; x <= tiles; x++)
    {
      glVertex3f (x, tiles, z);
      glVertex3f (x, 0,     z);
      /*mi->polygon_count++;*/
    }

  glEnd();
  glPopMatrix();
}


static void
draw_box ()
{
  /* boing_configuration *bp = &bps[MI_SCREEN(mi)]; */
  glPushMatrix();
  glTranslatef (0, 0, -0.5);
/*  glFrontFace (GL_CCW);*/
  draw_grid ();
  glPopMatrix();

  glPushMatrix();
  glRotatef (90, 1, 0, 0);
  glTranslatef (0, 0, 0.5);
/*  glFrontFace (GL_CW);*/
  draw_grid ();
  glPopMatrix();
}


static void
draw_ball ()
{
  boing_configuration *bp = &bps;
  int wire = 0; /*MI_IS_WIREFRAME(mi);*/
  int x, y;
  int xx = meridians;
  int yy = parallels;
  int scale = (smooth_p ? 5 : 1);

  if (lighting_p && !wire)
    glEnable (GL_LIGHTING);

  if (parallels < 3)
    scale *= 2;

  xx *= scale;
  yy *= scale;

  glFrontFace (GL_CW);

  glPushMatrix();
  glTranslatef (bp->ball_x, bp->ball_y, bp->ball_z);
  glScalef (ball_size, ball_size, ball_size);
  glRotatef (-angle,      0, 0, 1);
  glRotatef (bp->ball_th, 0, 1, 0);

  for (y = 0; y < yy; y++)
    {
      GLfloat thy0 = y     * (M_PI * 2) / (yy * 2) + M_PI_2;
      GLfloat thy1 = (y+1) * (M_PI * 2) / (yy * 2) + M_PI_2;

      for (x = 0; x < xx; x++)
        {
          GLfloat thx0 = x     * (M_PI * 2) / xx;
          GLfloat thx1 = (x+1) * (M_PI * 2) / xx;
          XYZ p;
          Bool bgp = ((x/scale) & 1) ^ ((y/scale) & 1);

          if (wire && bgp) continue;

          glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE,
                        (bgp ? bp->ball_color2 : bp->ball_color1));
          glColor3fv (bgp ? bp->ball_color2 : bp->ball_color1);

          glBegin (wire ? GL_LINE_LOOP : GL_QUADS);

          if (!smooth_p)
            {
              p.x = cos((thy0+thy1)/2) * cos((thx0+thx1)/2);
              p.y = sin((thy0+thy1)/2);
              p.z = cos((thy0+thy1)/2) * sin((thx0+thx1)/2);
              glNormal3f (-p.x, -p.y, -p.z);
            }

          p.x = cos(thy0) * cos(thx0) / 2;
          p.y = sin(thy0)             / 2;
          p.z = cos(thy0) * sin(thx0) / 2;
          if (smooth_p)
            glNormal3f (-p.x, -p.y, -p.z);
          glVertex3f (p.x, p.y, p.z);

          p.x = cos(thy1) * cos(thx0) / 2;
          p.y = sin(thy1)             / 2;
          p.z = cos(thy1) * sin(thx0) / 2;
          if (smooth_p)
            glNormal3f (-p.x, -p.y, -p.z);
          glVertex3f (p.x, p.y, p.z);

          p.x = cos(thy1) * cos(thx1) / 2;
          p.y = sin(thy1)             / 2;
          p.z = cos(thy1) * sin(thx1) / 2;
          if (smooth_p)
            glNormal3f (-p.x, -p.y, -p.z);
          glVertex3f (p.x, p.y, p.z);

          p.x = cos(thy0) * cos(thx1) / 2;
          p.y = sin(thy0)             / 2;
          p.z = cos(thy0) * sin(thx1) / 2;
          if (smooth_p)
            glNormal3f (-p.x, -p.y, -p.z);
          glVertex3f (p.x, p.y, p.z);

          glEnd ();
        }
    }

  glPopMatrix();

  if (lighting_p && !wire)
    glDisable(GL_LIGHTING);
}


static void
draw_shadow ()
{
  boing_configuration *bp = &bps;
  int wire = 0; /*MI_IS_WIREFRAME(mi);*/
  GLfloat xoff = 0.14;
  GLfloat yoff = 0.07;
  int y;
  int yy = parallels;
  int scale = (smooth_p ? 5 : 1);

  if (lighting_p && !wire)
    glEnable (GL_BLEND);

  if (parallels < 3)
    scale *= 2;

  yy *= scale;

  glPushMatrix();
  glTranslatef (bp->ball_x + xoff, bp->ball_y + yoff, -0.49);
  glScalef (ball_size, ball_size, ball_size);
  glRotatef (-angle, 0, 0, 1);

  glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bp->shadow_color);
  glColor4fv (bp->shadow_color);

  glFrontFace (GL_CCW);
  glNormal3f (0, 0, 1);
  glBegin (wire ? GL_LINE_LOOP : GL_TRIANGLE_FAN);
  if (!wire) glVertex3f (0, 0, 0);

  for (y = 0; y < yy*2+1; y++)
    {
      GLfloat thy0 = y * (M_PI * 2) / (yy * 2) + M_PI_2;
      glVertex3f (cos(thy0) / 2, sin(thy0) / 2, 0);
    }

  glEnd ();

  glPopMatrix();

  if (lighting_p && !wire)
    glDisable (GL_BLEND);
}


static void
draw_scanlines ()
{
  /* boing_configuration *bp = &bps[MI_SCREEN(mi)]; */
  int wire = 0; /*MI_IS_WIREFRAME(mi);*/
#pragma message fix these
  int w = 640;
  int h = 480;

  if (h <= 300) return;

  if (!wire)
    {
      glEnable (GL_BLEND);
      glDisable (GL_DEPTH_TEST);
    }

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  {
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    {
      int lh, ls;
      int y;
      glLoadIdentity();
      gluOrtho2D (0, w, 0, h);

      if      (h > 500) lh = 4, ls = 4;
      else if (h > 300) lh = 2, ls = 1;
      else              lh = 1, ls = 1;

      if (lh == 1)
        glDisable (GL_BLEND);

      glLineWidth (lh);
      glColor4f (0, 0, 0, 0.3);

      glBegin(GL_LINES);
      for (y = 0; y < h; y += lh + ls)
        {
          glVertex3f (0, y, 0);
          glVertex3f (w, y, 0);
        }
      glEnd();
    }
    glPopMatrix();
  }
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  if (!wire)
    {
      glDisable (GL_BLEND);
      glEnable (GL_DEPTH_TEST);
    }
}



static void
tick_physics ()
{
  boing_configuration *bp = &bps;
  GLfloat s2 = ball_size / 2;
  GLfloat max = 0.5 - s2;
  GLfloat min = -max;

  bp->ball_th += bp->ball_dth;
  while (bp->ball_th > 360) bp->ball_th -= 360;
  while (bp->ball_th < 0)   bp->ball_th += 360;

  bp->ball_dx += bp->ball_ddx;
  bp->ball_x  += bp->ball_dx;
  if      (bp->ball_x < min) bp->ball_x = min, bp->ball_dx = -bp->ball_dx,
    bp->ball_dth = -bp->ball_dth,
    bp->ball_dx += (frand(speed/2) - speed);
  else if (bp->ball_x > max) bp->ball_x = max, bp->ball_dx = -bp->ball_dx,
    bp->ball_dth = -bp->ball_dth,
    bp->ball_dx += (frand(speed/2) - speed);

  bp->ball_dy += bp->ball_ddy;
  bp->ball_y  += bp->ball_dy;
  if      (bp->ball_y < min) bp->ball_y = min, bp->ball_dy = -bp->ball_dy;
  else if (bp->ball_y > max) bp->ball_y = max, bp->ball_dy = -bp->ball_dy;

  bp->ball_dz += bp->ball_ddz;
  bp->ball_z  += bp->ball_dz;
  if      (bp->ball_z < min) bp->ball_z = min, bp->ball_dz = -bp->ball_dz;
  else if (bp->ball_z > max) bp->ball_z = max, bp->ball_dz = -bp->ball_dz;
}



/* Window management, etc
 */
void
reshape_boing (int width, int height)
{
  GLfloat h = (GLfloat) height / (GLfloat) width;

  h *= 4.0 / 3.0;   /* Back in the caveman days we couldn't even afford
                       square pixels! */

  glViewport (0, 0, (GLint) width, (GLint) height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective (8.0, 1/h, 1.0, 10.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt (0.0, 0.0, 8.0,
             0.0, 0.0, 0.0,
             0.0, 1.0, 0.0);

  glClear(GL_COLOR_BUFFER_BIT);
}


void
init_boing (struct BlankerData* bd)
{
  boing_configuration *bp;
  int wire = 0;

  bp = &bps;

  if (tiles < 1) tiles = 1;

  if (smooth_p)
    {
      if (meridians < 1) meridians = 1;
      if (parallels < 1) parallels = 1;
    }
  else
    {
      if (meridians < 3) meridians = 3;
      if (parallels < 2) parallels = 2;
    }

  if (meridians > 1 && meridians & 1) meridians++;  /* odd numbers look bad */


  if (thickness <= 0) thickness = 0.001;
  else if (thickness > 1) thickness = 1;

  parse_color ("ballColor1",  ball_color1_str,  bp->ball_color1);
  parse_color ("ballColor2",  ball_color2_str,  bp->ball_color2);
  parse_color ("gridColor",   grid_color_str,   bp->grid_color);
  parse_color ("shadowColor", shadow_str,       bp->shadow_color);
  parse_color ("background",  bg_str,           bp->bg_color);

  bp->shadow_color[3] = 0.9;

  glClearColor (bp->bg_color[0], bp->bg_color[1], bp->bg_color[2], 1);

  if (!wire)
    {
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);
    }

  bp->lightpos[0] = 0.5;
  bp->lightpos[1] = 0.5;
  bp->lightpos[2] = -1;
  bp->lightpos[3] = 0;

  if (lighting_p && !wire)
    {
      GLfloat amb[4] = {0, 0, 0, 1};
      GLfloat dif[4] = {1, 1, 1, 1};
      GLfloat spc[4] = {1, 1, 1, 1};
      glEnable(GL_LIGHT0);
      glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
      glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);
      glLightfv(GL_LIGHT0, GL_SPECULAR, spc);
    }

  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  speed = speed / 800.0;

  bp->ball_dth = (spin ? -speed * 7 * 360 : 0);

  bp->ball_x   = 0.5 - ((ball_size/2) + frand(1-ball_size));
  bp->ball_y   = 0.2;
  bp->ball_dx  = speed * 6 + frand(speed);
  bp->ball_ddy = -speed;

  bp->ball_dz  = speed * 6 + frand(speed);

  bp->trackball = gltrackball_init ();
}

void
draw_boing ()
{
  boing_configuration *bp = &bps;

  glShadeModel(GL_SMOOTH);

  glEnable(GL_NORMALIZE);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (! bp->button_down_p)
	tick_physics ();

  glPushMatrix ();
  gltrackball_rotate (bp->trackball);

  glLightfv (GL_LIGHT0, GL_POSITION, bp->lightpos);

  glDisable (GL_CULL_FACE);
  glDisable (GL_DEPTH_TEST);

  glEnable (GL_LINE_SMOOTH);
  glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable (GL_BLEND);

  draw_box ();
  draw_shadow ();

  glEnable (GL_CULL_FACE);
  glEnable (GL_DEPTH_TEST);

  draw_ball ();
  if (scanlines_p)
	draw_scanlines ();

  glPopMatrix ();

  glFinish();
}

#endif /* USE_GL */
