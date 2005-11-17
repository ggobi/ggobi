/* varchange.c: add or delete variables, including cloning */
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

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "vartable.h"


const double AddVarRowNumbers = -1.0;
const double AddVarBrushGroup = -1.0;


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

  tour1d_realloc_up (d->ncols, d, gg);
  tour2d3_realloc_up (d->ncols, d, gg);
  tour2d_realloc_up (d->ncols, d, gg);
  tourcorr_realloc_up (d->ncols, d, gg);

  missing_arrays_add_cols (d, gg);
}

/* XXX this routine just should not exist.  The appropriate elements
   should be responding to the variable_added routine.  But that could
   be messy, too, because they have to respond in a particular order ..
   Oh man ...

  g_signal_connect (G_OBJECT (gg),
                      "variable_added", 
                      G_CALLBACK (variable_notebook_varchange_cb),
                      GTK_OBJECT (notebook));

void 
variable_notebook_varchange_cb (ggobid *gg, vartabled *vt, gint which,
  datad *data, void *notebook)
*/
void
addvar_propagate (gint ncols_prev, gint ncols_added, datad *d, ggobid *gg)
{
  gint j, jvar;

  for (j=0; j<ncols_added; j++) {
    jvar = ncols_prev + j;  /*-- its new index --*/

    /*-- update the tree_view widget (the visible table) --*/
    vartable_row_append (jvar, d, gg);          /*-- append empty --*/

    vartable_cells_set_by_var (jvar, d);  /*-- then populate --*/
  }

  /*-- in case some datad now has variables and it didn't before --*/
  display_menu_build (gg);
}


void
newvar_add_with_values (gdouble *vals, gint nvals, gchar *vname,
 vartyped type,
 /*-- if categorical, we need ... --*/
 gint nlevels, gchar **level_names, gint *level_values, gint *level_counts,
 datad *d, ggobid *gg)
{
  gint i;
  gint d_ncols_prev = d->ncols;
  gint jvar = d_ncols_prev;
  vartabled *vt;

  if (nvals != d->nrows)
    return;

  /* Create a new element in the vartable list iff we need
     to. Otherwise use the one in the current position. */
  if(jvar >= g_slist_length(d->vartable))
    vartable_element_new (d);

  vt = vartable_element_get (jvar, d);
  if (type == categorical)
    vartable_element_categorical_init (vt, nlevels, level_names,
      level_values, level_counts);
  transform_values_init (jvar, d, gg);


  d->ncols += 1;
  addvar_pipeline_realloc (d, gg);

  for (i=0; i<d->nrows; i++) {
     if(vals == &AddVarRowNumbers) {
      d->raw.vals[i][jvar] = d->tform.vals[i][jvar] = (gfloat) (i+1);
    } else if(vals == &AddVarBrushGroup) {
      d->raw.vals[i][jvar] = d->tform.vals[i][jvar] = (gfloat) d->clusterid.els[i];
    } else if(GGobiMissingValue && GGobiMissingValue(vals[i]))
      setMissingValue(i, jvar, d, vt);
    else
      d->raw.vals[i][jvar] = d->tform.vals[i][jvar] = (gfloat) vals[i];
  }

  
  /*-- update the vartable struct --*/
  limits_set_by_var (jvar, true, true, d, gg);
  limits_display_set_by_var (jvar, d, gg);


  /*-- run the data through the head of the pipeline --*/
  tform_to_world_by_var (jvar, d, gg);

  vt->collab = vt->collab_tform = g_strdup (vname);
  vt->nickname = g_strndup (vname, 2);
  /*-- --*/

  addvar_propagate (d_ncols_prev, 1, d, gg);

/* XXX be careful:  this could be emitted before the variable type
       is set.
*/
  /*-- emit variable_added signal --*/
  g_signal_emit (G_OBJECT (gg),
    GGobiSignals[VARIABLE_ADDED_SIGNAL], 0, vt, d->ncols -1, d); 
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
    g_signal_emit (G_OBJECT (gg),
                     GGobiSignals[VARIABLE_ADDED_SIGNAL], 0, vt, n, d); 
  }

  /*
   * I'm sending this expose event because sometimes the tree_view
   * is scrambled after a variable is cloned, with the entire list
   * of variables appearing twice.  -- dfs 1/16/2002
  */
  {
    if (gg->vartable_ui.window) {
      gboolean rval = false;
      g_signal_emit_by_name (G_OBJECT (gg->vartable_ui.window),
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

    if(GGOBI_IS_EXTENDED_DISPLAY(display)) {
      GGobiExtendedDisplayClass *klass;
      klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS(display);
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

	return false;
  }

  keepers = g_malloc ((d->ncols-ncols) * sizeof (gint));
  nkeepers = find_keepers (d->ncols, ncols, cols, keepers);
  if (nkeepers == -1) {
    g_free (keepers);
	return false;
  }

  for (j=0; j<ncols; j++) {
    vartable_element_remove (cols[j], d);
    vt = vartable_element_get (cols[j], d);
  }

  /*-- delete rows from tree model; no copying is called for --*/
  if (d->vartable_tree_view[real] != NULL) {
	  for (j = 0; j < nkeepers; j++) {
		  GtkTreeModel *model;
		  GtkTreeIter iter;
		  GtkTreePath *path = gtk_tree_path_new_from_indices(keepers[j], -1);
		  vt = vartable_element_get(keepers[j], d);
		  model = gtk_tree_view_get_model(GTK_TREE_VIEW(d->vartable_tree_view[vt->vartype]));
		  gtk_tree_model_get_iter(model, &iter, path);
		  gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
		  gtk_tree_path_free(path);
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

  d->ncols -= ncols;

  /*-- emit a single variable_list_changed signal when finished --*/
  /*-- doesn't need to give a variable index any more, really --*/
  vt = vartable_element_get (cols[d->ncols-1], d);
  g_signal_emit (G_OBJECT (gg),
                   GGobiSignals[VARIABLE_LIST_CHANGED_SIGNAL], 0, d); 

  /*-- run the first part of the pipeline  --*/
  tform_to_world (d, gg);

  return true;
}


vartyped
ggobi_data_set_var_type(datad *d, int which, vartyped value)
{
    vartabled *vt;
    vartyped old;

    if(value < real || value >= all_vartypes)
        return(all_vartypes);

    vt = vartable_element_get(which, d);
    if(!vt) {
         return(all_vartypes);
    }

    old = vt->vartype;
    vt->vartype = value;

    return(old);
}


gboolean
ggobi_data_set_time_var(datad *d, int which, gboolean value)
{
    vartabled *vt;
    gboolean old;

    vt = vartable_element_get(which, d);
    if(!vt) {
         return(all_vartypes);
    }

    old = vt->isTime;
    vt->isTime = value;

    return(old);
}

