/* jitter.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
/*
 * Contains jittering routines for tform; see missing.c for the
 * jittering routines for missing data.
*/

#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

gfloat
jitter_randval (gint type)
{
/*
 * generate a random value.
*/
  gdouble drand;
  static gdouble dsave;
  static gboolean isave = false;

  if (type == UNIFORM) {
    drand = randvalue ();
    /*
     * Center and scale to [-1, 1]
    */
    drand = (drand - .5) * 2;

  } else if (type == NORMAL) {

    gboolean check = true;
    gdouble d, dfac;

    if (isave) {
      isave = false;
      /* prepare to return the previously saved value */
      drand = dsave;
    } else {
      isave = true;
      while (check) {

        rnorm2 (&drand, &dsave);
        d = drand*drand + dsave*dsave;

        if (d < 1.0) {
          check = false;
          dfac = sqrt (-2. * log (d) / d);
          drand = drand * dfac;
          dsave = dsave * dfac;
        }
      } /* end while */
    } /* end else */

    /*
     * Already centered; scale to approximately [-1, 1]
    */
    drand = (drand / 3.0);
  }
  return ((gfloat) drand);
}

void
rejitter (datad *d, ggobid *gg) {
  gint *selected_cols, nselected_cols = 0;
  gint i, j, k, m;
  gfloat frand, fworld, fjit;
  gfloat precis = (gfloat) PRECISION1;

/*
 * First determine the variables to be jittered:
 * this depends first on the selected variables and
 * second on vgroups (if jitter_vgroup is True)
*/
  selected_cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  nselected_cols = selected_cols_get (selected_cols, d->jitter.vgroup, d, gg);
  if (nselected_cols == 0)
    nselected_cols = plotted_cols_get (selected_cols, false, d, gg);

  for (j=0; j<nselected_cols; j++) {
    k = selected_cols[j];

    for (i=0; i<d->nrows_in_plot; i++) {
      m = d->rows_in_plot[i];
      /*-- jitter_one_value (m, k); --*/

      frand = jitter_randval (d->jitter.type) * precis;

      /*
       * The world.vals used here is already jittered:
       * subtract out the previous jittered value ...
      */
      if (d->jitter.convex) {
        fworld = (gfloat) (d->world.vals[m][k] - d->jitdata.vals[m][k]);
        fjit = d->vartable[k].jitter_factor * (frand - fworld);
      }
      else
        fjit = d->vartable[k].jitter_factor * frand;

      d->jitdata.vals[m][k] = (glong) fjit;
    }
  }
  tform_to_world (d, gg);
  /*-- do not redisplay the missing values displays --*/
  displays_tailpipe (REDISPLAY_PRESENT, gg);

  g_free ((gpointer) selected_cols);
}


/*
 * This needs a plotted_cols_get
*/
void
jitter_value_set (gfloat value, datad *d, ggobid *gg) {
  gint *cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  gint j;
  gint ncols = selected_cols_get (cols, d->jitter.vgroup, d, gg);

  if (ncols == 0)
    ncols = plotted_cols_get (cols, false, d, gg);

  for (j=0; j<ncols; j++)
    d->vartable[cols[j]].jitter_factor = value;

  g_free ((gpointer) cols);
}

