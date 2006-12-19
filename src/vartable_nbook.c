/* vartable_nbook.c */ 
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

/* interface code for the variable statistics table: notebook only */

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "ggobi-variable.h"

#include <string.h> /* for strcmp() */

enum { 
  VT_VARNAME, VT_TFORM,
  VT_REAL_USER_MIN, VT_REAL_USER_MAX,
  VT_REAL_DATA_MIN, VT_REAL_DATA_MAX,
  VT_MEAN, VT_MEDIAN,
  VT_NLEVELS, VT_LEVEL_NAME, VT_LEVEL_VALUE, VT_LEVEL_COUNT,
  VT_CAT_USER_MIN, VT_CAT_USER_MAX,
  VT_CAT_DATA_MIN, VT_CAT_DATA_MAX,
  VT_NMISSING,
  NCOLS_VT
  };
  
extern GtkWidget * vartable_buttonbox_build (ggobid *gg);
static void vartable_subwindow_init (GGobiStage *d, ggobid *gg);

/*-------------------------------------------------------------------------*/
/*            Listen for display_selected events                           */
/*-------------------------------------------------------------------------*/

/* Update variable selection panel */
void 
vartable_show_page_cb (ggobid *gg, displayd *display) {
  vartable_show_page(display->d, gg);
}

/*-------------------------------------------------------------------------*/

static void close_wmgr_cb (GtkWidget *cl, GdkEventButton *event, ggobid *gg)
{
  gtk_widget_hide (gg->vartable_ui.window);
}
static void destroyit (ggobid *gg)
{
  gtk_widget_destroy (gg->vartable_ui.window);
  gg->vartable_ui.window = NULL;
}

static void 
vartable_notebook_adddata_cb (ggobid *gg, GGobiStage *d, void *notebook)
{
  vartable_subwindow_init (d, gg);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (GTK_WIDGET(notebook)),
    g_slist_length (gg->d) > 1);
}
CHECK_EVENT_SIGNATURE(vartable_notebook_adddata_cb, datad_added_f)

GGobiVariableType
tree_view_get_type (GGobiStage *d, GtkWidget *tree_view)
{
  GGobiVariableType vartype = GGOBI_VARIABLE_ALL_VARTYPES;
  if (d->vartable_tree_view[GGOBI_VARIABLE_REAL] != NULL) {
    if (tree_view == d->vartable_tree_view[GGOBI_VARIABLE_REAL])
      vartype = GGOBI_VARIABLE_REAL;
  } else if (d->vartable_tree_view[GGOBI_VARIABLE_CATEGORICAL] != NULL) {
    if (tree_view == d->vartable_tree_view[GGOBI_VARIABLE_CATEGORICAL])
      vartype = GGOBI_VARIABLE_CATEGORICAL;
  } else if (d->vartable_tree_view[GGOBI_VARIABLE_INTEGER] != NULL) {
    if (tree_view == d->vartable_tree_view[GGOBI_VARIABLE_INTEGER])
      vartype = GGOBI_VARIABLE_INTEGER;
  } else if (d->vartable_tree_view[GGOBI_VARIABLE_COUNTER] != NULL) {
    if (tree_view == d->vartable_tree_view[GGOBI_VARIABLE_COUNTER])
      vartype = GGOBI_VARIABLE_COUNTER;
  } else if (d->vartable_tree_view[GGOBI_VARIABLE_UNIFORM] != NULL) {
    if (tree_view == d->vartable_tree_view[GGOBI_VARIABLE_UNIFORM])
      vartype = GGOBI_VARIABLE_UNIFORM;
  }

  return vartype;
}

