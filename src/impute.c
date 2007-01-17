/* impute.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


gboolean
impute_fixed (ImputeType impute_type, gfloat val, gint nvars, gint * vars,
              GGobiStage * d)
{
  gint i, j, k;
  gfloat maxval, minval, range, impval = 0;
  gboolean ok = true;
  GGobiVariable *var;


  if (impute_type == IMP_ABOVE || impute_type == IMP_BELOW) {
    gdouble drand;

    for (k = 0; k < nvars; k++) {
      gdouble jmult;
      j = vars[k];
      var = ggobi_stage_get_variable(d, j);

      /* Use find the limits of the non-missing data */
      minval = ggobi_variable_get_display_min(var);
      maxval = ggobi_variable_get_display_max(var);
      range = maxval - minval;

      /* Then fill it in */
      if (impute_type == IMP_ABOVE) {
        impval = maxval + (val / 100.) * range;
        jmult = (impval - maxval) * .2; /* using 20% of the space */
      }
      else {
        impval = minval - (val / 100.) * range;
        jmult = (minval - impval) * .2;
      }

      for (i = 0; i < d->n_rows; i++) {
        if (ggobi_stage_is_missing(d, i, j)) {
          drand = randvalue ();
          drand = (drand - .5) * jmult;
          ggobi_stage_set_raw_value(d, i, j, impval + (gfloat) drand);
        }
        //g_signal_emit_by_name(d, "col_data_changed", j);
      }
    }
  }
  else if (impute_type == IMP_FIXED) {
    for (i = 0; i < d->n_rows; i++) {
      for (k = 0; k < nvars; k++) {
        j = vars[k];
        if (ggobi_stage_is_missing(d, i, j)) {
          ggobi_stage_set_raw_value(d, i, j, val);
        }
        //g_signal_emit_by_name(d, "col_data_changed", j);
      }
    }
  }

  return ok;
}

gboolean
impute_mean_or_median (gint type, gint nvars, gint * vars,
                       GGobiStage * d)
{
  gint i, j, m, n;
  gint np, nmissing;
  greal sum, val;
  greal *x;
  gint *missv;
  GGobiVariable *var;
  gboolean redraw = false;

  if (!ggobi_stage_has_missings(d))
    return false;

  g_return_val_if_fail(GGOBI_IS_SESSION(d->gg), false);
  GGOBI_STAGE_ATTR_INIT_ALL(d);  

  /* If responding to brushing group ... */
  if (d->gg->impute.bgroup_p && d->nclusters > 1) {

    missv = (gint *) g_malloc (d->n_rows * sizeof (gint));
    x = (greal *) g_malloc (d->n_rows * sizeof (greal));

    /* Loop over the number of brushing groups */
    for (n = 0; n < d->nclusters; n++) {

      /* Then loop over the number of columns */
      for (m = 0; m < nvars; m++) {
        np = nmissing = 0;
        j = vars[m];
        sum = 0;

        /*
         * And finally over the rows, including only those rows
         * which belong to the current cluster
         */
        for (i = 0; i < d->n_rows; i++) {
          if (GGOBI_STAGE_GET_ATTR_CLUSTER(d, i) == n) {
            if (!GGOBI_STAGE_GET_ATTR_HIDDEN(d, i)) {  /* ignore erased values */
              if (ggobi_stage_is_missing(d, i, j))
                missv[nmissing++] = i;
              else {
                sum += ggobi_stage_get_raw_value(d, i, j); /* for mean */
                x[np++] = ggobi_stage_get_raw_value(d, i, j);  /* for median */
              }
            }
          }
        }
        if (np && nmissing) {
          if (d->gg->impute.type == IMP_MEAN) {
            val = sum / (greal) np;
          }
          else { // if (gg->impute.type == IMP_MEDIAN) {
            qsort ((void *) x, np, sizeof (gfloat), fcompare);
            val =
              ((np % 2) !=
               0) ? x[(np - 1) / 2] : (x[np / 2 - 1] + x[np / 2]) / 2.;
          }

          for (i = 0; i < nmissing; i++)
            ggobi_stage_set_raw_value(d, missv[i], j, val);
        }
        ggobi_stage_update_col(d, j);
      }
    }
    g_free (missv);
    g_free (x);
    redraw = true;
  }
  else {

    for (m = 0; m < nvars; m++) {
      j = vars[m];
      var = ggobi_stage_get_variable(d, j);
      for (i = 0; i < d->n_rows; i++) {
        if (!GGOBI_STAGE_GET_ATTR_HIDDEN(d, i)) {  /* ignore erased values altogether */
          if (ggobi_stage_is_missing(d, i, j)) {
            ggobi_stage_set_raw_value(d, i, j, (type == IMP_MEAN) ? 
              ggobi_variable_get_mean(var) : ggobi_variable_get_median(var));
            redraw = true;
          }
        }
        ggobi_stage_update_col(d, j);
      }
    }
  }
  ggobi_stage_flush_changes(d);
  return redraw;
}

