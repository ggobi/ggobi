/* pipeline_r.c: the reverse pipeline code */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*
 * note: world.data includes jitter.data
*/

void
world_to_raw_by_var (gint pt, gint var)
{
  gfloat precis = PRECISION1;
  gfloat ftmp, rdiff;

  rdiff = gg.lim[var].max - gg.lim[var].min;

  /*
   * The new world.data value is taken to include the
   * value of jitter.data
  */
  ftmp = (gfloat)(gg.world.data[pt][var] - gg.jitter.data[pt][var]) / precis;

  gg.tform.data[pt][var] = (ftmp + 1.0) * .5 * rdiff;
  gg.tform.data[pt][var] += gg.lim[var].min;

  gg.raw.data[pt][var] = gg.tform.data[pt][var];
}

void
world_to_raw (cpaneld *cpanel, splotd *sp, gint pt)
{
  gint i;

  switch (cpanel->projection) {
    case P1PLOT:
      world_to_raw_by_var (pt, sp->p1dvar);
      break;

    case XYPLOT:
      world_to_raw_by_var (pt, sp->xyvars.x);
      world_to_raw_by_var (pt, sp->xyvars.y);
      break;

    case ROTATE:
      world_to_raw_by_var (pt, sp->spinvars.x);
      world_to_raw_by_var (pt, sp->spinvars.y);
      world_to_raw_by_var (pt, sp->spinvars.z);
      break;

    case TOUR2D:
      for (i=0; i<sp->n_tourvars; i++)
        world_to_raw_by_var (pt, sp->tourvars[i]);
      break;

    case COTOUR:
      for (i=0; i<sp->n_corrvars_x; i++)
        world_to_raw_by_var (pt, sp->corrvars_x[i]);
      for (i=0; i<sp->n_corrvars_y; i++)
        world_to_raw_by_var (pt, sp->corrvars_y[i]);
      break;
  }
}

