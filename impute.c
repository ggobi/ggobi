/* impute_ui.c */

#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


gboolean
impute_fixed (gint impute_type, datad *d, ggobid *gg)
{
  gint i, j, k, m;
  gfloat maxval, minval, range, val, impval;
  gchar *val_str;
  gint *selected_cols;
  gint nselected_cols = 0;
  gboolean ok = true;

  selected_cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  nselected_cols = selected_cols_get (selected_cols, false, d, gg);
  if (nselected_cols == 0)
    nselected_cols = plotted_cols_get (selected_cols, false, d, gg);

  if (impute_type == IMP_ABOVE || impute_type == IMP_BELOW) {

    if (impute_type == IMP_ABOVE)
      val_str = gtk_entry_get_text (GTK_ENTRY (gg->impute.entry_above));
    else if (impute_type == IMP_BELOW)
      val_str = gtk_entry_get_text (GTK_ENTRY (gg->impute.entry_below));

    if (strlen (val_str) == 0) {
      gchar *message = g_strdup_printf (
      "You selected '%% over or under' but didn't specify a percentage.\n");
      quick_message (message, false);
      g_free (message);
      ok = false;
      return ok;
    }

    val = (gfloat) atof (val_str);
    if (val < 0 || val > 100) {
      gchar *message = g_strdup_printf (
        "You specified %f%%; please specify a percentage between 0 and 100.\n",
        val);
      quick_message (message, false);
      g_free (message);
      ok = false;
      return ok;
    }

    for (k=0; k<nselected_cols; k++) {
       j = selected_cols[k];

      /* First find the maximum and minimum values of the non-missing data */
      maxval = d->vartable[j].lim_raw.min;
      maxval = d->vartable[j].lim_raw.max;

      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        if (!d->missing.vals[m][j]) {
          if (d->raw.vals[m][j] > maxval) maxval = d->raw.vals[m][j];
          if (d->raw.vals[m][j] < minval) minval = d->raw.vals[m][j];
        }
      }
      range = maxval - minval;

      /* Then fill it in */
      if (impute_type == IMP_ABOVE)
        impval = maxval + (val/100.) * range;
      else if (impute_type == IMP_BELOW)
        impval = minval - (val/100.) * range;

      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        if (d->missing.vals[m][j]) {
          d->raw.vals[m][j] = d->tform.vals[m][j] = impval;
        }
      }
    }
  }
  else if (impute_type == IMP_FIXED) {

    val_str = gtk_entry_get_text (GTK_ENTRY (gg->impute.entry_val));
    if (strlen (val_str) == 0) {
      quick_message (
        "You've selected 'Specify' but haven't specified a value.\n",
         false);
      ok = false;
      return ok;
    }
    else {
      impval = (gfloat) atof (val_str);
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        for (k=0; k<nselected_cols; k++) {
          j = selected_cols[k];
          if (d->missing.vals[m][j]) {
            d->raw.vals[m][j] = d->tform.vals[m][j] = impval;
          }
        }
      }
    }
  }

/*  update_imputation(gg);*/

  g_free (selected_cols);
  return ok;
}

static void
impute_single (gint *missv, gint nmissing, gint *presv, gint npresent,
  gint col, datad *d, ggobid *gg)
{
  gint i, k;
  gfloat rrand;

  /*
   * Then loop over the missing values, plugging in some value
   * drawn from the present values.
  */
  for (i=0; i<nmissing; i++) {
    for (k=0; k<npresent; k++) {
      rrand = (gfloat) randvalue();

      if ( ((npresent - k) * rrand) < 1.0 ) {
        d->raw.vals[missv[i]][col] = d->raw.vals[presv[k]][col];
        /*
         * This is the default -- transformations will be applied
         * later to those that need it.
        */
        d->tform.vals[missv[i]][col] = d->tform.vals[presv[k]][col];
        break;
      }
    }
  }
}

void
impute_random (datad *d, ggobid *gg)
{
/* Perform single random imputation */

  gint i, j, k, n, m, npresent, *presv, nmissing, *missv;
  gint *selected_cols, nselected_cols;

  presv = (gint *) g_malloc (d->nrows_in_plot * sizeof (gint));
  missv = (gint *) g_malloc (d->nrows_in_plot * sizeof (gint));

  selected_cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  nselected_cols = selected_cols_get (selected_cols, false, d, gg);
  if (nselected_cols == 0)
    nselected_cols = plotted_cols_get (selected_cols, false, d, gg);

  if (gg->impute.vgroup_p && d->nclusters > 1) {

    /* Loop over the number of brushing groups */
    for (n=0; n<d->nclusters; n++) {

      /* Then loop over the number of columns */
      for (m=0; m<nselected_cols; m++) {
        npresent = nmissing = 0;
        j = selected_cols[m];

        /*
         * And finally over the rows, including only those rows
         * which belong to the current cluster
        */
        for (i=0; i<d->nrows_in_plot; i++) {
          k = d->rows_in_plot[i];
          if (d->clusterids.vals[k] == n) { 
            if (!d->hidden_now[k]) {   /* ignore erased values altogether */
              if (d->missing.vals[k][j])
                missv[nmissing++] = k;
              else
                presv[npresent++] = k;
            }
          }
        }
        impute_single (missv, nmissing, presv, npresent, j, d, gg);
      }
    }
  }

  else {
    for (m=0; m<nselected_cols; m++) {
      npresent = nmissing = 0;
      j = selected_cols[m];
      /*
       * Build the vector of indices of present values that can be used
       * to draw from.
      */
      for (i=0; i<d->nrows_in_plot; i++) {
        k = d->rows_in_plot[i];
        if (!d->hidden_now[k]) {   /* ignore erased values altogether */
          if (d->missing.vals[k][j])
            missv[nmissing++] = k;
          else
            presv[npresent++] = k;
        }
      }
      impute_single (missv, nmissing, presv, npresent, j, d, gg);
    }
  }

  /* Handle transformations, run through pipeline, plot */
/*  update_imputation (gg);*/

  g_free (presv);
  g_free (missv);
  g_free (selected_cols);
}