/*
 * Clear all selected rows from notebook pages when they're
 * de-selected.  This callback applies to swapping pages for
 * variable types within a datad.
*/
void
vartable_switch_page_cb (GtkNotebook *notebook, GtkNotebookPage *page,
  gint page_num, ggobid *gg)
{
  gint prev_page = gtk_notebook_get_current_page (notebook);
  GtkWidget *swin, *tree_view;
  GList *children;

  if (prev_page > -1) {
    GtkTreeSelection *tree_sel;
    swin = gtk_notebook_get_nth_page (notebook, prev_page);
    children = gtk_container_get_children (GTK_CONTAINER (swin));
    tree_view = g_list_nth_data (children, 0);
    tree_sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
    gtk_tree_selection_unselect_all(tree_sel);
  }

  // Figure out the page type from page_num -- doesn't do anything
  // yet.  dfs
  if (page_num > -1) {
    GGobiVariableType vartype;
    GGobiStage *d = datad_get_from_notebook(gg->vartable_ui.notebook);
    swin = gtk_notebook_get_nth_page (notebook, page_num);
    children = gtk_container_get_children (GTK_CONTAINER (swin));
    tree_view = g_list_nth_data (children, 0);
    vartype = tree_view_get_type(d, tree_view);
    //vartable_buttons_update(vartype, gg);
    // g_printerr ("vartype %d\n", vartype);
  }


}

GtkTreeModel *
vartable_tree_model_get (GGobiStage *d)
{
  return(d->vartable_tree_model);
}

GtkWidget *
vartable_tree_view_get (ggobid *gg) {
  GtkNotebook *nb, *subnb;
  gint indx, subindx;
  GtkWidget *swin;
  GList *children;
/*
 * Each page of vartable_ui.notebook has one child, which is
 * another notebook.
 * That notebook has two children, two scrolled windows, and
 * each scrolled window has one child, a tree_view
*/
/*
  vartable_ui.notebook
    page 0: datad 0
      nbook
        page 0: swin -> real
        page 1: swin -> categorical
    page n: datad n
      nbook
        page 0: swin -> real
        page 1: swin -> categorical
*/

  nb = GTK_NOTEBOOK (gg->vartable_ui.notebook);
  indx = gtk_notebook_get_current_page (nb);
  /*-- get the current page of the vartable notebook --*/
  subnb = (GtkNotebook *) gtk_notebook_get_nth_page (nb, indx);
  subindx = gtk_notebook_get_current_page (subnb);
  /*-- get the current page of the variable type notebook --*/
  swin = gtk_notebook_get_nth_page (subnb, subindx);
  children = gtk_container_get_children (GTK_CONTAINER (swin));

  return ((GtkWidget *) g_list_nth_data (children, 0));
/*
  swin = gtk_notebook_get_nth_page (nb, indx);
  GList *swin_children = gtk_container_get_children (GTK_CONTAINER (swin));
*/
}

void
vartable_show_page (GGobiStage *d, ggobid *gg)
{
  GtkNotebook *nb;
  gint page, page_new;
  GList *l, *children;
  GtkWidget *child, *tab_label;

  if (gg == NULL || gg->vartable_ui.notebook == NULL)
    return;


  nb = GTK_NOTEBOOK (gg->vartable_ui.notebook);
  page = gtk_notebook_get_current_page (nb);

  if (page < 0)
    return;

  page_new = 0;
  children = gtk_container_get_children (GTK_CONTAINER (gg->vartable_ui.notebook));
  for (l = children; l; l = l->next) {
    child = l->data;
    tab_label = (GtkWidget *) gtk_notebook_get_tab_label (nb, child);
    if (tab_label && GTK_IS_LABEL (tab_label)) {
      if (d->name && strcmp (GTK_LABEL (tab_label)->label, d->name) == 0) {
        if (page != page_new) {
          gtk_notebook_set_current_page (nb, page_new);
          break;
        }
      }
    }
    page_new++;
  }
}

gint
vartable_varno_from_path(GtkTreeModel *model, GtkTreePath *path)
{
  GtkTreeModel *unsorted_model, *root_model;
  GtkTreePath *unsorted_path, *root_path;
  gint varno;
	
  unsorted_model = gtk_tree_model_sort_get_model(GTK_TREE_MODEL_SORT(model));
  g_object_get(G_OBJECT(unsorted_model), "child-model", &root_model, NULL);
	
  unsorted_path = gtk_tree_model_sort_convert_path_to_child_path(
    GTK_TREE_MODEL_SORT(model), path);
  root_path = gtk_tree_model_filter_convert_path_to_child_path(
    GTK_TREE_MODEL_FILTER(unsorted_model), unsorted_path);
	
  varno = gtk_tree_path_get_indices(root_path)[0];
	
  gtk_tree_path_free(unsorted_path);
  gtk_tree_path_free(root_path);
	
  return(varno);
}
gboolean
vartable_iter_from_varno(gint var, GGobiStage *d, GtkTreeModel **model, GtkTreeIter *iter)
{
  GtkTreeModel *loc_model;
  GtkTreePath *path;
  gboolean valid;
  
  loc_model = vartable_tree_model_get(d);
  if (!loc_model)
	  return(FALSE);
  path = gtk_tree_path_new_from_indices(var, -1);
  valid = gtk_tree_model_get_iter(loc_model, iter, path);
  gtk_tree_path_free(path);
  
  if (model)
    *model = loc_model;
  
  return(valid);
}

