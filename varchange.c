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

void vartable_free_var (gint j, datad *d);
void vartable_copy_var (gint jfrom, gint jto, datad *d);
gboolean array_contains (gint* arr, gint n, gint el);
void vartable_init_var (gint j, datad *d);
void transform_values_copy (gint jfrom, gint jto, datad *d);

static void
addvar_vartable_expand (gint ncols, datad *d, ggobid *gg)
{
  gint jvar, k;

  vartable_realloc (d->ncols+ncols, d);
  for (k=0; k<ncols; k++) {
    jvar = d->ncols+k; /*-- its new index --*/

    /*-- initialize d->vartable struct with default values --*/
    vartable_init_var (jvar, d);
    transform_values_init (jvar, d, gg);
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

  /*-- variable checkboxes and circles --*/
  varpanel_checkboxes_add (d->ncols, d, gg);
  varcircles_add (d->ncols, d, gg);
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

  addvar_vartable_expand (1, d, gg);  /*-- add one variable --*/

  d->ncols += 1;
  addvar_pipeline_realloc (d, gg);

  if (vtype == ADDVAR_ROWNOS) {
    for (i=0; i<d->nrows; i++)
      d->raw.vals[i][jvar] = d->tform.vals[i][jvar] = (float) (i+1);
  } else if (vtype == ADDVAR_BGROUP) {
    clusters_set (d, gg);
    for (i=0; i<d->nrows; i++)
      d->raw.vals[i][jvar] = d->tform.vals[i][jvar] =
        (float) d->clusterid.els[i];
  }
  
  /*-- update the vartable struct --*/
  limits_set_by_var (jvar, true, true, d, gg);
  d->vartable[jvar].collab = 
    d->vartable[jvar].collab_tform = g_strdup (vname);
  /*-- --*/

  addvar_propagate (d_ncols_prev, 1, d, gg);
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

  if (nvals != d->nrows)
    return;

  addvar_vartable_expand (1, d, gg);  /*-- add one variable --*/

  d->ncols += 1;
  addvar_pipeline_realloc (d, gg);

  for (i=0; i<d->nrows; i++)
    d->raw.vals[i][jvar] = d->tform.vals[i][jvar] = (gfloat) vals[i];
  
  /*-- update the vartable struct --*/
  limits_set_by_var (jvar, true, true, d, gg);
  d->vartable[jvar].collab = 
    d->vartable[jvar].collab_tform = g_strdup (vname);
  /*-- --*/

  addvar_propagate (d_ncols_prev, 1, d, gg);
}

void
clone_vars (gint *cols, gint ncols, datad *d, ggobid *gg)
{
  gint i, k, n, jvar;
  gint d_ncols_prev = d->ncols;

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
}


/*-------------------------------------------------------------------------*/
/*                 eliminate the ncol columns in cols                      */
/*                          not currently used                             */
/*                but note that it's called by sphering!                   */
/*-------------------------------------------------------------------------*/

static gint
plotted (gint *cols, gint ncols, datad *d, ggobid *gg)
{
  gint j, k, projection;
  GList *dlist, *l;
  displayd *display;
  splotd *sp;
  gint jplotted = -1;

  /*-- check each display for each variable --*/
  for (dlist = gg->displays; dlist; dlist = dlist->next) {
    display = (displayd *) dlist->data;
    if (display->d != d)
      continue;

    if (jplotted >= 0)
      break;

    switch (display->displaytype) {
      case scatterplot:
        sp = (splotd *) display->splots->data;  /*-- only one splot --*/
        projection = projection_get (gg);

        switch (projection) {
          case P1PLOT:
            for (j=0; j<ncols; j++) {
              if (sp->p1dvar == cols[j]) {
                jplotted = sp->p1dvar;
                return jplotted;
              }
            }
          break;
          case XYPLOT:
            for (j=0; j<ncols; j++) {
              if (sp->xyvars.x == cols[j]) {
                jplotted = sp->xyvars.x;
                return jplotted;
              }
              if (sp->xyvars.y == cols[j]) {
                jplotted = sp->xyvars.y;
                return jplotted;
              }
            }
          break;
          case TOUR1D:
            for (j=0; j<ncols; j++) {
              for (k=0; k<display->t1d.nvars; k++) {
                if (display->t1d.vars.els[k] == cols[j]) {
                  jplotted = display->t1d.vars.els[k];
                  return jplotted;
                }
              }
            }
          break;
          case TOUR2D:
            for (j=0; j<ncols; j++) {
              for (k=0; k<display->t2d.nvars; k++) {
                if (display->t2d.vars.els[k] == cols[j]) {
                  jplotted = display->t2d.vars.els[k];
                  return jplotted;
                }
              }
            }
          break;
          case COTOUR:
            for (j=0; j<ncols; j++) {
              for (k=0; k<display->tcorr1.nvars; k++) {
                if (display->tcorr1.vars.els[k] == cols[j]) {
                  jplotted = display->tcorr1.vars.els[k];
                  return jplotted;
                }
              }
              for (k=0; k<display->tcorr2.nvars; k++) {
                if (display->tcorr2.vars.els[k] == cols[j]) {
                  jplotted = display->tcorr2.vars.els[k];
                  return jplotted;
                }
              }
            }
          break;
        }
      break;  /*-- end case scatterplot, with all its modes --*/

      case scatmat:
        for (l = display->splots; l; l = l->next) {
          sp = (splotd *) l->data;

          for (j=0; j<ncols; j++) {
            if (sp->p1dvar == -1) {
              if (sp->xyvars.x == cols[j]) {
                jplotted = sp->xyvars.x;
                return jplotted;
              }
              if (sp->xyvars.y == cols[j]) {
                jplotted = sp->xyvars.y;
                return jplotted;
              }
            } else if (sp->p1dvar == cols[j]) {
              jplotted = sp->p1dvar;
              return jplotted;
            }
          }
        }
      break;

      case parcoords:
        for (l = display->splots; l; l = l->next) {
          sp = (splotd *) l->data;

          for (j=0; j<ncols; j++) {
            if (sp->xyvars.x == cols[j]) {
              jplotted = sp->xyvars.x;
              return jplotted;
            }
          }
        }
      break;

      case tsplot:
        for (l = display->splots; l; l = l->next) {
          sp = (splotd *) l->data;

          for (j=0; j<ncols; j++) {
            if (sp->xyvars.x == cols[j]) {
              jplotted = sp->xyvars.x;
              return jplotted;
            }
            if (sp->xyvars.y == cols[j]) {
              jplotted = sp->xyvars.y;
              return jplotted;
            }
          }
        }
      break;
      case unknown_display_type:
      break;
    }
  }

  return jplotted;
}

gboolean
delete_vars (gint *cols, gint ncols, datad *d, ggobid *gg) 
{
  gint j, jfrom, jto;
  gint *keepers, nkeepers;
  GList *l;
  GtkCListRow *row;
  gchar *varstr;
  gint irow;

  /*-- don't allow all variables to be deleted --*/
  if (ncols >= d->ncols)
/**/return false;

  /*
   * If one of the variables to be deleted is currently plotted,
   * we won't proceed until the user cleans up.
  */
  if ((j = plotted (cols, ncols, d, gg)) != -1) {
    gchar *message;
    message = g_strdup_printf ("Deletion failed; the variable '%s' is currently plotted\n",
      d->vartable[j].collab);
    quick_message (message, false);
    g_free (message);

/**/return false;
  }

  keepers = g_malloc ((d->ncols-ncols) * sizeof (gint));
  nkeepers = find_keepers (d->ncols, ncols, cols, keepers);

  /*-- copy and reallocate the array of vartabled structures --*/
  /*-- delete elements from d->vartable array --*/
  for (j=0; j<nkeepers; j++) {
    jto = j;
    jfrom = keepers[j];
    if (jto != jfrom) {
      vartable_free_var (jto, d);  /*-- free collab and collab_tform --*/
      vartable_copy_var (jfrom, jto, d);
    }
  }

  /*-- having copied the keepers, free the last ncols elements --*/
  for (j=nkeepers; j<d->ncols; j++)
    vartable_free_var (j, d);
  vartable_realloc (nkeepers, d);

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

  /*-- run the first part of the pipeline  --*/
  tform_to_world (d, gg);

  return true;
}

