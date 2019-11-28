/* glmatrix, Copyright (c) 2003 Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * GLMatrix -- simulate the text scrolls from the movie "The Matrix".
 *
 * This program does a 3D rendering of the dropping characters that
 * appeared in the title sequences of the movies.  See also `xmatrix'
 * for a simulation of what the computer monitors actually *in* the
 * movie did.
 */

#undef countof
#define countof(x) (sizeof((x))/sizeof((*x)))

#undef BELLRAND
#define BELLRAND(n) ((frand((n)) + frand((n)) + frand((n))) / 3.0)

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <proto/exec.h>

#include "glmatrix.h"
#include "yarandom.h"
#include "blanker.h"
#include "xi_image.h"

extern unsigned char matrix3_png[];

#ifndef M_PI
#define M_PI 3.1415627165242f
#endif

#ifdef USE_GL /* whole file */

#define CHAR_COLS 16
#define CHAR_ROWS 13

static const int matrix_encoding[] = {
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
# if 0
    192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207
# else
    160, 161, 162, 163, 164, 165, 166, 167,
    168, 169, 170, 171, 172, 173, 174, 175
# endif
  };
static const int decimal_encoding[]  = {
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };
static const int hex_encoding[] = {
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 33, 34, 35, 36, 37, 38 };
static const int binary_encoding[] = { 16, 17 };
static const int dna_encoding[]    = { 33, 35, 39, 52 };

static const unsigned char char_map[256] = {
   96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96,  /*   0 */
   96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96,  /*  16 */
    0,  1,  2, 96,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,  /*  32 */
   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,  /*  48 */
   32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,  /*  64 */
   48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,  /*  80 */
   64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,  /*  96 */
   80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,  /* 112 */
   96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96,  /* 128 */
   96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96,  /* 144 */
   96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,  /* 160 */
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,  /* 176 */
  128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,  /* 192 */
  144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,  /* 208 */
#if 0
  160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,  /* 224 */
  176,177,178,195,180,181,182,183,184,185,186,187,188,189,190,191   /* 240 */
#else /* see spank_image() */
   96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96,  /* 224 */
   96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96,  /* 240 */
#endif
};

#define CURSOR_GLYPH 97

/* #define DEBUG */

#define GRID_SIZE  70     /* width and height of the arena */
#define GRID_DEPTH 35     /* depth of the arena */
#define WAVE_SIZE  22     /* periodicity of color (brightness) waves */
#define SPLASH_RATIO 0.7  /* ratio of GRID_DEPTH where chars hit the screen */

static const struct { GLfloat x, y; } nice_views[] = {
  {  0,     0 },
  {  0,   -20 },     /* this is a list of viewer rotations that look nice. */
  {  0,    20 },     /* every now and then we switch to a new one.         */
  { 25,     0 },     /* (but we only use the first one at start-up.)       */
  {-25,     0 },
  { 25,    20 },
  {-25,    20 },
  { 25,   -20 },
  {-25,   -20 },

  { 10,     0 },
  {-10,     0 },
  {  0,     0 },  /* prefer these */
  {  0,     0 },
  {  0,     0 },
  {  0,     0 },
  {  0,     0 },
};

matrix_configuration mps;

static GLfloat speed = 1.0f;
static GLfloat density = 20.0f;
static BOOL do_clock = TRUE;
static const char *timefmt = " %H%M ";
static BOOL do_fog = TRUE;
static BOOL do_waves = TRUE;
static BOOL do_rotate = TRUE;
static BOOL do_texture = TRUE;

/* Re-randomize the state of one strip.
 */