/** 'row' here corresponds to 'variable' (top-level rows) */
void
vartable_row_append (gint jvar, GGobiStage *d)
{
  gint k;
  GtkTreeModel *model = vartable_tree_model_get(d);
  GtkTreeIter iter;
  GtkTreeIter child;
  GGobiVariable *var = ggobi_stage_get_variable(d, jvar);
  if (!model)
    return;
  gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);
  /* If not categorical, we're finished */
  if (!GGOBI_VARIABLE_IS_CATEGORICAL(var)) return;
  /* If categorical, continue: add a row for each level */
  for (k=0; k < ggobi_variable_get_n_levels(var); k++)
    gtk_tree_store_append(GTK_TREE_STORE(model), &child, &iter);
}

static gboolean
real_filter_func (GtkTreeModel *model, GtkTreeIter *iter, GGobiStage *d)
{
  GtkTreePath *path = gtk_tree_model_get_path(model, iter);
  if (gtk_tree_path_get_depth(path) > 1)
    return(false);
  GGobiVariable *var = ggobi_stage_get_variable(d, gtk_tree_path_get_indices(path)[0]);
  gtk_tree_path_free(path);
  return(!GGOBI_VARIABLE_IS_CATEGORICAL(var));
}
static gboolean
cat_filter_func (GtkTreeModel *model, GtkTreeIter *iter, GGobiStage *d)
{
  GtkTreePath *path = gtk_tree_model_get_path(model, iter);
  if (gtk_tree_path_get_depth(path) > 1)
    return(true);
  GGobiVariable *var = ggobi_stage_get_variable(d, gtk_tree_path_get_indices(path)[0]);
  gtk_tree_path_free(path);
  return(GGOBI_VARIABLE_IS_CATEGORICAL(var));
}


static void
vartable_changed_col_foreach (guint j, GGobiStage *s)
{
  vartable_stats_set_by_var (s, j);
  vartable_limits_set_by_var (s, j);
}

static void
vartable_removed_col_foreach (guint j, GGobiStage *d)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreePath *path = gtk_tree_path_new_from_indices (j, -1);
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(d->vartable_tree_view[ggobi_stage_get_col_type(d, j)]));
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
  gtk_tree_path_free (path);
}

static void
vartable_stage_changed_cb (GGobiStage *s, GGobiPipelineMessage *msg, gpointer user_data)
{
  ggobi_pipeline_message_changed_cols_foreach(msg, 
    (GGobiIndexFunc)vartable_changed_col_foreach, s);
  ggobi_pipeline_message_removed_cols_foreach(msg, 
    (GGobiIndexFunc)vartable_removed_col_foreach, s);
}

static void
vartable_subwindow_init (GGobiStage *d, ggobid *gg)
{
  gint j;
  GtkWidget *sw, *wlbl;
  gchar *lbl;
  static gchar *titles[] = {
    "Variable",
    "Transform",
    "Min (user)", "Max (user)",
    "Min (data)", "Max (data)",
    "Mean", "Median",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "N NAs"};
  static gchar *titles_cat[] = {
    "Variable",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "N Levels",
    "Level",
    "Value",
    "Count",
    "Min (user)", "Max (user)",
    "Min (data)", "Max (data)",
    "N NAs",
  };
  GtkWidget *nbook = gtk_notebook_new ();
  GtkTreeStore *model;
  GtkTreeModel *sort_model, *filter_model;
  
  d = ggobi_stage_find(d, GGOBI_MAIN_STAGE_TRANSFORM);
  
  g_signal_connect (G_OBJECT (nbook), "switch-page",
    G_CALLBACK (vartable_switch_page_cb), gg);

  lbl = ggobi_stage_get_name(d);
  /*
   * We're showing all datasets for now, whether they have variables
   * or not.  That could change.
  */
  g_object_set_data(G_OBJECT(nbook), "datad", d);  /*setdata*/
  gtk_notebook_append_page (GTK_NOTEBOOK (gg->vartable_ui.notebook),
    nbook, gtk_label_new (lbl));
  g_free (lbl);


  /* Pack each tree_view into a scrolled window */
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_NONE);
 
