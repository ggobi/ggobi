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
missing_block_alloc (gint nblocks, gint bsize)
{  
  /*-- Allocate missing for nblocks*BLOCKSIZE rows; called from read_array --*/
  gint i;
  gulong nr = nblocks * bsize;

  if (nblocks == 1)
    xg.missing = (gshort **) g_malloc0 (nr * sizeof (gshort *));
  else
    xg.missing = (gshort **) g_realloc ((gpointer) xg.missing,
      nr * sizeof (gshort *));

  for (i=bsize*(nblocks-1); i<bsize*nblocks; i++)
    xg.missing[i] = (gshort *) g_malloc0 (xg.ncols * sizeof (gshort));
}

void
missing_alloc (gint nr, gint nc)
{
  /*-- Allocate the missing values array; called from read_array --*/
  gint i;

  xg.missing = (gshort **) g_malloc (nr * sizeof (gshort *));
  for (i=0; i<nr; i++) {
    xg.missing[i] = (gshort *) g_malloc0 (nc * sizeof (gshort));
  }
}

void
missing_world_alloc ()
{
  gint i;

  xg.missing_world_data = (glong **) g_malloc (xg.nrows * sizeof (glong *));
  xg.missing_jitter_data = (glong **) g_malloc (xg.nrows * sizeof (glong *));

  for (i=0; i<xg.nrows; i++) {
    xg.missing_world_data[i] = (glong *) g_malloc0 (xg.ncols * sizeof (glong));
    xg.missing_jitter_data[i] = (glong *) g_malloc0 (xg.ncols * sizeof (glong));
  }
}

void
missing_arrays_add_column (gint jvar)
{
  gint nc = xg.ncols + 1, nr = xg.nrows;
  gint i;

  if (xg.nmissing > 0) {
    for (i=0; i<nr; i++) {
      xg.missing[i] = (gshort *)
        g_realloc ((gpointer) xg.missing[i], nc * sizeof (gshort));
      xg.missing[i][nc-1] = xg.missing[i][jvar];

      xg.missing_jitter_data[i] = (glong *)
        g_realloc ((gpointer) xg.missing_jitter_data[i], nc * sizeof (glong));
      xg.missing_jitter_data[i][nc-1] = xg.missing_jitter_data[i][jvar];

      xg.missing_world_data[i] = (glong *)
        g_realloc ((gpointer) xg.missing_world_data[i], nc * sizeof (glong));
      xg.missing_world_data[i][nc-1] = xg.missing_world_data[i][jvar];
    }
  }
}

void
missing_free () 
{
  gint i;

  if (xg.nmissing) {
    for (i=0; i<xg.nrows; i++)
      g_free ((gpointer) xg.missing[i]);
    g_free ((gpointer) xg.missing);

    for (i=0; i<xg.nrows; i++) {
      g_free ((gpointer) xg.missing_world_data[i]);
      g_free ((gpointer) xg.missing_jitter_data[i]);
    }
    g_free ((gpointer) xg.missing_world_data);
    g_free ((gpointer) xg.missing_jitter_data);
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

  min = max = xg.missing[0][0];
  for (i=0; i<xg.nrows; i++) {
    for (j=0; j<xg.ncols; j++) {
      if (xg.missing[i][j] < min) min = xg.missing[i][j];
      if (xg.missing[i][j] > max) max = xg.missing[i][j];
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
          (xg.missing_world_data[m][jcol] - xg.missing_jitter_data[m][jcol]);
        fjit = xg.missing_jitter_factor * (frand - fworld);
      } else
        fjit = xg.missing_jitter_factor * frand;

      xg.missing_jitter_data[m][jcol] = (glong) fjit;
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
      ftmp = -1.0 + 2.0*(xg.missing[m][j] - min) / range;
      xg.missing_world_data[m][j] = (glong) (precis * ftmp);

      /* Add in the jitter values */
      xg.missing_world_data[m][j] += xg.missing_jitter_data[m][j];
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
