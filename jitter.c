/* jitter.c */
/*
 * Contains jittering routines for tform2; see missing.c for the
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
rejitter (ggobid *gg) {
  gint *selected_cols;
  gint i, j, k, m, nselected_cols = 0;
  gfloat frand, fworld, fjit;
  gfloat precis = (gfloat) PRECISION1;

/*
 * First determine the variables to be jittered:
 * this depends first on the selected variables and
 * second on vgroups (if jitter_vgroup is True)
*/
  selected_cols = (gint *) g_malloc (gg->ncols * sizeof (gint));
  nselected_cols = selected_cols_get (selected_cols, gg->jitter_vgroup, gg);
  if (nselected_cols == 0)
    nselected_cols = plotted_cols_get (selected_cols, false, gg);

  for (j=0; j<nselected_cols; j++) {
    k = selected_cols[j];

    for (i=0; i<gg->nrows_in_plot; i++) {
      m = gg->rows_in_plot[i];
      /*-- jitter_one_value (m, k); --*/

      frand = jitter_randval (gg->jitter_type) * precis;

      /*
       * The world.data used here is already jittered:
       * subtract out the previous jittered value ...
      */
      if (gg->jitter_convex) {
        fworld = (gfloat) (gg->world.data[m][k] - gg->jitter.data[m][k]);
        fjit = gg->vardata[k].jitter_factor * (frand - fworld);
      }
      else
        fjit = gg->vardata[k].jitter_factor * frand;

      gg->jitter.data[m][k] = (glong) fjit;
    }
  }
  tform_to_world (gg);
  /*-- do not redisplay the missing values displays --*/
  displays_tailpipe (REDISPLAY_PRESENT, gg);

/*
  if (clear_vartable) 
    vartable_unselect_all ();
*/

  g_free ((gpointer) selected_cols);
}


/*
 * This needs a plotted_cols_get
*/
void
jitter_value_set (gfloat value, ggobid *gg) {
  gint *cols = (gint *) g_malloc (gg->ncols * sizeof (gint));
  gint ncols = selected_cols_get (cols, gg->jitter_vgroup, gg);
  gint j;

  for (j=0; j<ncols; j++)
    gg->vardata[cols[j]].jitter_factor = value;

  g_free ((gpointer) cols);
}