/*
 * Page for real, counter and integer variables
*/
  model = gtk_tree_store_new(NCOLS_VT, G_TYPE_STRING, G_TYPE_STRING,
    G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, 
    G_TYPE_DOUBLE, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, 
    G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
	
  d->vartable_tree_model = GTK_TREE_MODEL(model);
  
  /*-- populate the tables BEFORE attaching filters --*/
  for (j = 0 ; j < d->n_cols ; j++) {
    vartable_row_append(j, d);
    vartable_cells_set_by_var(j, d);
  }
  
  filter_model = gtk_tree_model_filter_new(GTK_TREE_MODEL(model), NULL);
  gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(filter_model), 
  	(GtkTreeModelFilterVisibleFunc)real_filter_func, d, NULL);
  sort_model = gtk_tree_model_sort_new_with_model(filter_model);
  d->vartable_tree_view[GGOBI_VARIABLE_REAL] = gtk_tree_view_new_with_model(sort_model);
  populate_tree_view(d->vartable_tree_view[GGOBI_VARIABLE_REAL], titles, 
    G_N_ELEMENTS(titles), true,
    GTK_SELECTION_MULTIPLE, /*G_CALLBACK(selection_changed_cb)*/ NULL, gg);
  gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(d->vartable_tree_view[GGOBI_VARIABLE_REAL]), true);
  
  /*-- right justify all the numerical columns --*/
  /*-- set the column width automatically --*/

  gtk_container_add (GTK_CONTAINER (sw), d->vartable_tree_view[GGOBI_VARIABLE_REAL]);
  wlbl = gtk_label_new_with_mnemonic("_Real");
/*
This works for showing tooltips in the tabs, but unfortunately it
interferes with the normal operation of the widget -- I can't switch
pages any more!
  GtkWidget *ebox;
  ebox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (ebox), wlbl);
  gtk_widget_show(wlbl);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), ebox,
    "Table of statistics for real, integer and counter variables", NULL);
  gtk_notebook_append_page (GTK_NOTEBOOK (nbook), scrolled_window, ebox);
*/
  gtk_notebook_append_page (GTK_NOTEBOOK (nbook), sw, wlbl);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_NONE);

/*
 * Page for categorical variables
*/
  
  filter_model = gtk_tree_model_filter_new(GTK_TREE_MODEL(model), NULL);
  gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(filter_model), 
  	(GtkTreeModelFilterVisibleFunc)cat_filter_func, d, NULL);
  sort_model = gtk_tree_model_sort_new_with_model(filter_model);
  d->vartable_tree_view[GGOBI_VARIABLE_CATEGORICAL] = gtk_tree_view_new_with_model(sort_model);
  populate_tree_view(d->vartable_tree_view[GGOBI_VARIABLE_CATEGORICAL], titles_cat,
  	G_N_ELEMENTS(titles_cat), true, GTK_SELECTION_MULTIPLE, /*G_CALLBACK(selection_changed_cb)*/ NULL, gg);
  gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(d->vartable_tree_view[GGOBI_VARIABLE_CATEGORICAL]), true);
  
  /*-- right justify all the numerical columns --*/

  gtk_container_add (GTK_CONTAINER (sw),
    d->vartable_tree_view[GGOBI_VARIABLE_CATEGORICAL]);
  wlbl = gtk_label_new_with_mnemonic("_Categorical");
/*
  ebox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (ebox), wlbl);
  gtk_widget_show(wlbl);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), ebox,
    "Table of statistics for categorical variables", NULL);
  gtk_notebook_append_page (GTK_NOTEBOOK (nbook), scrolled_window, ebox);
*/
  gtk_notebook_append_page (GTK_NOTEBOOK (nbook), sw, wlbl);

  /*-- 3 = COLUMN_INSET --*/
  
  ggobi_stage_connect__changed(d, vartable_stage_changed_cb, NULL);
  gtk_widget_show_all (nbook);

}