static void reset_strip(strip *s)
{
	matrix_configuration *mp = &mps;
	int i;
  	BOOL time_displayed_p = FALSE;  /* never display time twice in one strip */

	memset (s, 0, sizeof(*s));
	s->x = (GLfloat) (frand((double)GRID_SIZE) - (GRID_SIZE / 2.0));
	s->y = (GLfloat) (GRID_SIZE/2.0 + BELLRAND(0.5));      /* shift top slightly */
	s->z = (GLfloat) (GRID_DEPTH * 0.2) - frand (GRID_DEPTH * 0.7);
	s->spinner_y = 0;

	s->dx = 0;
/*	s->dx = ((BELLRAND(0.01) - 0.005) * speed); */
	s->dy = 0;
	s->dz = (BELLRAND(0.02f) * speed);

	s->spinner_speed = (BELLRAND(0.3f) * speed);

	s->spin_speed = (int) BELLRAND(2.0f / speed) + 1;
	s->spin_tick  = 0;

	s->wave_position = 0;
	s->wave_speed = (int) BELLRAND(3.0f / speed) + 1;
	s->wave_tick  = 0;

	for (i = 0; i < GRID_SIZE; i++)
	{
		if (do_clock &&
			!time_displayed_p &&
			(i < GRID_SIZE-5) &&   /* display approx. once per 5 strips */
			(random() % (GRID_SIZE-5)*5 == 0))
		{
			int j;
			char text[80];
			time_t now = time ((time_t *) 0);
			struct tm *tm = localtime (&now);
			strftime (text, sizeof(text)-1, timefmt, tm);

			/* render time into the strip */
			for (j = 0; j < strlen(text) && i < GRID_SIZE; j++, i++)
			{
				s->glyphs[i] = char_map [((unsigned char *) text)[j]] + 1;
				s->highlight[i] = TRUE;
			}

			time_displayed_p = TRUE;	
		}
		else
		{
			int draw_p = (random() % 7);
			int spin_p = (draw_p && !(random() % 20));
			int g = (draw_p
				? mp->glyph_map[(random() % mp->nglyphs)] + 1
				: 0);
			if (spin_p) g = -g;
			s->glyphs[i] = g;
			s->highlight[i] = FALSE;
		}
	}

	s->spinner_glyph = - (mp->glyph_map[(random() % mp->nglyphs)] + 1);
}


/* Animate the strip one step.  Reset if it has reached the bottom.
 */
static void tick_strip(strip *s)
{
    matrix_configuration *mp = &mps;
	int i;

	if (mp->button_down_p)
		return;

	s->x += s->dx;
	s->y += s->dy;
	s->z += s->dz;

	if (s->z > GRID_DEPTH * SPLASH_RATIO)  /* splashed into screen */
	{
		reset_strip (s);
		return;
	}

	s->spinner_y += s->spinner_speed;
	if (s->spinner_y >= GRID_SIZE)
	{
		if (s->erasing_p)
		{
			reset_strip (s);
			return;
		}
		else
		{
			s->erasing_p = TRUE;
			s->spinner_y = 0;
			s->spinner_speed /= 2;  /* erase it slower than we drew it */
		}
	}

	/* Spin the spinners. */
	s->spin_tick++;
	if (s->spin_tick > s->spin_speed)
	{
		s->spin_tick = 0;
		s->spinner_glyph = - (mp->glyph_map[(random() % mp->nglyphs)] + 1);
		for (i = 0; i < GRID_SIZE; i++)
			if (s->glyphs[i] < 0)
			{
				s->glyphs[i] = -(mp->glyph_map[(random() % mp->nglyphs)] + 1);
				if (! (random() % 800))  /* sometimes they stop spinning */
					s->glyphs[i] = -s->glyphs[i];
			}
		}

	/* Move the color (brightness) wave. */
	s->wave_tick++;
	if (s->wave_tick > s->wave_speed)
	{
		s->wave_tick = 0;
		s->wave_position++;
		if (s->wave_position >= WAVE_SIZE)
			s->wave_position = 0;
	}
}


/* Draw a single character at the given position and brightness.
 */
