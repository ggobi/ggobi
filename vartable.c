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

static gboolean vartable_update_cloned_var (gint, gint, datad *, ggobid *);

/*-------------------------------------------------------------------------*/
/*                         utilities                                       */
/*-------------------------------------------------------------------------*/

gboolean
array_contains (gint* arr, gint n, gint el)
{
  gint j;

  for (j=0; j<n; j++)
    if (arr[j] == el)
      return true;
  
  return false;
}

gint
selected_cols_get (gint *cols, datad *d, ggobid *gg)
{
/*
 * Figure out which columns are selected.
*/
  gint j, ncols = 0;

  for (j=0; j<d->ncols; j++) 
    if (d->vartable[j].selected)
      cols[ncols++] = j;

  return (ncols);
}

/*
 * When there aren't any columns in the variable statistics table,
 * this is how we find out which columns are selected for plotting.
*/
gint
plotted_cols_get (gint *cols, datad *d, ggobid *gg) 
{
  gint mode = mode_get (gg);
  gint ncols = 0;
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;
  gint k;

  switch (display->displaytype) {
    case scatterplot:
      switch (mode) {
        case P1PLOT:
          cols[ncols++] = sp->p1dvar;
        break;
        case XYPLOT:
          cols[ncols++] = sp->xyvars.x;
          cols[ncols++] = sp->xyvars.y;
        break;
        case TOUR1D:
          for (k=0; k<display->ntour_vars; k++)
            cols[ncols++] = display->tour_vars.vals[k];
        break;
         case TOUR2D:
          for (k=0; k<display->ntour_vars; k++)
            cols[ncols++] = display->tour_vars.vals[k];
        break;
      }
    break;
    case scatmat:
    {
      GList *l;
      splotd *s;
      for (l=display->splots; l; l=l->next) {
        s = (splotd *) l->data;
        if (s->p1dvar == -1) {
          if (!array_contains (cols, ncols, sp->xyvars.x))
            cols[ncols++] = sp->xyvars.x;
          if (!array_contains (cols, ncols, sp->xyvars.y))
            cols[ncols++] = sp->xyvars.y;
        } else {
          if (!array_contains (cols, ncols, sp->p1dvar))
            cols[ncols++] = sp->p1dvar;
        }
      }
    }
    break;
    case parcoords:
    {
      GList *l;
      splotd *s;
      for (l=display->splots; l; l=l->next) {
        s = (splotd *) l->data;
        if (!array_contains (cols, ncols, sp->p1dvar))
          cols[ncols++] = sp->p1dvar;
      }
    }
    break;
  }

  return ncols;
}

/*-------------------------------------------------------------------------*/
/*                         memory management                               */
/*-------------------------------------------------------------------------*/

static void
vartable_free_var (gint j, datad *d)
{
  if (d->vartable[j].collab != NULL)
    g_free (d->vartable[j].collab);
  if (d->vartable[j].collab_tform != NULL)
    g_free (d->vartable[j].collab_tform);
}

static void
vartable_free (datad *d)
{
  gint j;
  for (j=0; j<d->ncols; j++) {
    vartable_free_var (j, d);
  }
  g_free ((gpointer) d->vartable);
}

void
vartable_alloc (datad *d)
{
  if (d->vartable != NULL)
    vartable_free (d);

  d->vartable = (vartabled *) g_malloc (d->ncols * sizeof (vartabled));
}

void
vartable_realloc (gint n, datad *d)
{
  d->vartable = (vartabled *) g_realloc ((gpointer) d->vartable,
    n * sizeof (vartabled));
}

void
vartable_copy_var (gint jfrom, gint jto, datad *d)
{
  void transform_values_copy (gint jfrom, gint jto, datad *d);

  d->vartable[jto].jref = d->vartable[jfrom].jref;

  d->vartable[jto].collab = g_strdup (d->vartable[jfrom].collab);
  d->vartable[jto].collab_tform = g_strdup (d->vartable[jfrom].collab_tform);

  d->vartable[jto].mean = d->vartable[jfrom].mean;
  d->vartable[jto].median = d->vartable[jfrom].median;
  d->vartable[jto].lim.min =
    d->vartable[jto].lim_raw.min =
    d->vartable[jto].lim_tform.min = d->vartable[jfrom].lim_tform.min;
  d->vartable[jto].lim.max =
    d->vartable[jto].lim_raw.max =
    d->vartable[jto].lim_tform.max = d->vartable[jfrom].lim_tform.max;

  transform_values_copy (jfrom, jto, d);

  vartable_free_var (jfrom, d);
}


