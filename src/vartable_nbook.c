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

#include "vartable.h"

#include <string.h> /* for strcmp() */


extern GtkWidget * vartable_buttonbox_build (ggobid *gg);
static void vartable_subwindow_init (datad *d, ggobid *gg);

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
vartable_notebook_adddata_cb (ggobid *gg, datad *d, void *notebook)
{
  vartable_subwindow_init (d, gg);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (GTK_WIDGET(notebook)),
    g_slist_length (gg->d) > 1);
}
CHECK_EVENT_SIGNATURE(vartable_notebook_adddata_cb, datad_added_f)

/*
 * Clear all selected rows from notebook pages
 * when they're de-selected.
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
}

GtkTreeModel *
vartable_tree_model_get (datad *d)
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
vartable_show_page (datad *d, ggobid *gg)
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
      if (strcmp (GTK_LABEL (tab_label)->label, d->name) == 0) {
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
vartable_iter_from_varno(gint var, datad *d, GtkTreeModel **model, GtkTreeIter *iter)
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

#if 0
gint
vartable_rownum_from_varno (gint jvar, vartyped vartype, datad *d)
{
  gint row = -1;
  gint rownum = 0;
  vartyped type = (vartype == categorical) ? categorical : real;
  
  if (d->vartable_tree_view[type] != NULL) {
    GtkTreeModel *model = d->vartable_tree_view[type];
	GtkTreeIter iter;
	gint col = gtk_tree_model_get_n_columns(model) - 1;
	gboolean row_valid = gtk_tree_model_get_iter_first(model, &iter);
    while (row_valid) {
		gtk_tree_model_get(model, &iter, col, &row);
        if (row == jvar)
          return rownum;
        rownum++;
		row_valid = gtk_tree_model_iter_next(model, &iter);
    }
  }
  return -1;
}

gint
vartable_varno_from_rownum (gint rownum, vartyped vartype, datad *d)
{
  gint row = -1;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(d->vartable_tree_view[vartype]));
  gint col = gtk_tree_model_get_n_columns(model) - 1;
  GtkTreePath *path = gtk_tree_path_new_from_indices(rownum, -1);
  gboolean row_valid = gtk_tree_model_get_iter(model, &iter, path);

  gtk_tree_path_free(path);
  
  if(row_valid) {
	  gtk_tree_model_get(model, &iter, col, &row);
  }
  
  return row;
}
#endif

/* GtkTreeSelection only tells us when the selection has changed,
	so we have to reset the selected status of the vartable */
void
selection_changed_cb (GtkTreeSelection *tree_sel, ggobid *gg)
{
  gint j;
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  vartabled *vt;
  GList *rows, *l;
  GtkTreeModel *model;
  
  for (j = 0 ; j < d->ncols ; j++) { /* clear selection */
    vt = vartable_element_get (j, d);
	vt->selected = false;
  }
  
  rows = gtk_tree_selection_get_selected_rows(tree_sel, &model);
  for (l = rows; l; l = l->next) {
	  gint varno;
	  GtkTreePath *path = (GtkTreePath*)l->data;
	  
	  varno = vartable_varno_from_path(model, path);
	  gtk_tree_path_free(path);
	  
	  vt = vartable_element_get (varno, d);
	  vt->selected = true;
  }
  g_list_free(rows);
}
#if 0
static void
vartable_row_assemble (gint jvar, vartyped type, gchar **row,
  datad *d, ggobid *gg)
{
/*-- a new (empty) row will be appended --*/
  switch (type) {
    case counter:
    case integer:
    case uniform:
    case real:
      row[REAL_CLIST_VARNO] = g_strdup_printf ("%d", jvar);
      row[REAL_CLIST_VARNAME] = g_strdup ("");
      row[REAL_CLIST_TFORM] = g_strdup ("");
      row[REAL_CLIST_USER_MIN] = g_strdup ("");
      row[REAL_CLIST_USER_MAX] = g_strdup ("");
      row[REAL_CLIST_DATA_MIN] = g_strdup_printf ("%8.3f", 0.0);
      row[REAL_CLIST_DATA_MAX] = g_strdup_printf ("%8.3f", 0.0);
      row[REAL_CLIST_MEAN] = g_strdup_printf ("%8.3f", 0.0);
      row[REAL_CLIST_MEDIAN] = g_strdup_printf ("%8.3f", 0.0);
      row[REAL_CLIST_NMISSING] = g_strdup_printf ("%d", 0);
    break;
    case categorical:
      row[CAT_CLIST_VARNO] = g_strdup_printf ("%d", jvar);
      row[CAT_CLIST_VARNAME] = g_strdup ("");
      row[CAT_CLIST_NLEVELS] = g_strdup ("");
      row[CAT_CLIST_LEVEL_NAME] = g_strdup ("");
      row[CAT_CLIST_LEVEL_VALUE] = g_strdup ("");
      row[CAT_CLIST_LEVEL_COUNT] = g_strdup ("");
      row[CAT_CLIST_USER_MIN] = g_strdup ("");
      row[CAT_CLIST_USER_MAX] = g_strdup ("");
      row[CAT_CLIST_DATA_MIN] = g_strdup ("");
      row[CAT_CLIST_DATA_MAX] = g_strdup ("");
      row[CAT_CLIST_NMISSING] = g_strdup ("");
    break;
    case all_vartypes:
      g_printerr ("(vartable_row_assemble) %d: illegal variable type %d\n",
        jvar, all_vartypes);
    break;
  }
}