static void draw_glyph(int glyph, BOOL highlight,
					   GLfloat x, GLfloat y, GLfloat z,
					   GLfloat brightness)
{
	matrix_configuration *mp = &mps;
	GLfloat w = mp->tex_char_width;
	GLfloat h = mp->tex_char_height;
	GLfloat cx = 0, cy = 0;
	GLfloat S = 1;
	BOOL spinner_p = (glyph < 0);

	if (glyph == 0) abort();
	if (glyph < 0) glyph = -glyph;

	if (spinner_p)
		brightness *= 1.5f;

	if (!do_texture)
	{
		S  = 0.8;
		x += 0.1;
		y += 0.1;
	}
	else
	{
		int ccx = ((glyph - 1) % CHAR_COLS);
		int ccy = ((glyph - 1) / CHAR_COLS);
		cx = ccx * w;
		cy = (mp->real_char_rows - ccy - 1) * h;

		if (do_fog)
		{
			GLfloat depth;
			depth = (z / GRID_DEPTH) + 0.5f;  /* z ratio from back/front      */
			depth = 0.2f + (depth * 0.8f);     /* scale to range [0.2 - 1.0]   */
			brightness *= depth;             /* so no row goes all black.    */
		}
	}

	{
		GLfloat r, g, b, a;

		if (highlight)
			brightness *= 2;

		if (!do_texture && !spinner_p)
			r = b = 0, g = 1;
		else
			r = g = b = 1;

		a = brightness;

		/*	If the glyph is very close to the screen (meaning it is very large,
			and is about to splash into the screen and vanish) then start fading
			it out, proportional to how close to the glass it is.
		*/

		if (z > GRID_DEPTH/2)
		{
			GLfloat ratio = ((z - GRID_DEPTH/2) / ((GRID_DEPTH * SPLASH_RATIO) - GRID_DEPTH/2));
			int i = ratio * WAVE_SIZE;

			if (i < 0)
			{
				i = 0;
			}
			else if (i >= WAVE_SIZE)
			{
				i = WAVE_SIZE-1;
			}

			a *= mp->brightness_ramp[i];
		}

		glColor4f(r,g,b,a);
	}

	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glTexCoord2f(cx,     cy);     glVertex3f (x,     y,     z);
	glTexCoord2f(cx + w, cy);     glVertex3f (x + S, y,     z);
	glTexCoord2f(cx + w, cy + h); glVertex3f (x + S, y + S, z);
	glTexCoord2f(cx,     cy + h); glVertex3f (x,     y + S, z);
	glEnd ();
}

/* Draw all the visible glyphs in the strip.
 */
static void draw_strip(strip *s)
{
	matrix_configuration *mp = &mps;
	int i;
	for (i = 0; i < GRID_SIZE; i++)
	{
		int g = s->glyphs[i];
		BOOL below_p = (s->spinner_y >= i);

		if (s->erasing_p)
			below_p = !below_p;

		if (g && below_p)       /* don't draw cells below the spinner */
		{
			GLfloat brightness;
			if (!do_waves)
				brightness = 1.0;
			else
			{
				int j = WAVE_SIZE - ((i + (GRID_SIZE - s->wave_position)) % WAVE_SIZE);
				brightness = mp->brightness_ramp[j];
			}

			draw_glyph (g, s->highlight[i], s->x, s->y - i, s->z, brightness);
		}
	}

	if (!s->erasing_p)
		draw_glyph (s->spinner_glyph, FALSE, s->x, s->y - s->spinner_y, s->z, 1.0);
}


/* qsort comparator for sorting strips by z position */
static int
cmp_strips (const void *aa, const void *bb)
{
  const strip *a = *(strip **) aa;
  const strip *b = *(strip **) bb;
  return ((int) (a->z * 10000) -
          (int) (b->z * 10000));
}


/* Auto-tracking
 */

static void auto_track_init()
{
	matrix_configuration *mp = &mps;
	mp->last_view = 0;
	mp->target_view = 0;
	mp->view_x = nice_views[mp->last_view].x;
	mp->view_y = nice_views[mp->last_view].y;
	mp->view_steps = 100;
	mp->view_tick = 0;
	mp->auto_tracking_p = FALSE;
}

