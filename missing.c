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
missing_world_free ()
{
  arrayl_free (&xg.missing_world, 0, 0);
  arrayl_free (&xg.missing_jitter, 0, 0);
}

void
missing_world_alloc ()
{
  if (xg.missing_world.data != NULL) missing_world_free ();

  arrayl_alloc (&xg.missing_world, xg.nrows, xg.ncols);
  arrayl_alloc (&xg.missing_jitter, xg.nrows, xg.ncols);
}

void
missing_arrays_add_column (gint jvar)
{
  gint nc = xg.ncols + 1;

  if (xg.nmissing > 0) {
    arrays_add_cols (&xg.missing, nc);
    arrayl_add_cols (&xg.missing_jitter, nc);
    arrayl_add_cols (&xg.missing_world, nc);
  }
}


/*------------------------------------------------------------------*/
/*      Scaling and jittering missing value plots                   */
/*------------------------------------------------------------------*/

void
missing_lim_set ()
{
  gint i, j;
  gshort min, max;

  min = max = xg.missing.data[0][0];
  for (i=0; i<xg.nrows; i++) {
    for (j=0; j<xg.ncols; j++) {
      if (xg.missing.data[i][j] < min) min = xg.missing.data[i][j];
      if (xg.missing.data[i][j] > max) max = xg.missing.data[i][j];
    }
  }
  xg.missing_lim.min = (gfloat) min;
  xg.missing_lim.max = (gfloat) max;
}

/*
 * This jitters the missings indicator vector for one variable
*/
void
missing_jitter_variable (gint jcol)
{
  gint i, m;
  gfloat precis = (gfloat) PRECISION1;
  gfloat frand, fworld, fjit;

  for (i=0; i<xg.nrows; i++) {
    if ((m = xg.rows_in_plot[i]) >= xg.nlinkable)
      break;
    else {

      frand = jitter_randval (xg.jitter_type) * precis;

      if (xg.jitter_convex) {
        fworld = (gfloat)
          (xg.missing_world.data[m][jcol] - xg.missing_jitter.data[m][jcol]);
        fjit = xg.missing_jitter_factor * (frand - fworld);
      } else
        fjit = xg.missing_jitter_factor * frand;

      xg.missing_jitter.data[m][jcol] = (glong) fjit;
    }
  }
}

void
missing_jitter_value_set (gfloat value) {
  xg.missing_jitter_factor = (value > 0) ? value : 0;
}

static void
missing_jitter_init ()
{
  gint j;

  missing_jitter_value_set (.2);
  for (j=0; j<xg.ncols; j++)
    missing_jitter_variable (j);
}

void
missing_to_world ()
{
  gint i, j, m;
  gfloat ftmp;
  gfloat precis = PRECISION1;
  gfloat min = xg.missing_lim.min;
  gfloat range = xg.missing_lim.max - xg.missing_lim.min;
  static gboolean initd = false;

  if (!initd) {
    missing_jitter_init ();
    initd = true;
  }

  for (j=0; j<xg.ncols; j++) {
    for (i=0; i<xg.nrows_in_plot; i++) {
      m = xg.rows_in_plot[i];
      ftmp = -1.0 + 2.0*(xg.missing.data[m][j] - min) / range;
      xg.missing_world.data[m][j] = (glong) (precis * ftmp);

      /* Add in the jitter values */
      xg.missing_world.data[m][j] += xg.missing_jitter.data[m][j];
    }
  }
}

void
missing_rejitter () {
  gint j;

/*
 * This rejitters <everything> which is perhaps excessive, no?
*/
  for (j=0; j<xg.ncols; j++)
    missing_jitter_variable (j);

  missing_to_world ();

  /*-- redisplay only the missings displays --*/
  displays_tailpipe (REDISPLAY_MISSING); 
}