void
vartable_open (ggobid *gg)
{                                  
  GtkWidget *vbox, *hbox;
  GSList *l;
  GGobiStage *d;

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length (gg->d) == 0) 
	  return;

  /*-- if new datad's have been added, the user has to reopen the window --*/
  if (gg->vartable_ui.window != NULL) {
    destroyit (gg);
  }

  gg->vartable_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(gg->vartable_ui.window), 750, 300);
  g_signal_connect (G_OBJECT (gg->vartable_ui.window),
    "delete_event", G_CALLBACK (close_wmgr_cb), gg);
  gtk_window_set_title (GTK_WINDOW (gg->vartable_ui.window),
    "Variable Manipulation");

  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER (gg->vartable_ui.window), vbox);
  gtk_widget_show (vbox);

  /* Create a notebook, set the position of the tabs */
  gg->vartable_ui.notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gg->vartable_ui.notebook),
    GTK_POS_TOP);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gg->vartable_ui.notebook),
    g_slist_length (gg->d) > 1);
  gtk_box_pack_start (GTK_BOX (vbox), gg->vartable_ui.notebook,
    true, true, 2);
  /* Needed?: a switch-page callback so that we can keep track of
   * the GGobiVariableType of the current page, and show or hide buttons
   * as appropriate -- dfs */

  /* Connecting to display_selected event */
  g_signal_connect (G_OBJECT (gg), "display_selected",
    G_CALLBACK (vartable_show_page_cb), NULL);
  /* */

  for (l = gg->d; l; l = l->next) {
    d = (GGobiStage *) l->data;
    vartable_subwindow_init (d, gg);
  }

  /*-- listen for datad_added events --*/
  g_signal_connect (G_OBJECT (gg),
    "datad_added", G_CALLBACK (vartable_notebook_adddata_cb),
     GTK_OBJECT (gg->vartable_ui.notebook));

  hbox = vartable_buttonbox_build (gg);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 1);

  gtk_widget_show_all (gg->vartable_ui.window);

  /*-- set it to the page corresponding to the current display --*/
  d = (gg->current_display ? gg->current_display->d : (GGobiStage *)gg->d->data);
  vartable_show_page (d, gg);
}

/*-------------------------------------------------------------------------*/
/*                 set values in the table                                 */
/*-------------------------------------------------------------------------*/

/*-- sets the name of the un-transformed variable --*/
void
vartable_collab_set_by_var (GGobiStage *d, guint j)
{
  GGobiVariable *var = ggobi_stage_get_variable(d, j);
  gint k;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreeIter child;
  gchar *fmtname;

  if (!vartable_iter_from_varno(j, d, &model, &iter))
	  return;
  
  if (GGOBI_VARIABLE_IS_CATEGORICAL(var)) {
    gint *values = ggobi_variable_get_level_values(var);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
      VT_NLEVELS, ggobi_variable_get_n_levels(var), -1);
    gtk_tree_model_iter_children(model, &child, &iter);

    /*-- set the level fields --*/
    for (k=0; k<ggobi_variable_get_n_levels(var); k++) {
      fmtname = g_markup_printf_escaped("%s", ggobi_variable_get_level_name(var, values[k]));
      gtk_tree_store_set(GTK_TREE_STORE(model), &child, VT_LEVEL_NAME,
        fmtname, VT_LEVEL_VALUE, values[k],
        VT_LEVEL_COUNT, ggobi_variable_get_level_count(var, values[k]), -1);
      g_free(fmtname);
      gtk_tree_model_iter_next(model, &child);
    }
    g_free(values);
  }
  gtk_tree_store_set(GTK_TREE_STORE(model), &iter, VT_VARNAME, 
    ggobi_stage_get_col_name(ggobi_stage_get_root(d), j), -1);

}

/*-- sets the name of the transformed variable --*/
void
vartable_collab_tform_set_by_var (GGobiStage *d, guint j)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  if (!vartable_iter_from_varno(j, d, &model, &iter))
	  return;

  gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
    VT_TFORM, ggobi_stage_get_col_name(d, j), -1);
}