static void auto_track()
{
	matrix_configuration *mp = &mps;

	if (! do_rotate)
		return;
	if (mp->button_down_p)
		return;

	/* if we're not moving, maybe start moving.  Otherwise, do nothing. */
	if (! mp->auto_tracking_p)
	{
		if (++mp->track_tick < 20/speed) return;

		mp->track_tick = 0;
		if (! (random() % 20))
			mp->auto_tracking_p = TRUE;
		else
			return;
	}


	{
		GLfloat ox = nice_views[mp->last_view].x;
		GLfloat oy = nice_views[mp->last_view].y;
		GLfloat tx = nice_views[mp->target_view].x;
		GLfloat ty = nice_views[mp->target_view].y;

		/* move from A to B with sinusoidal deltas, so that it doesn't jerk
			to a stop.
		*/

		GLfloat th = sin ((M_PI / 2) * (double) mp->view_tick / mp->view_steps);

		mp->view_x = (ox + ((tx - ox) * th));
		mp->view_y = (oy + ((ty - oy) * th));
		mp->view_tick++;

		if (mp->view_tick >= mp->view_steps)
		{
			mp->view_tick = 0;
			mp->view_steps = (350.0 / speed);
			mp->last_view = mp->target_view;
			mp->target_view = (random() % (countof(nice_views) - 1)) + 1;
			mp->auto_tracking_p = FALSE;
		}
	}
}


/* Window management, etc
 */