/*-- eliminate the ncol columns in cols --*/
void
delete_vars (gint *cols, gint ncols, datad *d, ggobid *gg) 
{
  gint j, jfrom, jto;
  gint *keepers, nkeepers;
  gint n;
  gint *vars;
  GList *dlist;
  displayd *display;
  splotd *sp;

  keepers = g_malloc ((d->ncols-ncols) * sizeof (gint));
  nkeepers = find_keepers (d->ncols, ncols, cols, keepers);

  /*-- copy and reallocate the array of vartabled structures --*/
  /*-- delete elements from d->vartable array --*/
  for (j=0; j<nkeepers; j++) {
    jto = j;
    jfrom = keepers[j];
    vartable_free_var (jto, d);  /*-- free collab and collab_tform --*/
    vartable_copy_var (jto, jfrom, d);
  }
  for (j=nkeepers; j<d->ncols; j++)
    vartable_free_var (j, d);
  vartable_realloc (nkeepers, d);

  /*-- delete rows from clist; no copying is called for --*/
  {
    GList *l = g_list_last (GTK_CLIST (d->vartable_clist)->selection);
    gint irow;
    while (l) {
      irow = GPOINTER_TO_INT (l->data);
      if (!array_contains (keepers, nkeepers, irow))
        gtk_clist_remove (GTK_CLIST (d->vartable_clist), irow);
      l = l->prev;
    }
  }

  /*-- delete columns from pipeline arrays --*/
  arrayf_delete_cols (&d->raw, ncols, cols);
  arrayf_delete_cols (&d->tform, ncols, cols);
  if (d->nmissing)
    arrays_delete_cols (&d->missing, ncols, cols);

  arrayl_delete_cols (&d->jitdata, ncols, cols);

  /*-- reallocate the rest of the arrays --*/
  arrayl_alloc (&d->world, d->nrows, nkeepers);

  /*-- delete checkboxes --*/
  for (j=ncols-1; j>=0; j--) {
    checkbox_delete_nth (cols[j], d);
  }

  /*-- delete variable circles --*/
  for (j=ncols-1; j>=0; j--) {
    varcircles_delete_nth (cols[j], d);
  }
  varcircles_layout_reset (d->ncols-ncols, d, gg);

  d->ncols -= ncols;

  /*-- run the pipeline  --*/
  tform_to_world (d, gg);

  /*-- clean up the plotted variables for each display type and mode --*/
  for (dlist = gg->displays; dlist; dlist = dlist->next) {
    display = (displayd *) dlist->data;

    switch (display->displaytype) {
      case scatterplot:
        sp = (splotd *) display->splots->data;
        /*-- make sure p1dvar is reasonable --*/
        if (sp->p1dvar >= d->ncols-1)
          sp->p1dvar = 0;

        /*-- make sure xyvars are reasonable --*/
        if (sp->xyvars.x >= d->ncols-1)
          sp->xyvars.x = 0;
        if (sp->xyvars.y >= d->ncols-1 || sp->xyvars.y == sp->xyvars.x)
          sp->xyvars.y = 1;

        /*-- make sure tour_vars are reasonable --*/
        n = 0;
        vars = (gint *)
          g_malloc (MIN (display->ntour_vars, d->ncols) * sizeof (gint));
        for (j=0; j<display->ntour_vars; j++)
          if (display->tour_vars.vals[j] < d->ncols-1)
            vars[n++] = display->tour_vars.vals[j];

        for (j=0; j<n; j++)
          display->tour_vars.vals[j] = vars[j];
        display->ntour_vars = n;

        g_free (vars);
      break;

/*-- delete or replace variables in these two modes --*/
      case scatmat:
      break;

      case parcoords:
      break;
    }
  }

  displays_tailpipe (REDISPLAY_ALL, gg);
}

/*-------------------------------------------------------------------------*/

void vartable_init (datad *d)
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

/*-------------------------------------------------------------------------*/
/*                   adding and deleting variables                         */
/*-------------------------------------------------------------------------*/

