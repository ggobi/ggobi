/* vartable.c */
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

#include "ggobi-variable.h"

extern gdouble no_change (gdouble, gdouble);


/*-------------------------------------------------------------------------*/
/*                         utilities                                       */
/*-------------------------------------------------------------------------*/

/*-- also used in varchange.c --*/
gboolean
array_contains (gint* arr, gint n, gint el)
{
  gint j;

  for (j=0; j<n; j++)
    if (arr[j] == el)
      return true;
  
  return false;
}

/* we currently use the GGobiStage to access the vartable widgets, but
  in the future we will get these somehow from the GGobiSession, so we keep
  both parameters for now */
gint
selected_cols_get (gint **cols, GGobiStage *d, GGobiSession *gg)
{
/*
 * Figure out which columns are selected.
*/
  
  // FIXME: for now we just allocate memory for all cols, since clients often
  // use this array as a template. They probably shouldn't.
  gint j, ncols = d->n_cols;
  /* Only allocate space for the number of selected cols
  for (j = 0; j < GGOBI_VARIABLE_ALL_VARTYPES; j++) {
    GtkTreeView *view = GTK_TREE_VIEW(d->vartable_tree_view[j]);
    GtkTreeSelection *sel;
    if (!view)
      continue;
    sel = gtk_tree_view_get_selection(view);
    ncols += gtk_tree_selection_count_selected_rows(sel);
  } */
  *cols = g_new(gint, ncols);
  ncols = 0;
  for (j = 0; j < GGOBI_VARIABLE_ALL_VARTYPES; j++) {
    GtkTreeModel *model;
    GtkTreeView *view = GTK_TREE_VIEW(d->vartable_tree_view[j]);
    GtkTreeSelection *sel;
    if (!view)
      continue;
    sel = gtk_tree_view_get_selection(view);
    GList *rows = gtk_tree_selection_get_selected_rows(sel, &model), *tmp_rows;
    for (tmp_rows = rows; tmp_rows; tmp_rows = tmp_rows->next)
      // FIXME: not efficient because we have to dig for the root model each time
      (*cols)[ncols++] = vartable_varno_from_path(model, 
        (GtkTreePath *)tmp_rows->data);
    g_list_foreach(rows, (GFunc)gtk_tree_path_free, NULL);
    g_list_free(rows);
  }
  
  return (ncols);
}

/*
 * When there aren't any columns in the variable statistics table,
 * this is how we find out which columns are selected for plotting.
*/
gint
plotted_cols_get (gint *cols, GGobiStage *d, GGobiSession *gg) 
{
  gint ncols = 0;
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;

  if(GGOBI_IS_EXTENDED_DISPLAY(display)) {
     GGobiExtendedDisplayClass *klass;
     klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS(display);
     ncols = klass->plotted_vars_get(display, cols, d, gg);
  }

  return ncols;
}