void reshape_matrix(int width, int height)
{
	GLfloat h = (GLfloat) height / (GLfloat) width;

	glViewport(0, 0, (GLint) width, (GLint) height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80.0f, 1/h, 1.0f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(	0.0f, 0.0f, 25.0f,
					0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f);

	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
}


#if 1
static BOOL
bigendian (void)
{
  union { int i; char c[sizeof(int)]; } u;
  u.i = 1;
  return !u.c[0];
}
#endif


/* The image with the characters in it is 512x598, meaning that it needs to
   be copied into a 512x1024 texture.  But some machines can't handle textures
   that large...  And it turns out that we aren't using most of the characters
   in that image anyway, since this program doesn't do anything that makes use
   of the full range of Latin1 characters.  So... this function tosses out the
   last 32 of the Latin1 characters, resulting in a 512x506 image, which we
   can then stuff in a 512x512 texture.  Voila.

   If this hack ever grows into something that displays full Latin1 text,
   well then, Something Else Will Need To Be Done.
 */
static void spank_image(XImage *xi)
{
	matrix_configuration *mp = &mps;

	int ch = xi->height / CHAR_ROWS;
	int cut = 2;
	unsigned char *bits = (unsigned char *) xi->data;
	unsigned char *from, *to, *s, *end;
	int L = xi->bytes_per_line * ch;
	/* int i; */

	/* Copy row 12 into 10 (which really means, copy 2 into 0,
		since texture data is upside down.).
	*/
	to   = bits + (L * cut);
	from = bits;
	end  = from + L;
	s    = from;
	while (s < end)
		*to++ = *s++;

	/* Then, pull all the bits down by 2 rows.
	*/
	to   = bits;
	from = bits + (L * cut);
	end  = bits + (L * CHAR_ROWS);
	s    = from;
	while (s < end)
		*to++ = *s++;

	/* And clear out the rest, for good measure.
	*/
	from = bits + (L * (CHAR_ROWS - cut));
	end  = bits + (L * CHAR_ROWS);
	s    = from;
	while (s < end)
		*s++ = 0;

	xi->height -= (cut * ch);
	mp->real_char_rows -= cut;

#if 0
	/* Finally, pull the map indexes back to match the new bits.
	*/
	for (i = 0; i < countof(matrix_encoding); i++)
		if (matrix_encoding[i] > (CHAR_COLS * (CHAR_ROWS - cut)))
			matrix_encoding[i] -= (cut * CHAR_COLS);
#endif
}

static void load_textures(BOOL flip_p, BOOL invertAlpha)
{
	matrix_configuration *mp = &mps;
	XImage *xi;
	unsigned int x, y;
	int cw, ch;
	int orig_w, orig_h;

	/* The Matrix XPM is 512x598 -- but GL texture sizes must be powers of 2.
		So we waste some padding rows to round up.
	*/

	xi = inmemory_png_to_ximage(matrix3_png);
	if (NULL != xi)
	{
		orig_w = xi->width;
		orig_h = xi->height;
		mp->real_char_rows = CHAR_ROWS;

		spank_image(xi);

		if (xi->height != 512 && xi->height != 1024)
		{
			uint32 oldsize = xi->height * xi->bytes_per_line;
			xi->height = (xi->height < 512 ? 512 : 1024);
			uint32 newsize = xi->height * xi->bytes_per_line;
			APTR* temp = IExec->AllocVecTags(xi->height * xi->bytes_per_line, AVT_ClearWithValue, 0, TAG_DONE);

			if (oldsize<=newsize)
				IExec->CopyMem(xi->data, temp, oldsize);
			else
				IExec->CopyMem(xi->data, temp, newsize);
			
			IExec->FreeVec(xi->data);
			xi->data = (unsigned long*)temp;
			if (!xi->data)
			{
				exit(1);
			}
		}

		if (xi->width != 512) abort();
		if (xi->height != 512 && xi->height != 1024) abort();

		/* char size in pixels */
		cw = orig_w / CHAR_COLS;
		ch = orig_h / CHAR_ROWS;

		/* char size in ratio of final (padded) texture size */
		mp->tex_char_width  = (GLfloat) cw / xi->width;
		mp->tex_char_height = (GLfloat) ch / xi->height;

		/* Flip each character's bits horizontally -- we could also just do this
			by reversing the texture coordinates on the quads, but on some systems
			that slows things down a lot.
		*/
		if (flip_p)
		{
			int xx, col;
			unsigned long buf[100];
			for (y = 0; y < xi->height - (2 * ch) - 3 ; y++)
			{
				for (col = 0, xx = 0; col < CHAR_COLS; col++, xx += cw)
				{
					for (x = 0; x < cw; x++)
						buf[x] = XGetPixel (xi, xx+x, y);
					for (x = 0; x < cw; x++)
						XPutPixel (xi, xx+x, y, buf[cw-x-1]);
				}
			}
		}

		/*	The pixmap is a color image with no transparency.  Set the texture's
			alpha to be the green channel, and set the green channel to be 100%.
		*/
		{
			int rpos, gpos, bpos, apos;  /* bitfield positions */
#if 1
			/*	#### Cherub says that the little-endian case must be taken on MacOSX,
				or else the colors/alpha are the wrong way around.  How can
				that be the case?
			*/

			if (bigendian())
			{
				rpos = 24, gpos = 16, bpos =  8, apos =  0;
			}
			else
#endif
			{
				rpos =  0, gpos =  8, bpos = 16, apos = 24;
			}
			
			for (y = 0; y < xi->height; y++)
			{
				for (x = 0; x < xi->width; x++)
				{
					unsigned long p = XGetPixel (xi, x, y);
					unsigned char r = (p >> rpos) & 0xFF;
					unsigned char g = (p >> gpos) & 0xFF;
					unsigned char b = (p >> bpos) & 0xFF;
					unsigned char a = g;
					g = 0xFF;
					p = (r << rpos) | (g << gpos) | (b << bpos) | (a << apos);
					XPutPixel (xi, x, y, p);
				}
			}
		}

		/*	Now load the texture into GL. */
		/*	clear_gl_error(); */
		glGenTextures (1, &mp->texture);

		glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
		/*glPixelStorei (GL_UNPACK_ROW_LENGTH, xi->width);*/
		glBindTexture (GL_TEXTURE_2D, mp->texture);
		/*  check_gl_error ("texture init"); */
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, xi->width, xi->height, 0, GL_RGBA,
					GL_UNSIGNED_BYTE, xi->data);
	/*	{
			char buf[255];
			sprintf (buf, "creating %dx%d texture:", xi->width, xi->height);
			check_gl_error (buf);
		}*/

		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		/*	I'd expect CLAMP to be the thing to do here, but oddly, we get a
			faint solid green border around the texture if it is *not* REPEAT!
		*/
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	/*	check_gl_error ("texture param"); */

		/*xi->data = 0;*/ /*  don't free the texture data */
		XDestroyImage(xi);
	}
}