#endif

/** 'row' here corresponds to 'variable' (top-level rows) */
void
vartable_row_append (gint jvar, datad *d, ggobid *gg)
{
  gint k;
  vartabled *vt = vartable_element_get (jvar, d);
  GtkTreeModel *model = vartable_tree_model_get(d);
  GtkTreeIter iter;
  GtkTreeIter child;
  if (!model)
	  return;
  gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);
  for (k=0; k<vt->nlevels; k++)
	  gtk_tree_store_append(GTK_TREE_STORE(model), &child, &iter);
}

static gboolean
real_filter_func (GtkTreeModel *model, GtkTreeIter *iter, datad *d)
{
	GtkTreePath *path = gtk_tree_model_get_path(model, iter);
	if (gtk_tree_path_get_depth(path) > 1)
		return(false);
	vartabled *vt = vartable_element_get (gtk_tree_path_get_indices(path)[0], d);
	gtk_tree_path_free(path);
	return(vt->vartype != categorical);
}
static gboolean
cat_filter_func (GtkTreeModel *model, GtkTreeIter *iter, datad *d)
{
	GtkTreePath *path = gtk_tree_model_get_path(model, iter);
	if (gtk_tree_path_get_depth(path) > 1)
		return(true);
	vartabled *vt = vartable_element_get (gtk_tree_path_get_indices(path)[0], d);
	gtk_tree_path_free(path);
	return(vt->vartype == categorical);
}

static void
vartable_subwindow_init (datad *d, ggobid *gg)
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
  
  g_signal_connect (G_OBJECT (nbook), "switch-page",
    G_CALLBACK (vartable_switch_page_cb), gg);

  lbl = datasetName (d, gg);
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
  for (j = 0 ; j < d->ncols ; j++) {
	  vartable_row_append(j, d, gg);
	  vartable_cells_set_by_var(j, d);
  }
  
  filter_model = gtk_tree_model_filter_new(GTK_TREE_MODEL(model), NULL);
  gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(filter_model), 
  	(GtkTreeModelFilterVisibleFunc)real_filter_func, d, NULL);
  sort_model = gtk_tree_model_sort_new_with_model(filter_model);
  d->vartable_tree_view[real] = gtk_tree_view_new_with_model(sort_model);
  populate_tree_view(d->vartable_tree_view[real], titles, G_N_ELEMENTS(titles), true,
  	GTK_SELECTION_MULTIPLE, G_CALLBACK(selection_changed_cb), gg);
  gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(d->vartable_tree_view[real]), true);
  
  /*-- right justify all the numerical columns --*/
  /*-- set the column width automatically --*/

  gtk_container_add (GTK_CONTAINER (sw), d->vartable_tree_view[real]);
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
  d->vartable_tree_view[categorical] = gtk_tree_view_new_with_model(sort_model);
  populate_tree_view(d->vartable_tree_view[categorical], titles_cat,
  	G_N_ELEMENTS(titles_cat), true, GTK_SELECTION_MULTIPLE, G_CALLBACK(selection_changed_cb), gg);
  gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(d->vartable_tree_view[categorical]), true);
  
  /*-- right justify all the numerical columns --*/

  gtk_container_add (GTK_CONTAINER (sw),
    d->vartable_tree_view[categorical]);
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
  
  
  
  gtk_widget_show_all (nbook);

}

void
vartable_open (ggobid *gg)
{                                  
  GtkWidget *vbox, *hbox;
  GSList *l;
  datad *d;

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

  /* Connecting to display_selected event */
  g_signal_connect (G_OBJECT (gg), "display_selected",
    G_CALLBACK (vartable_show_page_cb), NULL);
  /* */

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
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
  d = (gg->current_display ? gg->current_display->d : (datad *)gg->d->data);
  vartable_show_page (d, gg);
}

/*-------------------------------------------------------------------------*/
/*                 set values in the table                                 */
/*-------------------------------------------------------------------------*/

