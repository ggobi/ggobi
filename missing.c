/* missing.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*--------------------------------------------------------------------*/
/*             Memory management routines                             */
/*--------------------------------------------------------------------*/

void
missing_world_free (datad *d, ggobid *gg)
{
  arrayl_free (&d->missing_world, 0, 0);
  arrayl_free (&d->missing_jitter, 0, 0);
}

void
missing_world_alloc (datad *d, ggobid *gg)
{
  if (d->missing_world.vals != NULL) missing_world_free (d, gg);

  arrayl_alloc (&d->missing_world, d->nrows, d->ncols);
  arrayl_alloc (&d->missing_jitter, d->nrows, d->ncols);
}

void
missing_arrays_add_cols (gint jvar, datad *d, ggobid *gg)
{
  if (d->nmissing > 0 && d->missing.ncols < d->ncols) {
    arrays_add_cols (&d->missing, d->ncols);
    arrayl_add_cols (&d->missing_jitter, d->ncols);
    arrayl_add_cols (&d->missing_world, d->ncols);
  }
}

void
missing_arrays_add_rows (gint nrows, datad *d)
{
  if (d->nmissing > 0) {
    arrays_add_rows (&d->missing, nrows);
    arrayl_add_rows (&d->missing_jitter, nrows);
    arrayl_add_rows (&d->missing_world, nrows);
  }
}

/*------------------------------------------------------------------*/
/*      Scaling and jittering missing value plots                   */
/*------------------------------------------------------------------*/

void
missing_lim_set (datad *d, ggobid *gg)
{
  gint i, j;
  gshort min, max;

  min = max = d->missing.vals[0][0];
  for (i=0; i<d->nrows; i++) {
    for (j=0; j<d->ncols; j++) {
      if (d->missing.vals[i][j] < min) min = d->missing.vals[i][j];
      if (d->missing.vals[i][j] > max) max = d->missing.vals[i][j];
    }
  }
  d->missing_lim.min = (gfloat) min;
  d->missing_lim.max = (gfloat) max;
}

/*
 * This jitters the missings indicator vector for one variable
*/
void
missing_jitter_variable (gint jcol, datad *d, ggobid *gg)
{
  gint i, m;
  gfloat precis = (gfloat) PRECISION1;
  gfloat frand, fworld, fjit;

  for (i=0; i<d->nrows; i++) {
    m = d->rows_in_plot[i];

    frand = jitter_randval (d->jitter.type) * precis;

    if (d->jitter.convex) {
      fworld = (gfloat)
        (d->missing_world.vals[m][jcol] - d->missing_jitter.vals[m][jcol]);
      fjit = d->missing_jitter_factor * (frand - fworld);
    } else
      fjit = d->missing_jitter_factor * frand;

    d->missing_jitter.vals[m][jcol] = (glong) fjit;
  }
}

void
missing_jitter_value_set (gfloat value, datad *d, ggobid *gg) {
  d->missing_jitter_factor = (value > 0) ? value : 0;
}

static void
missing_jitter_init (datad *d, ggobid *gg)
{
  gint j;

  missing_jitter_value_set (.2, d, gg);
  for (j=0; j<d->ncols; j++)
    missing_jitter_variable (j, d, gg);
}

void
missing_to_world (datad *d, ggobid *gg)
{
  gint i, j, m;
  gfloat ftmp;
  gfloat precis = PRECISION1;
  gfloat min = d->missing_lim.min;
  gfloat range = d->missing_lim.max - d->missing_lim.min;
  static gboolean initd = false;

  if (!initd) {
    missing_jitter_init (d, gg);
    initd = true;
  }

  for (j=0; j<d->ncols; j++) {
    for (i=0; i<d->nrows_in_plot; i++) {
      m = d->rows_in_plot[i];
      ftmp = -1.0 + 2.0*(d->missing.vals[m][j] - min) / range;
      d->missing_world.vals[m][j] = (glong) (precis * ftmp);

      /* Add in the jitter values */
      d->missing_world.vals[m][j] += d->missing_jitter.vals[m][j];
    }
  }
}

void
missing_rejitter (datad *d, ggobid *gg) {
  gint j;

/*
 * This rejitters <everything> which is perhaps excessive, no?
*/
  for (j=0; j<d->ncols; j++)
    missing_jitter_variable (j, d, gg);

  missing_to_world (d, gg);

  /*-- redisplay only the missings displays --*/
  displays_tailpipe (REDISPLAY_MISSING, gg); 
}
