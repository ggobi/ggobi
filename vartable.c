/* vartable.c */
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
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "vartable.h"

#ifdef __cplusplus
extern "C" {
#endif
extern gfloat no_change (gfloat, gfloat);
#ifdef __cplusplus
}
#endif

void vartable_alloc (datad *d, ggobid *gg)
{
  if (d->vartable != NULL)
    g_free ((gpointer) d->vartable);

  d->vartable = (vartabled *) g_malloc (d->ncols * sizeof (vartabled));
}

void vartable_realloc (gint n, datad *d, ggobid *gg)
{
  d->vartable = (vartabled *) g_realloc ((gpointer) d->vartable,
    n * sizeof (vartabled));
}


void vartable_init (datad *d, ggobid *gg)
{
  gint j;
  for (j=0; j<d->ncols; j++) {
    d->vartable[j].selected = false;
    d->vartable[j].nmissing = 0;

    d->vartable[j].jref = -1;  /*-- not cloned --*/

    d->vartable[j].mean = 0.0;
    d->vartable[j].median = 0.0;

    d->vartable[j].lim_specified_p = false;  /*-- no user-specified limits --*/

    d->vartable[j].lim_raw.min = 0.0;
    d->vartable[j].lim_raw.max = 0.0;
    d->vartable[j].lim_tform.min = 0.0;
    d->vartable[j].lim_tform.max = 0.0;

    d->vartable[j].domain_incr = 0.;
    d->vartable[j].domain_adj = no_change;
    d->vartable[j].inv_domain_adj = no_change;
    d->vartable[j].tform1 = NO_TFORM1;
    d->vartable[j].param = 0.;
	d->vartable[j].tform2 = NO_TFORM2;

    d->vartable[j].jitter_factor = 0.0;

    d->vartable[j].collab = NULL;
    d->vartable[j].collab_tform = NULL;
  }
}

/*-------------------------------------------------------------------------*/
/*                 finding the statistics for the table                    */
/*-------------------------------------------------------------------------*/

void
vartable_stats_print (datad *d, ggobid *gg) 
{
  gint j;
  for (j=0; j<d->ncols; j++) {
    g_printerr ("mean=%f, median=%f\n",
      d->vartable[j].mean, d->vartable[j].median);
    g_printerr ("lims: %7.2f %7.2f %7.2f %7.2f\n",
      d->vartable[j].lim_raw.min, d->vartable[j].lim_raw.max,
      d->vartable[j].lim_tform.min, d->vartable[j].lim_tform.max);
  }
}

/*
 * Sets lim_raw from d->raw, and lim_tform from d->tform.
*/
void
vartable_stats_set (datad *d, ggobid *gg) 
{
  gint j, i, m, np;
  gfloat *sumv = (gfloat *) g_malloc0 (d->ncols * sizeof (gfloat));
  gfloat *x = (gfloat *) g_malloc (d->nrows * sizeof (gfloat));

  /*
   * this could be done with less code, but this 
   * minimizes looping and function calls, and should
   * be as as fast as we can make it
  */

  for (j=0; j<d->ncols; j++) {
    d->vartable[j].lim_raw.min = d->raw.vals[0][j];
    d->vartable[j].lim_raw.max = d->raw.vals[0][j];
    d->vartable[j].lim_tform.min = d->tform.vals[0][j];
    d->vartable[j].lim_tform.max = d->tform.vals[0][j];
  }

  for (j=0; j<d->ncols; j++) {
    np = 0;
    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot[m];
      if (d->nmissing > 0 && MISSING_P(i,j))
        ;
      else {

        if (d->raw.vals[i][j] < d->vartable[j].lim_raw.min)
          d->vartable[j].lim_raw.min = d->raw.vals[i][j];
        else if (d->raw.vals[i][j] > d->vartable[j].lim_raw.max)
          d->vartable[j].lim_raw.max = d->raw.vals[i][j];

        if (d->tform.vals[i][j] < d->vartable[j].lim_tform.min)
          d->vartable[j].lim_tform.min = d->tform.vals[i][j];
        else if (d->tform.vals[i][j] > d->vartable[j].lim_tform.max)
          d->vartable[j].lim_tform.max = d->tform.vals[i][j];

        sumv[j] += d->raw.vals[i][j];
        x[np] = d->raw.vals[i][j];
        np++;
      }
    }
    d->vartable[j].mean = sumv[j] / (gfloat) d->nrows;

    /*-- median: sort the temporary vector, and find its center --*/
    qsort((void *) x, np, sizeof (gfloat), fcompare);
    d->vartable[j].median = 
      ((np % 2) != 0) ?  x[(np-1)/2] : (x[np/2-1] + x[np/2])/2. ;
  }

  g_free ((gpointer) sumv);
  g_free ((gpointer) x);
}

/*-------------------------------------------------------------------------*/
/*                   adding and deleting variables                         */
/*-------------------------------------------------------------------------*/

