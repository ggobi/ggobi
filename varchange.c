/* varchange.c: add or delete variables, including cloning */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "vartable.h"

void vartable_copy_var (gint jfrom, gint jto, datad *d);
gboolean array_contains (gint* arr, gint n, gint el);
void vartable_element_new (datad *d);
void transform_values_copy (gint jfrom, gint jto, datad *d);

static void
addvar_vartable_expand (gint ncols, datad *d, ggobid *gg)
{
  gint j;

  for (j=d->ncols; j<d->ncols+ncols; j++) {
    /*-- allocate the new vartable element, initialize with default values --*/
    vartable_element_new (d);
    transform_values_init (j, d, gg);
  }
}

/*-- specific adding variables --*/
static void
addvar_pipeline_realloc (datad *d, ggobid *gg)
{
  /*-- realloc pipeline arrays --*/
  arrayf_add_cols (&d->raw, d->ncols);
  arrayf_add_cols (&d->tform, d->ncols);

  tour2d_realloc_up (d->ncols, d, gg);
  tour1d_realloc_up (d->ncols, d, gg);
  tourcorr_realloc_up (d->ncols, d, gg);

  missing_arrays_add_cols (d, gg);
}

static void
addvar_propagate (gint ncols_prev, gint ncols, datad *d, ggobid *gg)
{
  gint k, jvar;

  for (k=0; k<ncols; k++) {
    jvar = ncols_prev + k;  /*-- its new index --*/

    /*-- update the clist widget (the visible table) --*/
    vartable_row_append (d, gg);          /*-- append empty --*/
    vartable_cells_set_by_var (jvar, d);  /*-- then populate --*/

    /*-- run the data through the head of the pipeline --*/
    tform_to_world_by_var (jvar, d, gg);
  }

  /*-- variable toggle buttons and circles --*/
  varpanel_widgets_add (d->ncols, d, gg);
  varcircles_add (d->ncols, d, gg);

  /*-- make sure the right toggle widgets and circles are showing --*/
  varpanel_refresh (gg->current_display, gg);
  varcircles_visibility_set (gg->current_display, gg);
}


/*
 * Under construction
 * Routine used to add a new variable: defined by brushing
 * groups, for example, or perhaps just the row numbers.
*/
void
newvar_add (gint vtype, gchar *vname, datad *d, ggobid *gg)
{
  gint i;
  gint d_ncols_prev = d->ncols;
  gint jvar = d_ncols_prev;
  vartabled *vt;

  addvar_vartable_expand (1, d, gg);  /*-- add one variable --*/

  d->ncols += 1;
  addvar_pipeline_realloc (d, gg);

  if (vtype == ADDVAR_ROWNOS) {
    for (i=0; i<d->nrows; i++)
      d->raw.vals[i][jvar] = d->tform.vals[i][jvar] = (gfloat) (i+1);
  } else if (vtype == ADDVAR_BGROUP) {
    clusters_set (d, gg);
    for (i=0; i<d->nrows; i++)
      d->raw.vals[i][jvar] = d->tform.vals[i][jvar] =
        (gfloat) d->clusterid.els[i];
  }
  
  vt = vartable_element_get (jvar, d);
  /*-- update the vartable struct --*/
  limits_set_by_var (jvar, true, true, d, gg);
  vt->collab = vt->collab_tform = g_strdup (vname);
  /*-- --*/

  addvar_propagate (d_ncols_prev, 1, d, gg);

  /*-- emit variable_added signal --*/
  gtk_signal_emit (GTK_OBJECT (gg),
                   GGobiSignals[VARIABLE_ADDED_SIGNAL], vt, d->ncols-1, gg); 
}

/*
 * The first argument is gdouble because this is only used from
 * the API; for now it's only called from R.  Possibly
 * it should live in ggobi-API.c
*/
void
newvar_add_with_values (gdouble *vals, gint nvals, gchar *vname,
  datad *d, ggobid *gg)
{
  gint i;
  gint d_ncols_prev = d->ncols;
  gint jvar = d_ncols_prev;
  vartabled *vt;

  if (nvals != d->nrows)
    return;

  addvar_vartable_expand (1, d, gg);  /*-- add one variable --*/

  d->ncols += 1;
  addvar_pipeline_realloc (d, gg);

  for (i=0; i<d->nrows; i++)
    d->raw.vals[i][jvar] = d->tform.vals[i][jvar] = (gfloat) vals[i];
  
  /*-- update the vartable struct --*/
  limits_set_by_var (jvar, true, true, d, gg);
  vt = vartable_element_get (jvar, d);
  vt->collab = vt->collab_tform = g_strdup (vname);
  /*-- --*/

  addvar_propagate (d_ncols_prev, 1, d, gg);

  /*-- emit variable_added signal --*/
  gtk_signal_emit (GTK_OBJECT (gg),
                   GGobiSignals[VARIABLE_ADDED_SIGNAL], vt, d->ncols -1, gg); 
}

