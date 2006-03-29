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


// displays should eventually listen to pipeline themselves
void
tour_realloc_up (GGobiData *d, gint nc)
{
  displayd *dsp;
  GList *l;

  if (!GGOBI_IS_GGOBI(d->gg))
    return;

  for (l=d->gg->displays; l; l=l->next) {
    GGobiExtendedDisplayClass *klass;
    dsp = (displayd *) l->data;

    if(!GGOBI_IS_EXTENDED_DISPLAY(dsp))
      continue;
    klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS(dsp);
    if(klass->tourcorr_realloc)
        klass->tourcorr_realloc(dsp, nc, d);
    if(klass->tour2d3_realloc)
        klass->tour2d3_realloc(dsp, nc, d);
    if(klass->tour2d_realloc)
        klass->tour2d_realloc(dsp, nc, d);
    if(klass->tour1d_realloc)
        klass->tour1d_realloc(dsp, nc, d);
  }
}


guint
create_explicit_variable (GGobiData * d, gchar * vname, NewVariableType vtype)
{
  guint jvar = ggobi_data_add_cols(d, 1);
  ggobi_data_set_col_name(d, jvar, vname);

  for (guint i = 0; i < d->nrows; i++) {
    switch(vtype) {
      case ADDVAR_ROWNOS:
        ggobi_data_set_raw_value(d, i, jvar, (gfloat) (i + 1));
        break;
      case ADDVAR_BGROUP:
        ggobi_data_set_raw_value(d, i, jvar, (gfloat) d->clusterid.els[i]);
        break;
    }
  }
  g_signal_emit_by_name(d, "col_data_changed", jvar);
  return jvar;
}

void
clone_vars (gint * cols, gint ncols, GGobiData * d)
{
  gint i, k, jfrom, jto;
  gint nprev = ggobi_data_add_cols(d, ncols);
  
  for (k = 0; k < ncols; k++) {
    jfrom = cols[k];        
    jto = nprev + k; 

    /*-- copy the data --*/
    for (i = 0; i < d->nrows; i++) {
      ggobi_data_set_raw_value(d, i, jto, d->tform.vals[i][jfrom]);
      if (ggobi_data_is_missing(d, i, jfrom))
        ggobi_data_set_missing(d, i, jto);
    }

    vartable_copy_var(
      ggobi_data_get_vartable(d, jto), ggobi_data_get_vartable(d, jfrom)
    );
  }

  for (k = 0; k < ncols; k++) {
    g_signal_emit_by_name(d, "col_data_changed", cols[k]);
  }

}


static gint
is_variable_plotted (gint * cols, gint ncols, GGobiData * d)
{
  GList *dlist;
  displayd *display;
  gint jplotted = -1;

  if (!GGOBI_IS_GGOBI(d->gg))
    return 0;

  /*-- check each display for each variable --*/
  for (dlist = d->gg->displays; dlist; dlist = dlist->next) {
    display = (displayd *) dlist->data;
    if (display->d != d)
      continue;

    if (jplotted >= 0)
      break;

    if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
      GGobiExtendedDisplayClass *klass;
      klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
      jplotted = klass->variable_plotted_p (display, cols, ncols, d);
    }
  }

  return jplotted;
}

gboolean
delete_vars (gint * cols, gint ncols, GGobiData * d)
{
  gint j;
  gint *keepers, nkeepers;

  if (!GGOBI_IS_GGOBI(d->gg))
    return false;

  /*-- don't allow all variables to be deleted --*/
  if (ncols >= d->ncols)
    return false;

  /*
   * If one of the variables to be deleted is currently plotted,
   * we won't proceed until the user cleans up.
   */
  if ((j = is_variable_plotted (cols, ncols, d)) != -1) {
    gchar *message;
    message =
      g_strdup_printf
      ("Deletion failed; the variable '%s' is currently plotted\n",
       ggobi_data_get_col_name(d, j));
    quick_message (message, false);
    g_free (message);

    return false;
  }


  keepers = g_malloc ((d->ncols - ncols) * sizeof (gint));
  nkeepers = find_keepers (d->ncols, ncols, cols, keepers);
  if (nkeepers == -1) {
    g_free (keepers);
    return false;
  }

  if (d->vartable_tree_view[real] != NULL) {
    for (j = 0; j < ncols; j++) {
      GtkTreeModel *model;
      GtkTreeIter iter;
      GtkTreePath *path = gtk_tree_path_new_from_indices (cols[j], -1);
      model =
        gtk_tree_view_get_model (GTK_TREE_VIEW
                                 (d->vartable_tree_view[ggobi_data_get_col_type(d, cols[j])]));
      gtk_tree_model_get_iter (model, &iter, path);
      gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
      gtk_tree_path_free (path);
    }
  }

  for (j = 0; j < ncols; j++) {
    vartable_element_remove (cols[j], d);
  }

  /*-- delete columns from pipeline arrays --*/
  arrayf_delete_cols (&d->raw, ncols, cols);
  arrayf_delete_cols (&d->tform, ncols, cols);
  tour2d_realloc_down (ncols, cols, d, d->gg);
  tour1d_realloc_down (ncols, cols, d, d->gg);
  tourcorr_realloc_down (ncols, cols, d, d->gg);
  arrays_delete_cols (&d->missing, ncols, cols);
  arrayg_delete_cols (&d->jitdata, ncols, cols);

  /*-- reallocate the rest of the arrays --*/
  arrayg_alloc (&d->world, d->nrows, nkeepers);


  /*-- delete checkboxes --*/
  for (j = ncols - 1; j >= 0; j--) {
    varpanel_delete_nth (cols[j], d);
  }

  /*-- delete variable circles --*/
  for (j = ncols - 1; j >= 0; j--) {
    varcircles_delete_nth (cols[j], d);
  }

  d->ncols -= ncols;

  /*-- emit a single variable_list_changed signal when finished --*/
  /*-- doesn't need to give a variable index any more, really --*/
  g_signal_emit (G_OBJECT (d->gg),
                 GGobiSignals[VARIABLE_LIST_CHANGED_SIGNAL], 0, d);

  /*-- run the first part of the pipeline  --*/
  tform_to_world(d);


  g_free (keepers);

  return true;
}