/*-- sets the name of the un-transformed variable --*/
void
vartable_collab_set_by_var (gint j, datad *d)
{
  vartabled *vt = vartable_element_get (j, d);
  gint k;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreeIter child;
  
  if (!vartable_iter_from_varno(j, d, &model, &iter))
	  return;
  
  if (vt) {
    switch (vt->vartype) {
      case categorical:
		
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
 			VT_NLEVELS, vt->nlevels, -1);
		gtk_tree_model_iter_children(model, &child, &iter);
    	/*-- set the level fields --*/
     	for (k=0; k<vt->nlevels; k++) {
            gtk_tree_store_set(GTK_TREE_STORE(model), &child, VT_LEVEL_NAME,
			  	vt->level_names[k], VT_LEVEL_VALUE, vt->level_values[k],
				VT_LEVEL_COUNT, vt->level_counts[k], -1);
			gtk_tree_model_iter_next(model, &child);
        }
	   // no more break
      case integer:
      case counter:
      case uniform:
      case real:
        gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			VT_VARNAME, vt->collab, -1);
      break;
      case all_vartypes:
        g_printerr ("(vartable_collab_set_by_var) illegal variable type %d\n", all_vartypes);
      break;
    }
  }
}

/*-- sets the name of the transformed variable --*/
void
vartable_collab_tform_set_by_var (gint j, datad *d)
{
  vartabled *vt;
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  if (!vartable_iter_from_varno(j, d, &model, &iter))
	  return;

  vt = vartable_element_get (j, d);
  if (vt->tform0 == NO_TFORM0 &&
	vt->tform1 == NO_TFORM1 &&
	vt->tform2 == NO_TFORM2)
  {
	  gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			VT_TFORM, "", -1);
  } else {
	  gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			VT_TFORM, vt->collab_tform, -1);
  }
}

/*-- sets the limits for a variable --*/
void
vartable_limits_set_by_var (gint j, datad *d)
{
  vartabled *vt = vartable_element_get (j, d);
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  if (!vartable_iter_from_varno(j, d, &model, &iter))
	  return;
  
  if (vt) {
    //gint rownum = vartable_rownum_from_varno (j, vt->vartype, d);

    switch (vt->vartype) {
      case integer:
      case counter:
      case uniform:
      case real:
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
		 	VT_REAL_DATA_MIN, vt->lim_display.min,
			VT_REAL_DATA_MAX, vt->lim_display.max, -1);
		if (vt->lim_specified_p) {
			gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
		  		VT_REAL_USER_MIN, vt->lim_specified_tform.min,
				VT_REAL_USER_MAX, vt->lim_specified_tform.max, -1);
        }
      break;

      case categorical:
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
			VT_CAT_DATA_MIN, (gint)vt->lim_display.min,
			VT_CAT_DATA_MAX, (gint)vt->lim_display.max, -1); 
  	    if (vt->lim_specified_p) {
			gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
		  		VT_CAT_USER_MIN, (gint)vt->lim_specified_tform.min,
				VT_CAT_USER_MAX, (gint)vt->lim_specified_tform.max, -1);
        }
      break;
      case all_vartypes:
        g_printerr ("(vartable_limits_set_by_var) %d: illegal variable type %d\n",
          j, all_vartypes);
      break;
    }
  }
}
void
vartable_limits_set (datad *d) 
{
  gint j;
  if (d->vartable_tree_model != NULL)
    for (j=0; j<d->ncols; j++)
      vartable_limits_set_by_var (j, d);
}

/*-- sets the mean, median, and number of missings for a variable --*/
void
vartable_stats_set_by_var (gint j, datad *d) {
  vartabled *vt = vartable_element_get (j, d);
  vartyped type;
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (!vartable_iter_from_varno(j, d, &model, &iter))
	  return;
  
  if (vt) {
    //gint rownum = vartable_rownum_from_varno (j, vt->vartype, d);
    switch (vt->vartype) {
      case integer:
      case counter:
      case uniform:
      case real:
        type = real;
        /*-- for counter variables, don't display the mean --*/
		if (vt->vartype != counter)
		  gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
			VT_MEAN, vt->mean, VT_MEDIAN, vt->median, -1);
      //break;
	  case categorical:
        gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
			VT_NMISSING, vt->nmissing, -1);
      break;
      case all_vartypes:
        g_printerr ("(vartable_stats_set_by_var) %d: illegal variable type %d\n",
          j, vt->vartype);
      break;
    }
  }
}

void
vartable_stats_set (datad *d) {
  gint j;

  if (d->vartable_tree_model != NULL)
    for (j=0; j<d->ncols; j++)
      vartable_stats_set_by_var (j, d);
}

/*
 * in one routine, populate every cell in a row -- all these
 * functions call gtk_tree_view_set_text.
*/
void
vartable_cells_set_by_var (gint j, datad *d) 
{
  vartable_stats_set_by_var (j, d);
  vartable_limits_set_by_var (j, d);
  vartable_collab_set_by_var (j, d);
  vartable_collab_tform_set_by_var (j, d);
}