/*-- sets the limits for a variable --*/
void
vartable_limits_set_by_var (GGobiStage *d, guint j)
{
  GGobiVariable *var = ggobi_stage_get_variable(d, j);
  GtkTreeModel *model;
  GtkTreeIter iter;
  if (!vartable_iter_from_varno(j, d, &model, &iter))
	  return;
  
  if (var) {

    switch (ggobi_variable_get_vartype(var)) {
      case GGOBI_VARIABLE_INTEGER:
      case GGOBI_VARIABLE_COUNTER:
      case GGOBI_VARIABLE_UNIFORM:
      case GGOBI_VARIABLE_REAL:
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
	 	VT_REAL_DATA_MIN, ggobi_variable_get_display_min(var),
		VT_REAL_DATA_MAX, ggobi_variable_get_display_max(var), -1);
	if (var->lim_specified_p) {
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
	  	  VT_REAL_USER_MIN, var->lim_specified.min,
		  VT_REAL_USER_MAX, var->lim_specified.max, -1);
        }
      break;

      case GGOBI_VARIABLE_CATEGORICAL:
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
		VT_CAT_DATA_MIN, ggobi_variable_get_display_min((gint)var),
		VT_CAT_DATA_MAX, ggobi_variable_get_display_max((gint)var), -1); 
        if (var->lim_specified_p) {
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
	  	  VT_CAT_USER_MIN, (gint)var->lim_specified.min,
		  VT_CAT_USER_MAX, (gint)var->lim_specified.max, -1);
        }
      break;
      case GGOBI_VARIABLE_ALL_VARTYPES:
        g_printerr ("(vartable_limits_set_by_var) %d: illegal variable type %d\n",
          j, GGOBI_VARIABLE_ALL_VARTYPES);
      break;
    }
  }
}
void
vartable_limits_set (GGobiStage *d) 
{
  gint j;
  if (d->vartable_tree_model != NULL)
    for (j=0; j<d->n_cols; j++)
      vartable_limits_set_by_var (d, j);
}

/*-- sets the mean, median, and number of missings for a variable --*/
void
vartable_stats_set_by_var (GGobiStage *d, guint j) {
  GGobiVariable *var = ggobi_stage_get_variable(d, j);
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (!var || !vartable_iter_from_varno(j, d, &model, &iter))
    return;
  
  switch (ggobi_stage_get_col_type(d, j)) {
    case GGOBI_VARIABLE_COUNTER:
      break;
    case GGOBI_VARIABLE_CATEGORICAL:
      gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
        VT_NMISSING, ggobi_stage_get_col_n_missing(d, j), -1);
    break;
    default:
      gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
        VT_MEAN, ggobi_variable_get_mean(var), 
        VT_MEDIAN, ggobi_variable_get_median(var), 
        -1
      );
    break;
  }
}
void
vartable_stats_set (GGobiStage *d) {
  gint j;

  if (!d->vartable_tree_model)
    return;
    
  for (j=0; j<d->n_cols; j++)
    vartable_stats_set_by_var (d, j);
}

/*
 * in one routine, populate every cell in a row -- all these
 * functions call gtk_tree_view_set_text.
*/
void
vartable_cells_set_by_var (gint j, GGobiStage *d) 
{
  vartable_collab_set_by_var (d, j);
  vartable_collab_tform_set_by_var (d, j);
  vartable_stats_set_by_var (d, j);
  vartable_limits_set_by_var (d, j);
}
/*
void 
vartable_col_name_changed(GGobiStage *d, guint j, GtkWidget* w) {
}

void 
vartable_col_data_changed(GGobiStage *d, guint j, GtkWidget* w) {
}

void 
vartable_col_rows_added(GGobiStage *d, guint n, GtkWidget* w) {  
}


void vartable_init(GGobiStage *d) {
  GtkWidget *widget = vartable_gui_init(d);
  
  vartable_col_rows_added(d, d->n_cols);
  for (guint j = 0; j < d->n_cols; j++) {
    vartable_col_name_changed(d, j);
    vartable_col_data_changed(d, j);
  }

  ggobi_data_connect__cols_added(GGOBI_DATA(d), vartable_cols_added, widget);
  ggobi_data_connect__col_name_changed(GGOBI_DATA(d), vartable_col_name_changed, widget;
  ggobi_data_connect__col_data_changed(GGOBI_DATA(d), vartable_col_data_changed, widget);
}*/