void
vartable_row_assemble (gint j, gchar **row, datad *d, ggobid *gg)
{
  /*-- the new row will be appended --*/
  gint nrows = GTK_CLIST (d->vartable_clist)->rows;

  if (j == -1) {
    row[CLIST_VARNO] = g_strdup_printf ("%d", nrows);
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
    row[CLIST_VARNO] = g_strdup_printf ("%d", nrows);
    row[CLIST_VARNAME] = g_strdup (d->vartable[j].collab);
    row[CLIST_TFORM] = g_strdup ("");
    if (d->vartable[j].lim_specified_p) {
      row[CLIST_USER_MIN] = g_strdup_printf ("%8.3f",
        d->vartable[j].lim_specified_tform.min);
      row[CLIST_USER_MAX] = g_strdup_printf ("%8.3f",
        d->vartable[j].lim_specified_tform.max);
    } else {
      row[CLIST_USER_MIN] = g_strdup ("");
      row[CLIST_USER_MAX] = g_strdup ("");
    }
    row[CLIST_DATA_MIN] = g_strdup_printf ("%8.3f",
      d->vartable[j].lim_tform.min);
    row[CLIST_DATA_MAX] = g_strdup_printf ("%8.3f",
      d->vartable[j].lim_tform.max);
    row[CLIST_MEAN] = g_strdup_printf ("%8.3f", d->vartable[j].mean);
    row[CLIST_MEDIAN] = g_strdup_printf ("%8.3f", d->vartable[j].median);
    row[CLIST_NMISSING] = g_strdup_printf ("%d", d->vartable[j].nmissing);
  }
}

void
vartable_row_append (gint j, datad *d, ggobid *gg)
{
  if (d->vartable_clist != NULL) {
    gint k;
    gchar **row = (gchar **) g_malloc (NCOLS_CLIST * sizeof (gchar *));

    vartable_row_assemble (j, row, d, gg);
    gtk_clist_freeze (GTK_CLIST (d->vartable_clist));
    gtk_clist_append ((GtkCList *) d->vartable_clist, row);
    gtk_clist_thaw (GTK_CLIST (d->vartable_clist));

    for (k=0; k<NCOLS_CLIST; k++)
      g_free ((gpointer) row[k]);
    g_free ((gpointer) row);
  }
}

/*
 * n: the index of the variable being cloned
 * jvar: the index of the new variable
*/
static gboolean
vartable_update_cloned_var (gint n, gint jvar, datad *d, ggobid *gg)
{
  if (n >= 0 && jvar > n) {
    d->vartable[jvar].jref = n;
    d->vartable[jvar].collab = g_strdup (d->vartable[n].collab_tform);
    d->vartable[jvar].collab_tform = g_strdup (d->vartable[n].collab_tform);
    d->vartable[jvar].mean = d->vartable[n].mean;
    d->vartable[jvar].median = d->vartable[n].median;
    d->vartable[jvar].lim.min =
      d->vartable[jvar].lim_raw.min =
      d->vartable[jvar].lim_tform.min = d->vartable[n].lim_tform.min;
    d->vartable[jvar].lim.max =
      d->vartable[jvar].lim_raw.max =
      d->vartable[jvar].lim_tform.max = d->vartable[n].lim_tform.max;

    d->vartable[jvar].nmissing = d->vartable[n].nmissing;

    return (true);

  } else return (false); 
}

void
clone_vars (gint *cols, gint ncols, datad *d, ggobid *gg)
{
  gint i, k, n, jvar;
  gint d_ncols_prev = d->ncols;

  vartable_realloc (d->ncols+ncols, d);
  for (k=0; k<ncols; k++) {
    n = cols[k];       /*-- variable being cloned --*/
    jvar = d->ncols+k; /*-- its new index --*/

    /*-- fill in the values in d->vartable --*/
    vartable_update_cloned_var (n, jvar, d, gg);  /*-- from, to --*/

    transform_values_init (jvar, d, gg);
  }

/*
 * Be extremely careful in this latter portion: make sure
 * that d->ncols is being incremented in the right place.
 * A problem in that sequence just made me chase mysterious
 * bugs for two days.
*/

  d->ncols += ncols;

  /*-- pipeline --*/
  arrayf_add_cols (&d->raw, d->ncols);
  arrayf_add_cols (&d->tform, d->ncols);

  for (k=0; k<ncols; k++) {
    n = cols[k];              /*-- variable being cloned --*/
    jvar = d_ncols_prev + k;  /*-- its new index --*/
    for (i=0; i<d->nrows; i++)
      d->raw.vals[i][jvar] = d->tform.vals[i][jvar] = d->tform.vals[i][n];

    tform_to_world_by_var (jvar, d, gg);

    /*-- update the vartable display --*/
    vartable_row_append (jvar, d, gg);
  }
  /*-- --*/
  missing_arrays_add_cols (jvar, d, gg);

  /*-- variable checkboxes --*/
  for (k=0; k<ncols; k++) {
    jvar = d_ncols_prev + k;  /*-- its new index --*/
    varpanel_checkbox_add (jvar, d, gg);
  }

  /*-- variable circles --*/
  varcircles_add (d->ncols, d, gg);
}