void
vartable_row_append (gint j, datad *d, ggobid *gg)
{
  if (d->vartable_clist != NULL) {
    gint k;
    gchar **row;
    row = (gchar **) g_malloc (NCOLS_CLIST * sizeof (gchar *));

    if (j == -1) {
      row[CLIST_VARNO] = g_strdup_printf ("%d", 0);
      row[CLIST_VARNAME] = g_strdup ("");
      row[CLIST_TFORM] = g_strdup ("");
      row[CLIST_USER_MIN] = g_strdup_printf ("%8.3f", 0.0);
      row[CLIST_USER_MAX] = g_strdup_printf ("%8.3f", 0.0);
      row[CLIST_DATA_MIN] = g_strdup_printf ("%8.3f", 0.0);
      row[CLIST_DATA_MAX] = g_strdup_printf ("%8.3f", 0.0);
      row[CLIST_MEAN] = g_strdup_printf ("%8.3f", 0.0);
      row[CLIST_MEDIAN] = g_strdup_printf ("%8.3f", 0.0);
      row[CLIST_NMISSING] = g_strdup_printf ("%d", 0);
    } else {
      row[CLIST_VARNO] = g_strdup_printf ("%d", j);
      row[CLIST_VARNAME] = g_strdup (d->vartable[j].collab);
      row[CLIST_TFORM] = g_strdup ("");
      if (d->vartable[j].lim_specified_p) {
        row[CLIST_USER_MIN] = g_strdup_printf ("%8.3f",
          d->vartable[j].lim_specified.min);
        row[CLIST_USER_MAX] = g_strdup_printf ("%8.3f",
          d->vartable[j].lim_specified.max);
      } else {
        row[CLIST_USER_MIN] = g_strdup ("");
        row[CLIST_USER_MAX] = g_strdup ("");
      }
      row[CLIST_DATA_MIN] = g_strdup_printf ("%8.3f",
        d->vartable[j].lim_raw.min);
      row[CLIST_DATA_MAX] = g_strdup_printf ("%8.3f",
        d->vartable[j].lim_raw.max);
      row[CLIST_MEAN] = g_strdup_printf ("%8.3f", d->vartable[j].mean);
      row[CLIST_MEDIAN] = g_strdup_printf ("%8.3f", d->vartable[j].median);
      row[CLIST_NMISSING] = g_strdup_printf ("%d", d->vartable[j].nmissing);
    }

    gtk_clist_append ((GtkCList *) d->vartable_clist, row);

    for (k=0; k<NCOLS_CLIST; k++)
      g_free ((gpointer) row[k]);
    g_free ((gpointer) row);
  }
}

void
variable_clone (gint jvar, const gchar *newName, gboolean update,
  datad *d, ggobid *gg) 
{
  gint nc = d->ncols + 1;
  
/*-- vartable_ui --*/
  /*-- set a view of the data values before adding the new label --*/
  vartable_row_append (d->ncols-1, d, gg);

/*-- vartable --*/
  vartable_realloc (nc, d, gg);
  d->vartable[nc-1].collab =
    g_strdup ((newName && newName[0]) ? newName : d->vartable[jvar].collab);
  d->vartable[nc-1].collab_tform =
    g_strdup ((newName && newName[0]) ? newName : d->vartable[jvar].collab);

/*-- varpanel_ui  --*/
  d->varpanel_ui.checkbox = (GtkWidget **)
    g_realloc (d->varpanel_ui.checkbox, nc * sizeof (GtkWidget *));
  varpanel_checkbox_add (nc-1, d, gg);

/*-- vartable --*/
  /*-- now the rest of the variables --*/
  d->vartable[nc-1].jitter_factor = d->vartable[jvar].jitter_factor;
  d->vartable[nc-1].nmissing = d->vartable[jvar].nmissing;

  if (update) {
    updateAddedColumn (nc, jvar, d, gg);
  }

  gtk_widget_show_all (gg->varpanel_ui.notebook);
}

gboolean
updateAddedColumn (gint nc, gint jvar, datad *d, ggobid *gg)
{
  gint i;

/*-- vartable --*/
  if (jvar > -1) {
    d->vartable[nc-1].mean = d->vartable[jvar].mean;
    d->vartable[nc-1].median = d->vartable[jvar].median;
    d->vartable[nc-1].lim.min =
      d->vartable[nc-1].lim_raw.min = d->vartable[nc-1].lim_tform.min = 
      d->vartable[jvar].lim_raw.min;
    d->vartable[nc-1].lim.max =
      d->vartable[nc-1].lim_raw.max = d->vartable[nc-1].lim_tform.max = 
      d->vartable[jvar].lim_raw.max;
  } 

  transform_values_init (nc-1, d, gg);
/*-- --*/

/*-- pipeline --*/
/*
  pipeline_arrays_add_column (jvar, d, gg);
*/

  arrayf_add_cols (&d->raw, nc);  /*-- adding exactly one column --*/
  arrayf_add_cols (&d->tform, nc);
  for (i=0; i<d->nrows; i++)
    d->raw.vals[i][nc-1] = d->tform.vals[i][nc-1] = d->raw.vals[i][jvar];

  missing_arrays_add_column (jvar, d, gg);

  d->ncols++;
  tform_to_world (d, gg); /*-- need this only for the new variable --*/

  return (true);
}