void init_matrix(struct BlankerData* bd)
{
	matrix_configuration *mp = &mps;
	BOOL flip_p = FALSE;
	int i;

	/*if (mp->strips != NULL)
	{
		// Reinit
		IExec->FreeVec(mp->strips);
	}
	*/
	mp->strips = NULL;

	mp->button_down_p = FALSE;

	speed = (GLfloat)bd->speed / 100.0f;
	density = (GLfloat)bd->density;
	do_fog = bd->fog;
	do_waves = bd->wave;
	do_rotate = bd->rotate;

	switch (bd->encoding)
	{
		case 0:
			flip_p = 1;
			mp->glyph_map = matrix_encoding;
			mp->nglyphs   = countof(matrix_encoding);
			break;
		case 1:
			flip_p = 0;
			mp->glyph_map = dna_encoding;
			mp->nglyphs   = countof(dna_encoding);
			break;
		case 2:
			flip_p = 0;
			mp->glyph_map = binary_encoding;
			mp->nglyphs   = countof(binary_encoding);
			break;
		case 3:
			flip_p = 0;
			mp->glyph_map = hex_encoding;
			mp->nglyphs   = countof(hex_encoding);
			break;
		case 4:
			flip_p = 0;
			mp->glyph_map = decimal_encoding;
			mp->nglyphs   = countof(decimal_encoding);
			break;
		default:
			exit (1);
	}

	glShadeModel(GL_SMOOTH);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_NORMALIZE);

	if (do_texture)
	{
		load_textures(flip_p, FALSE);
	}

	/* to scale coverage-percent to strips, this number looks about right... */
	mp->nstrips = (int)(density * 2.2);
	if	(mp->nstrips < 1) mp->nstrips = 1;
	else if	(mp->nstrips > 2000) mp->nstrips = 2000;

	mp->strips = IExec->AllocVecTags(mp->nstrips * sizeof(strip), AVT_ClearWithValue, 0, TAG_DONE);
	for (i = 0; i < mp->nstrips; i++)
	{
		strip *s = &mp->strips[i];
		reset_strip(s);

		/*	If we start all strips from zero at once, then the first few seconds
			of the animation are much denser than normal.  So instead, set all
			the initial strips to erase-mode with random starting positions.
			As these die off at random speeds and are re-created, we'll get a
			more consistent density. */

		s->erasing_p = TRUE;
		s->spinner_y = frand(GRID_SIZE);
		memset (s->glyphs, 0, sizeof(s->glyphs));  /* no visible glyphs */
	}

	/*	Compute the brightness ramp.
	*/

	for (i = 0; i < WAVE_SIZE; i++)
	{
		GLfloat j = ((WAVE_SIZE - i) / (GLfloat) (WAVE_SIZE - 1));
		j *= (M_PI / 2.0f);		/* j ranges from 0.0 - PI/2  */
		j = sin(j);			/* j ranges from 0.0 - 1.0   */
		j = 0.2 + (j * 0.8);	/* j ranges from 0.2 - 1.0   */
		mp->brightness_ramp[i] = j;	/* printf("%2d %8.2f\n", i, j); */
	}

	auto_track_init();
}


#ifdef DEBUG