static void
impute_single (gint * missv, gint nmissing, gint * presv, gint npresent,
               gint col, GGobiStage * d)
{
  gint i, k;
  gfloat rrand;

  /*
   * Then loop over the missing values, plugging in some value
   * drawn from the present values.
   */
  for (i = 0; i < nmissing; i++) {
    for (k = 0; k < npresent; k++) {
      rrand = (gfloat) randvalue ();

      if (((npresent - k) * rrand) < 1.0) {
        ggobi_stage_set_raw_value(d, missv[i], col, ggobi_stage_get_raw_value(d, presv[k], col));
        break;
      }
    }
  }
}

void
impute_random (GGobiStage * d, gint nvars, gint * vars)
{
/* Perform single random imputation */

  gint i, j, n, m, npresent, *presv, nmissing, *missv;

  if (!ggobi_stage_has_missings(d))
    return;

  g_return_if_fail(GGOBI_IS_SESSION(d->gg));

  presv = (gint *) g_malloc (d->n_rows * sizeof (gint));
  missv = (gint *) g_malloc (d->n_rows * sizeof (gint));

  GGOBI_STAGE_ATTR_INIT_ALL(d);  

  if (d->gg->impute.bgroup_p && d->nclusters > 1) {

    /* Loop over the number of brushing groups */
    for (n = 0; n < d->nclusters; n++) {

      /* Then loop over the number of columns */
      for (m = 0; m < nvars; m++) {
        npresent = nmissing = 0;
        j = vars[m];

        /*
         * And finally over the rows, including only those rows
         * which belong to the current cluster
         */
        for (i = 0; i < d->n_rows; i++) {
          if (GGOBI_STAGE_GET_ATTR_CLUSTER(d, i) == n) {
            if (!GGOBI_STAGE_GET_ATTR_HIDDEN(d, i)) {  /* ignore erased values altogether */
              if (ggobi_stage_is_missing(d, i, j))
                missv[nmissing++] = i;
              else
                presv[npresent++] = i;
            }                                 
          }
        }
        impute_single (missv, nmissing, presv, npresent, j, d);
        ggobi_stage_update_col(d, j);
      }
    }
  }

  else {
    for (m = 0; m < nvars; m++) {
      npresent = nmissing = 0;
      j = vars[m];
      /*
       * Build the vector of indices of present values that can be used
       * to draw from.
       */
      for (i = 0; i < d->n_rows; i++) {
        if (!GGOBI_STAGE_GET_ATTR_HIDDEN(d, i)) {  /* ignore erased values altogether */
          if (ggobi_stage_is_missing(d, i, j))
            missv[nmissing++] = i;
          else
            presv[npresent++] = i;
        }
      }
      impute_single (missv, nmissing, presv, npresent, j, d);
      ggobi_stage_update_col(d, j);
    }
  }

  ggobi_stage_flush_changes(d);
    
  g_free (presv);
  g_free (missv);
}
