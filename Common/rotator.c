/* xscreensaver, Copyright (c) 1998-2002 Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#include <math.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include "rotator.h"
#include "yarandom.h"
#include <proto/exec.h>

struct rotator {

  double spin_x_speed, spin_y_speed, spin_z_speed;
  double wander_speed;

  double rotx, roty, rotz;	   /* current object rotation */
  double dx, dy, dz;		   /* current rotational velocity */
  double ddx, ddy, ddz;		   /* current rotational acceleration */
  double d_max;			   /* max rotational velocity */

  int wander_frame;		   /* position in the wander cycle */

};


#undef ABS
#define ABS(x) ((x)<0?-(x):(x))

#define BELLRAND(n) ((frand((n)) + frand((n)) + frand((n))) / 3)
#define RANDSIGN() ((random() & 1) ? 1 : -1)

static void
rotate_1 (double *pos, double *v, double *dv, double speed, double max_v)
{
  double ppos = *pos;

  if (speed == 0) return;

  /* tick position */
  if (ppos < 0)
    ppos = -(ppos + *v);
  else
    ppos += *v;

  if (ppos > 1.0)
    ppos -= 1.0;
  else if (ppos < 0)
    ppos += 1.0;

  if (ppos < 0) abort();
  if (ppos > 1.0) abort();
  *pos = (*pos > 0 ? ppos : -ppos);

  /* accelerate */
  *v += *dv;

  /* clamp velocity */
  if (*v > max_v || *v < -max_v)
    {
      *dv = -*dv;
    }
  /* If it stops, start it going in the other direction. */
  else if (*v < 0)
    {
      if (random() % 4)
	{
	  *v = 0;

	  /* keep going in the same direction */
	  if (random() % 2)
	    *dv = 0;
	  else if (*dv < 0)
	    *dv = -*dv;
	}
      else
	{
	  /* reverse gears */
	  *v = -*v;
	  *dv = -*dv;
	  *pos = -*pos;
	}
    }

  /* Alter direction of rotational acceleration randomly. */
  if (! (random() % 120))
    *dv = -*dv;

  /* Change acceleration very occasionally. */
  if (! (random() % 200))
    {
      if (*dv == 0)
	*dv = 0.00001;
      else if (random() & 1)
	*dv *= 1.2;
      else
	*dv *= 0.8;
    }
}


/* Returns a rotator object, which encapsulates rotation and motion state.

   spin_[xyz]_speed indicates the relative speed of rotation.
   Specify 0 if you don't want any rotation around that axis.

   spin_accel specifies a scaling factor for the acceleration that is
   randomly applied to spin: if you want the speed to change faster,
   make this > 1.

   wander_speed indicates the relative speed through space.

   If randomize_initial_state_p is true, then the initial position and
   rotation will be randomized (even if the spin speeds are 0.)  If it
   is false, then all values will be initially zeroed.
 */
rotator *
make_rotator (double spin_x_speed,
              double spin_y_speed,
              double spin_z_speed,
              double spin_accel,
              double wander_speed,
              int randomize_initial_state_p)
{
  rotator *r = (rotator *) IExec->AllocVecTags(sizeof(*r), AVT_ClearWithValue, 0, TAG_DONE);
  double d, dd;

  if (!r) return 0;

  if (spin_x_speed < 0 || spin_y_speed < 0 || spin_z_speed < 0 ||
      wander_speed < 0)
    abort();

  r->spin_x_speed = spin_x_speed;
  r->spin_y_speed = spin_y_speed;
  r->spin_z_speed = spin_z_speed;
  r->wander_speed = wander_speed;

  if (randomize_initial_state_p)
    {
      r->rotx = frand(1.0) * RANDSIGN();
      r->roty = frand(1.0) * RANDSIGN();
      r->rotz = frand(1.0) * RANDSIGN();

      r->wander_frame = random() % 0xFFFF;
    }
  else
    {
      r->rotx = r->roty = r->rotz = 0;
      r->wander_frame = 0;
    }

  d  = 0.006;
  dd = 0.00006;

  r->dx = BELLRAND(d * r->spin_x_speed);
  r->dy = BELLRAND(d * r->spin_y_speed);
  r->dz = BELLRAND(d * r->spin_z_speed);

  r->d_max = r->dx * 2;

  r->ddx = (dd + frand(dd+dd)) * r->spin_x_speed * spin_accel;
  r->ddy = (dd + frand(dd+dd)) * r->spin_y_speed * spin_accel;
  r->ddz = (dd + frand(dd+dd)) * r->spin_z_speed * spin_accel;

# if 0
  fprintf (stderr, "rotator:\n");
  fprintf (stderr, "   wander: %3d %6.2f\n", r->wander_frame, r->wander_speed);
  fprintf (stderr, "    speed: %6.2f %6.2f %6.2f\n",
           r->spin_x_speed, r->spin_y_speed, r->spin_z_speed);
  fprintf (stderr, "      rot: %6.2f %6.2f %6.2f\n",
           r->rotx, r->roty, r->rotz);
  fprintf (stderr, "        d: %6.2f %6.2f %6.2f, %6.2f\n",
           r->dx, r->dy, r->dz,
           r->d_max);
  fprintf (stderr, "       dd: %6.2f %6.2f %6.2f\n",
           r->ddx, r->ddy, r->ddz);
# endif

  return r;
}


void
free_rotator (rotator *r)
{
  free (r);
}

void
get_rotation (rotator *rot, double *x_ret, double *y_ret, double *z_ret,
              int update_p)
{
  double x, y, z;

  if (update_p) {
    rotate_1 (&rot->rotx, &rot->dx, &rot->ddx, rot->spin_x_speed, rot->d_max);
    rotate_1 (&rot->roty, &rot->dy, &rot->ddy, rot->spin_y_speed, rot->d_max);
    rotate_1 (&rot->rotz, &rot->dz, &rot->ddz, rot->spin_z_speed, rot->d_max);
  }

  x = rot->rotx;
  y = rot->roty;
  z = rot->rotz;
  if (x < 0) x = 1 - (x + 1);
  if (y < 0) y = 1 - (y + 1);
  if (z < 0) z = 1 - (z + 1);

  if (x_ret) *x_ret = x;
  if (y_ret) *y_ret = y;
  if (z_ret) *z_ret = z;
}


void
get_position (rotator *rot, double *x_ret, double *y_ret, double *z_ret,
              int update_p)
{
  double x = 0.5, y = 0.5, z = 0.5;

  if (rot->wander_speed != 0)
    {
      if (update_p)
        rot->wander_frame++;

# define SINOID(F) ((1 + sin((rot->wander_frame * (F)) / 2 * M_PI)) / 2.0)
      x = SINOID (0.71 * rot->wander_speed);
      y = SINOID (0.53 * rot->wander_speed);
      z = SINOID (0.37 * rot->wander_speed);
# undef SINOID
    }

  if (x_ret) *x_ret = x;
  if (y_ret) *y_ret = y;
  if (z_ret) *z_ret = z;
}