static void draw_grid()
{
/*	if (!MI_IS_WIREFRAME(mi))
	{*/
		glDisable(GL_TEXTURE_2D);
/*		glDisable(GL_BLEND);
	} */
	glPushMatrix();
	glColor3f(1, 1, 1);
	glBegin(GL_LINES);
	glVertex3f(-GRID_SIZE, 0, 0); glVertex3f(GRID_SIZE, 0, 0);
	glVertex3f(0, -GRID_SIZE, 0); glVertex3f(0, GRID_SIZE, 0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(-GRID_SIZE/2, -GRID_SIZE/2, 0);
	glVertex3f(-GRID_SIZE/2,  GRID_SIZE/2, 0);
	glVertex3f( GRID_SIZE/2,  GRID_SIZE/2, 0);
	glVertex3f( GRID_SIZE/2, -GRID_SIZE/2, 0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(-GRID_SIZE/2, GRID_SIZE/2, -GRID_DEPTH/2);
	glVertex3f(-GRID_SIZE/2, GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2, GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2, GRID_SIZE/2, -GRID_DEPTH/2);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(-GRID_SIZE/2, -GRID_SIZE/2, -GRID_DEPTH/2);
	glVertex3f(-GRID_SIZE/2, -GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2, -GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2, -GRID_SIZE/2, -GRID_DEPTH/2);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(-GRID_SIZE/2, -GRID_SIZE/2, -GRID_DEPTH/2);
	glVertex3f(-GRID_SIZE/2,  GRID_SIZE/2, -GRID_DEPTH/2);
	glVertex3f(-GRID_SIZE/2, -GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f(-GRID_SIZE/2,  GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2, -GRID_SIZE/2, -GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2,  GRID_SIZE/2, -GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2, -GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2,  GRID_SIZE/2,  GRID_DEPTH/2);
	glEnd();
	glPopMatrix();
/*	if (!MI_IS_WIREFRAME(mi))
    {*/
      glEnable(GL_TEXTURE_2D);
/*		glEnable(GL_BLEND);
    } */
}
#endif /* DEBUG */


void draw_matrix()
{
	matrix_configuration *mp = &mps;
	int i;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix ();
  if (do_texture)
    {
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);

      /* Jeff Epler points out:
         By using GL_ONE instead of GL_SRC_ONE_MINUS_ALPHA, glyphs are
         added to each other, so that a bright glyph with a darker one
         in front is a little brighter than the bright glyph alone.
       */
      glBlendFunc (GL_SRC_ALPHA, GL_ONE);
    }

	if (do_rotate)
	{
		glRotatef (mp->view_x, 1, 0, 0);
		glRotatef (mp->view_y, 0, 1, 0);
	}

#ifdef DEBUG
# if 0
	glScalef(0.5, 0.5, 0.5);
# endif
# if 0
	glRotatef(-30, 0, 1, 0);
# endif
	draw_grid();
#endif

	/*	Render (and tick) each strip, starting at the back
		(draw the ones farthest from the camera first, to make
		the alpha transparency work out right.)
	*/
	{
		strip **sorted = IExec->AllocVecTags(mp->nstrips * sizeof(*sorted), AVT_ClearWithValue, 0, TAG_DONE);
		for (i = 0; i < mp->nstrips; i++)
		{
			sorted[i] = &mp->strips[i];
		}

		qsort(sorted, i, sizeof(*sorted), cmp_strips);
		for (i = 0; i < mp->nstrips; i++)
		{
			strip *s = sorted[i];
			tick_strip(s);
			draw_strip(s);
		}
		IExec->FreeVec(sorted);
	}

	auto_track();

#if 0
	glBegin(GL_QUADS);
	glColor3f(1,1,1);
	glTexCoord2f (0,0);  glVertex3f(-15,-15,0);
	glTexCoord2f (0,1);  glVertex3f(-15,15,0);
	glTexCoord2f (1,1);  glVertex3f(15,15,0);
	glTexCoord2f (1,0);  glVertex3f(15,-15,0);
	glEnd();
#endif

	glPopMatrix ();
}

#endif /* USE_GL */