void
clone_vars (gint *cols, gint ncols, datad *d, ggobid *gg)
{
  gint i, k, n, jvar;
  gint d_ncols_prev = d->ncols;
  vartabled *vt;

  addvar_vartable_expand (ncols, d, gg);

/*
 * Be extremely careful here: make sure that d->ncols is
 * incremented in the right place.  A problem in this sequence
 * just made me chase mysterious bugs for two days.
*/

  d->ncols += ncols;
  addvar_pipeline_realloc (d, gg);


  for (k=0; k<ncols; k++) {
    n = cols[k];              /*-- variable being cloned --*/
    jvar = d_ncols_prev + k;  /*-- its new index --*/

    /*-- copy the data --*/
    for (i=0; i<d->nrows; i++)
      d->raw.vals[i][jvar] = d->tform.vals[i][jvar] = d->tform.vals[i][n];

    /*-- update the vartable struct --*/
    vartable_copy_var (n, jvar, d);
    transform_values_copy (n, jvar, d);
  }

  addvar_propagate (d_ncols_prev, ncols, d, gg);

  for (k=0; k<ncols; k++) {
    n = cols[k];
    vt = vartable_element_get (n, d);

    /*-- emit variable_added signal. Is n the correct index? --*/
    gtk_signal_emit (GTK_OBJECT (gg),
                     GGobiSignals[VARIABLE_ADDED_SIGNAL], vt, n, gg); 
  }

  /*
   * I'm sending this expose event because sometimes the clist
   * is scrambled after a variable is cloned, with the entire list
   * of variables appearing twice.  -- dfs 1/16/2002
  */
  {
    if (gg->vartable_ui.window) {
      gboolean rval = false;
      gtk_signal_emit_by_name (GTK_OBJECT (gg->vartable_ui.window),
        "expose_event",
        (gpointer) gg, (gpointer) &rval);
    }
  }
}


/*-------------------------------------------------------------------------*/
/*                 eliminate the ncol columns in cols                      */
/*                          not currently used                             */
/*                but note that it's called by sphering!                   */
/*-------------------------------------------------------------------------*/

static gint
plotted (gint *cols, gint ncols, datad *d, ggobid *gg)
{
  GList *dlist;
  displayd *display;
  gint jplotted = -1;

  /*-- check each display for each variable --*/
  for (dlist = gg->displays; dlist; dlist = dlist->next) {
    display = (displayd *) dlist->data;
    if (display->d != d)
      continue;

    if (jplotted >= 0)
      break;

    if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
      GtkGGobiExtendedDisplayClass *klass;
      klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
      jplotted = klass->variable_plotted_p(display, cols, ncols, d);
    }
  }

  return jplotted;
}

gboolean
delete_vars (gint *cols, gint ncols, datad *d, ggobid *gg) 
{
  gint j;
  gint *keepers, nkeepers;
  GList *l;
  GtkCListRow *row;
  gchar *varstr;
  gint irow;
  vartabled *vt;

  /*-- don't allow all variables to be deleted --*/
  if (ncols >= d->ncols)
/**/return false;

  /*
   * If one of the variables to be deleted is currently plotted,
   * we won't proceed until the user cleans up.
  */
  if ((j = plotted (cols, ncols, d, gg)) != -1) {
    gchar *message;
    vt = vartable_element_get (j, d);
    message = g_strdup_printf ("Deletion failed; the variable '%s' is currently plotted\n",
      vt->collab);
    quick_message (message, false);
    g_free (message);

/**/return false;
  }

  keepers = g_malloc ((d->ncols-ncols) * sizeof (gint));
  nkeepers = find_keepers (d->ncols, ncols, cols, keepers);
  if (nkeepers == -1) {
    g_free (keepers);
/**/return false;
  }

  for (j=0; j<ncols; j++) {
    vartable_element_remove (cols[j], d);
    vt = vartable_element_get (cols[j], d);
  }

  /*-- delete rows from clist; no copying is called for --*/
  if (d->vartable_clist != NULL) {
    l = g_list_last (GTK_CLIST (d->vartable_clist)->row_list);
    while (l) {
      row = GTK_CLIST_ROW (l);
      /*-- grab the text in the invisible cell of the row to get the index --*/
      varstr = GTK_CELL_TEXT(row->cell[CLIST_VARNO])->text;
      if (varstr != NULL && strlen (varstr) > 0) {
        irow = atoi (varstr);
        if (!array_contains (keepers, nkeepers, irow)) {
          gtk_clist_freeze (GTK_CLIST (d->vartable_clist));
          gtk_clist_remove (GTK_CLIST (d->vartable_clist), irow);
          gtk_clist_thaw (GTK_CLIST (d->vartable_clist));
        }
      }
      l = l->prev;
    }
  }

  /*-- delete columns from pipeline arrays --*/
  arrayf_delete_cols (&d->raw, ncols, cols);
  arrayf_delete_cols (&d->tform, ncols, cols);
  tour2d_realloc_down (ncols, cols, d, gg);
  tour1d_realloc_down (ncols, cols, d, gg);
  tourcorr_realloc_down (ncols, cols, d, gg);
  if (d->nmissing)
    arrays_delete_cols (&d->missing, ncols, cols);

  arrayg_delete_cols (&d->jitdata, ncols, cols);

  /*-- reallocate the rest of the arrays --*/
  arrayg_alloc (&d->world, d->nrows, nkeepers);

  /*-- delete checkboxes --*/
  for (j=ncols-1; j>=0; j--) {
    /*checkbox_delete_nth (cols[j], d);*/
    varpanel_delete_nth (cols[j], d);
  }

  /*-- delete variable circles --*/
  for (j=ncols-1; j>=0; j--) {
    varcircles_delete_nth (cols[j], d);
  }
#ifdef CIRCLES_IN_TABLE
  varcircles_layout_reset (d->ncols-ncols, d, gg);
#endif

  d->ncols -= ncols;

  /*-- emit a single variable_list_changed signal when finished --*/
  /*-- doesn't need to give a variable index any more, really --*/
  vt = vartable_element_get (cols[d->ncols-1], d);
  gtk_signal_emit (GTK_OBJECT (gg),
                   GGobiSignals[VARIABLE_LIST_CHANGED_SIGNAL], gg); 

  /*-- run the first part of the pipeline  --*/
  tform_to_world (d, gg);

  return true;
}

