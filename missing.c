/* missing.c */

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
missing_world_free (ggobid *gg)
{
  arrayl_free (&gg->missing_world, 0, 0);
  arrayl_free (&gg->missing_jitter, 0, 0);
}

void
missing_world_alloc (ggobid *gg)
{
  if (gg->missing_world.vals != NULL) missing_world_free (gg);

  arrayl_alloc (&gg->missing_world, gg->nrows, gg->ncols);
  arrayl_alloc (&gg->missing_jitter, gg->nrows, gg->ncols);
}

void
missing_arrays_add_column (gint jvar, ggobid *gg)
{
  gint nc = gg->ncols + 1;

  if (gg->nmissing > 0) {
    arrays_add_cols (&gg->missing, nc);
    arrayl_add_cols (&gg->missing_jitter, nc);
    arrayl_add_cols (&gg->missing_world, nc);
  }
}


/*------------------------------------------------------------------*/
/*      Scaling and jittering missing value plots                   */
/*------------------------------------------------------------------*/

void
missing_lim_set (ggobid *gg)
{
  gint i, j;
  gshort min, max;

  min = max = gg->missing.vals[0][0];
  for (i=0; i<gg->nrows; i++) {
    for (j=0; j<gg->ncols; j++) {
      if (gg->missing.vals[i][j] < min) min = gg->missing.vals[i][j];
      if (gg->missing.vals[i][j] > max) max = gg->missing.vals[i][j];
    }
  }
  gg->missing_lim.min = (gfloat) min;
  gg->missing_lim.max = (gfloat) max;
}

/*
 * This jitters the missings indicator vector for one variable
*/
void
missing_jitter_variable (gint jcol, ggobid *gg)
{
  gint i, m;
  gfloat precis = (gfloat) PRECISION1;
  gfloat frand, fworld, fjit;

  for (i=0; i<gg->nrows; i++) {
    m = gg->rows_in_plot[i];

    frand = jitter_randval (gg->jitter.type) * precis;

    if (gg->jitter.convex) {
      fworld = (gfloat)
        (gg->missing_world.vals[m][jcol] - gg->missing_jitter.vals[m][jcol]);
      fjit = gg->missing_jitter_factor * (frand - fworld);
    } else
      fjit = gg->missing_jitter_factor * frand;

    gg->missing_jitter.vals[m][jcol] = (glong) fjit;
  }
}

void
missing_jitter_value_set (gfloat value, ggobid *gg) {
  gg->missing_jitter_factor = (value > 0) ? value : 0;
}

static void
missing_jitter_init (ggobid *gg)
{
  gint j;

  missing_jitter_value_set (.2, gg);
  for (j=0; j<gg->ncols; j++)
    missing_jitter_variable (j, gg);
}

void
missing_to_world (ggobid *gg)
{
  gint i, j, m;
  gfloat ftmp;
  gfloat precis = PRECISION1;
  gfloat min = gg->missing_lim.min;
  gfloat range = gg->missing_lim.max - gg->missing_lim.min;
  static gboolean initd = false;

  if (!initd) {
    missing_jitter_init (gg);
    initd = true;
  }

  for (j=0; j<gg->ncols; j++) {
    for (i=0; i<gg->nrows_in_plot; i++) {
      m = gg->rows_in_plot[i];
      ftmp = -1.0 + 2.0*(gg->missing.vals[m][j] - min) / range;
      gg->missing_world.vals[m][j] = (glong) (precis * ftmp);

      /* Add in the jitter values */
      gg->missing_world.vals[m][j] += gg->missing_jitter.vals[m][j];
    }
  }
}

void
missing_rejitter (ggobid *gg) {
  gint j;

/*
 * This rejitters <everything> which is perhaps excessive, no?
*/
  for (j=0; j<gg->ncols; j++)
    missing_jitter_variable (j, gg);

  missing_to_world (gg);

  /*-- redisplay only the missings displays --*/
  displays_tailpipe (REDISPLAY_MISSING, gg); 
}
