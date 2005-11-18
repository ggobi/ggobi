/*-- utils_ui.c --*/
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
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


GtkWidget *
CreateMenuItemWithCheck (GtkWidget *menu,
 gchar *szName, gchar *szAccel, gchar *szTip,
 GtkWidget *win_main, GtkAccelGroup *accel_group,
 GtkSignalFunc func, gpointer data, ggobid *gg,
 GSList *radiogroup, gboolean check);

/*
 * Taken from 'Developing Linux Applications with GTK+ and GDK'
 * by Eric Harlow.
*/

/*
 * CreateMenuItem
 *
 * Creates an item and puts it in the menu and returns the item.
 *
 * menu - container menu
 * szName - Name of the menu - NULL for a separator
 * szAccel - Acceleration string - "^C" for Control-C
 * szTip - Tool tip
 * func - Call back function
 * data - call back function data
 *
 * returns new menuitem
 */
GtkWidget *
CreateMenuItem (GtkWidget *menu,
  gchar *szName, gchar *szAccel, gchar *szTip,
  GtkWidget *win_main, GtkAccelGroup *accel_group,
  GtkSignalFunc func, gpointer data, ggobid *gg)
{
  return(CreateMenuItemWithCheck(menu, szName, szAccel, szTip,
    win_main, accel_group, func, data, gg, NULL, false));
}

GtkWidget *
CreateMenuItemWithCheck (GtkWidget *menu,
  gchar *szName, gchar *szAccel, gchar *szTip,
  GtkWidget *win_main, GtkAccelGroup *accel_group,
  GtkSignalFunc func, gpointer data, ggobid *gg, 
  GSList *RadioGroup, gboolean check)
{
  GtkWidget *menuitem;

  if(check && RadioGroup == NULL) {
     menuitem = gtk_radio_menu_item_new(RadioGroup);
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), TRUE);
     RadioGroup = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menuitem));
  }

  /* --- If there's a name, create the item and add the signal handler --- */
  if (szName && strlen (szName)) {
    menuitem = check ? gtk_radio_menu_item_new_with_label(RadioGroup, szName) : gtk_menu_item_new_with_label (szName);
    if(func)
      g_signal_connect (G_OBJECT (menuitem), "activate",
			  G_CALLBACK (func), data);

    GGobi_widget_set (GTK_WIDGET (menuitem), gg,  true);

  } else {
    /* --- Create a separator --- */
    menuitem = check ? gtk_radio_menu_item_new(RadioGroup) : gtk_menu_item_new ();
  }


  if(check)
     RadioGroup = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menuitem));

  /* --- Add menu item to the menu and show it. --- */
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
  gtk_widget_show (menuitem);

  /* --- If there was an accelerator --- */
  if (szAccel && accel_group) {
    if (szAccel[0] == '^') {  /* control-keypress */
      gtk_widget_add_accelerator (menuitem, "activate", accel_group,
        szAccel[1], GDK_CONTROL_MASK,
        GTK_ACCEL_VISIBLE);
      /* How can I find out if there's an item already using this
	 signal?  Sheesh, nothing seems to work. */

    } else {                  /* alt-keypress */
      gtk_widget_add_accelerator (menuitem, "activate", accel_group,
        szAccel[0], GDK_MOD1_MASK,
        GTK_ACCEL_VISIBLE);
    }
  }

  /* --- If there was a tool tip --- */
  if (szTip && strlen (szTip))
      gtk_tooltips_set_tip (gg->tips, menuitem, szTip, NULL);

  return (menuitem);
}

/*
 * Taken from 'Developing Linux Applications with GTK+ and GDK'
 * by Eric Harlow.
*/

/*
 * CreateMenuCheck
 *
 * Create a menu checkbox
 *
 * menu - container menu
 * szName - name of the menu
 * func - Call back function.
 * data - call back function data
 * state - whether it's on or not
 *
 * returns new menuitem
 */
GtkWidget *CreateMenuCheck (GtkWidget *menu,
                            gchar *szName,
                            GtkSignalFunc func,
                            gpointer data,
                            gboolean state, ggobid *gg)
{
    GtkWidget *menuitem;

    /* --- Create menu item --- */
    menuitem = gtk_check_menu_item_new_with_label (szName);

    /*-- display always, not just when the mouse floats over --*/
	/* I don't even think this is possible in GTK2 - mfl */
    //gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (menuitem), true);

    GGobi_widget_set(GTK_WIDGET(menuitem), gg, true);

    /* --- set its state --- */
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem), state);

    /* --- Add it to the menu --- */
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    gtk_widget_show (menuitem);

    /* --- Listen for "toggled" messages --- */
    g_signal_connect (G_OBJECT (menuitem), "toggled",
                        G_CALLBACK (func), data);

    return (menuitem);
}

/*
 * Function to open a dialog box displaying the message provided.
 * Now based on GTK2 convenience class GtkMessageDialog
*/

void quick_message (const gchar * const message, gboolean modal) {

  GtkWidget *dialog;
    
  /* Create the widgets */
    
  dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, message);

  if (modal)
    gtk_window_set_modal (GTK_WINDOW (dialog), true);

  gtk_dialog_run(GTK_DIALOG(dialog));
  
  gtk_widget_destroy(dialog);
  
  #if 0
  label = gtk_label_new (message);
  okay_button = gtk_button_new_with_label ("Okay");
    
  /* Ensure that the dialog box is destroyed when the user clicks ok. */
    /
  g_signal_connect_swapped (G_OBJECT (okay_button), "clicked",
    G_CALLBACK (gtk_widget_destroy), G_OBJECT (dialog));
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
                     okay_button);

  /* Add the label, and show everything we've added to the dialog. */

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox),
                     label);
  gtk_widget_show_all (dialog);
  #endif
}

/* This function produces a menu bar from a GtkUIManager and ui spec.
	It accepts a GtkUIManager that is assumed to be configured with
	the necessary actions referenced from the ui_xml that is loaded into the
	manager before creation of the menubar. If window is non-NULL then the
	accelerators from the GtkUIManager are loaded into the specified window.
	*/
GtkWidget *
create_menu_bar (GtkUIManager *manager, const gchar *ui_xml, GtkWidget *window)
{
  GError *error = NULL;
  GtkWidget *mbar = NULL;
  
  if (!gtk_ui_manager_add_ui_from_string(manager, ui_xml, -1, &error)) {
	  g_message("building ui failed: %s\n", error->message);
	  g_error_free(error);
  } else {
	  if (window) {
		  gtk_window_add_accel_group(GTK_WINDOW (window), 
				  gtk_ui_manager_get_accel_group(manager));
		  g_object_set_data_full(G_OBJECT(window), "ui-manager", manager, g_object_unref);
	  }
	  mbar = gtk_ui_manager_get_widget(manager, "/menubar");
  }

  return(mbar);
}

void
populate_combo_box(GtkWidget *combo_box, gchar **lbl, gint nitems,
  GCallback func, gpointer obj)
{
  gint i;
  for (i=0; i<nitems; i++) {
	  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), lbl[i]);
  }
  //gtk_combo_box_set_add_tearoffs(GTK_COMBO_BOX(combo_box), true);
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), 0);
  if (func)
	  g_signal_connect(G_OBJECT(combo_box), "changed", G_CALLBACK(func), obj);
}

/* adds columns to the tree_view labeled by lbl. If headers is true, the headers
	are displayed, otherwise they are not and the labels are ignored. Columns
	are only added for non-NULL labels if headers is true. The callback
	is connected to the 'changed' signal of the associated GtkTreeSelection. 
	The columns render text from the corresponding model columns. */
void
populate_tree_view(GtkWidget *tree_view, gchar **lbl, gint nitems, gboolean headers,
	GtkSelectionMode mode, GCallback func, gpointer obj)
{
	gint i;
	GtkTreeSelection *sel;
	
	for (i=0; i<nitems; i++) {
		if (!headers || lbl[i]) {
			GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
				headers ? lbl[i] : NULL, gtk_cell_renderer_text_new(), "text", i, NULL);
			gtk_tree_view_column_set_sort_column_id(col, i);
			gtk_tree_view_column_set_resizable(col, true);
			gtk_tree_view_insert_column(GTK_TREE_VIEW(tree_view), col, -1);
		}
	}
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), headers);
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	gtk_tree_selection_set_mode(sel, mode);
	if (func)
		g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(func), obj);
}

/* this utility disabled, because it is no longer necessary since accelerators
	don't work on toplevel menus in GTK2 (just use a mnemonic)
*/
#if 0
GtkWidget *
submenu_make (gchar *lbl, guint key, GtkAccelGroup *accel_group) {
  GtkWidget *item;
  gint tmp_key;

  /* This gets me the underline, but the accelerator doesn't always work */
  item = gtk_menu_item_new_with_mnemonic (lbl);
  
  gtk_widget_show (item);
  return item;
}
#endif

/* this is no longer necessary given GTK2 GtkMenuShell's functionality */
#if 0
void
submenu_insert (GtkWidget *item, GtkWidget * mbar, gint pos) {

  if (pos == -1) {  /*-- append at the end? --*/
    GSList *children;
    children = (GSList *) gtk_container_get_children (GTK_CONTAINER (mbar));
    pos = g_slist_length (children) - 1;
    g_slist_free (children);
  }

  gtk_menu_shell_insert (GTK_MENU_SHELL (mbar), item, pos);
}

void
submenu_append (GtkWidget *item, GtkWidget * mbar) {
  gint pos;
  GSList *children;

  children = (GSList *) gtk_container_get_children (GTK_CONTAINER (mbar));
  pos = g_slist_length (children);
  gtk_menu_bar_insert (GTK_MENU_BAR (mbar), item, pos);
}
#endif

/* this stuff is no longer used */
#if 0
void
submenu_destroy (GtkWidget *item)
{ 
  if (item != NULL && GTK_IS_WIDGET(item)) {
    if (GTK_IS_MENU_ITEM(item)) {
      GtkMenuItem *menu_item = GTK_MENU_ITEM (item);
      if (menu_item->submenu)
        gtk_widget_destroy (menu_item->submenu);
    }
    gtk_widget_destroy (item);
  }
}

void
position_popup_menu (GtkMenu *menu, gint *px, gint *py, gpointer data)
{
  gint w, h;
  GtkWidget *top = (GtkWidget *)
    g_object_get_data(G_OBJECT (menu), "top");

  gdk_window_get_size (top->window, &w, &h);
  gdk_window_get_origin (top->window, px, py);

  *py += h;
}
#endif

void scale_set_default_values (GtkScale *scale)
{
  gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_draw_value (scale, false);
}

/*--------------------------------------------------------------------*/
/*      Notebook containing the variable list for each datad          */
/*--------------------------------------------------------------------*/

enum { VARLIST_NAME, VARLIST_INDEX, VARLIST_NCOLS };

void
variable_notebook_subwindow_add (datad *d, GCallback func,
  GtkWidget *notebook, vartyped vtype, datatyped dtype, ggobid *gg)
{
  GtkWidget *swin, *tree_view;
  GtkListStore *model;
  GtkTreeIter iter;
  gint j;
  vartabled *vt;
  GtkSelectionMode mode = (GtkSelectionMode)
          g_object_get_data(G_OBJECT(notebook), "SELECTION");

  if (d->ncols == 0)
    return;

  if (vtype == categorical) {
    /* is there in fact a categorical variable? */
    gboolean categorical_variable_present = false;
    for (j=0; j<g_slist_length (d->vartable); j++) {
      vt = (vartabled *) g_slist_nth_data (d->vartable, j);
      if (vt->vartype == categorical) {
        categorical_variable_present = true;
        break;
      }
    }
    if (!categorical_variable_present)
      return;
  }

  /* Create a scrolled window to pack the tree view widget into */
  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  g_object_set_data(G_OBJECT(swin), "datad", d);  /*setdata*/
/*
 * name or nickname?  Which one we'd prefer to use depends on the
 * size of the space we're working in -- maybe this will become an
 * argument.
*/
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), swin,
    (d->nickname != NULL) ?
      gtk_label_new (d->nickname) : gtk_label_new (d->name)); 

  /* add the tree view */
  model = gtk_list_store_new(VARLIST_NCOLS, G_TYPE_STRING, G_TYPE_INT);
  tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
  gtk_widget_set_size_request(tree_view, -1, 75);
  g_object_set_data(G_OBJECT (tree_view), "datad", d);
  populate_tree_view(tree_view, NULL, 1, false, mode, func, gg);
  gtk_tree_view_column_set_spacing(gtk_tree_view_get_column(GTK_TREE_VIEW(tree_view), 0), 0);
  //if(func)
  //   g_signal_connect (G_OBJECT (tree_view), "select_row", G_CALLBACK (func), gg);

  for (j=0; j<d->ncols; j++) {
    vt = vartable_element_get (j, d);
    if (vtype == all_vartypes ||
        (vtype == categorical && vt->vartype == categorical) ||
        (vtype == integer && vt->vartype == integer) ||
        (vtype == real && vt->vartype == real))
    {
      gtk_list_store_append(model, &iter);
	  gtk_list_store_set(model, &iter, 
	  	VARLIST_NAME, vt->collab_tform, 
	  	VARLIST_INDEX, j, -1);
    }
  }

  gtk_container_add (GTK_CONTAINER (swin), tree_view);
  gtk_widget_show_all (swin);
}

static void 
variable_notebook_adddata_cb (ggobid *gg, datad *d, void *notebook)
{
  GtkSignalFunc func = NULL;
  vartyped vtype;
  datatyped dtype;

  vtype = (vartyped) g_object_get_data(G_OBJECT(notebook), "vartype");
  dtype = (vartyped) g_object_get_data(G_OBJECT(notebook), "datatype");

  if ((dtype == all_datatypes) ||
      (dtype == no_edgesets && d->edge.n == 0) ||
      (dtype == edgesets_only && d->edge.n > 0))
  {
    if (g_slist_length (d->vartable)) {
      variable_notebook_subwindow_add (d, func, notebook, vtype, dtype, gg);
    }
  }

  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (GTK_OBJECT(notebook)),
                              g_slist_length (gg->d) > 1);
}

GtkWidget *
get_tree_view_from_object (GObject *obj)
{
  GtkWidget *notebook = NULL, *swin = NULL, *tree_view = NULL;
  gint page;

  if (obj != NULL) {

    /*-- find the current notebook page, then get the current tree_view --*/
    notebook = (GtkWidget *) g_object_get_data (obj, "notebook");
    if (notebook && GTK_IS_NOTEBOOK(notebook)) {
      page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
      swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page);
      if (swin) {
        tree_view = GTK_BIN (swin)->child;
      }
    }
  }

  return tree_view;
}
gint  /*-- assumes GTK_SELECTION_SINGLE --*/
get_one_selection_from_tree_view (GtkWidget *tree_view, datad *d)
{
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint selected_var = -1;
	
	gboolean selected = gtk_tree_selection_get_selected(sel, &model, &iter);
	if (selected)
		gtk_tree_model_get(model, &iter, VARLIST_INDEX, &selected_var, -1);

	return selected_var;
}
/** returns the dataset indices of the view's selected variables */
gint * /*-- assumes multiple selection is possible --*/
get_selections_from_tree_view (GtkWidget *tree_view, gint *nvars)
{
  GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
  GtkTreeModel *model;
  GtkTreeIter iter;
  GList *rows, *l;
  gint selected_var, i;
  gint *vars;
  
  rows = gtk_tree_selection_get_selected_rows(sel, &model);
  *nvars = g_list_length(rows);
  vars = g_new(gint, *nvars);
  
  for (l = rows, i = 0; l; l = l->next, i++) {
	  GtkTreePath *path = (GtkTreePath*)l->data;
	  gtk_tree_model_get_iter(model, &iter, path);
	  gtk_tree_model_get(model, &iter, VARLIST_INDEX, &selected_var, -1);
	  vars[i] = selected_var;
	  gtk_tree_path_free(path);
  }
  g_list_free(rows);
  
  return vars;
}

void
select_tree_view_row(GtkWidget *tree_view, gint row)
{
	GtkTreeSelection *tree_sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	GtkTreePath *path = gtk_tree_path_new_from_indices(row, -1);
	gtk_tree_selection_select_path(tree_sel, path);
	gtk_tree_path_free(path);
}

/** gets the selected row index from a GtkTreeSelection in 'single' mode.
	if the model is a GtkTreeModelSort, it will get the row index in the child model
	note: only works for flat views
*/
gint
tree_selection_get_selected_row(GtkTreeSelection *tree_sel)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  GtkTreePath *path, *child_path;
  gint row = -1;
  gboolean selected;
  
  selected = gtk_tree_selection_get_selected(tree_sel, &model, &iter);
  
  if (selected) {
	  path = gtk_tree_model_get_path(model, &iter);
	  if (GTK_IS_TREE_MODEL_SORT(model)) {
		  child_path = gtk_tree_model_sort_convert_path_to_child_path(
		  	GTK_TREE_MODEL_SORT(model), path);
		  gtk_tree_path_free(path);
		  path = child_path;
	  }
	  row = gtk_tree_path_get_indices(path)[0];
	  gtk_tree_path_free(path);
  }
  
  return(row);
}
/*-------------------------------------------------------------------------*/


/*
* Notice that this callback could be used to respond to any
* change in the variable list, because it doesn't count the
* number of variables; it just clears the list and then
* rebuilds it.
*/
void 
variable_notebook_varchange_cb (ggobid *gg, vartabled *vt, gint which,
  datad *data, void *notebook)
{
  GtkWidget *swin, *tree_view;

  /*-- add one or more variables to this datad --*/
  datad *d = (datad *) datad_get_from_notebook (GTK_WIDGET(notebook), gg);
  gint kd = g_slist_index (gg->d, d);

  /*-- get the tree_view associated with this data; clear and rebuild --*/
  swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (GTK_WIDGET(notebook)), kd);
  if (swin) {
    gint j;
    vartabled *vt;
    GtkTreeModel *model;
	GtkTreeIter iter;
	tree_view = GTK_BIN (swin)->child;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
    
	gtk_list_store_clear(GTK_LIST_STORE(model));
    for (j=0; j<d->ncols; j++) {
      vt = vartable_element_get (j, d);
      if (vt) {
        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
	  		VARLIST_NAME, vt->collab_tform, 
			VARLIST_INDEX, j, -1);
      }
    }
  }
}


void 
variable_notebook_list_changed_cb(ggobid *gg, datad *d, void *notebook)
{
  variable_notebook_varchange_cb(gg, NULL, -1, d, notebook);
}

CHECK_EVENT_SIGNATURE(variable_notebook_adddata_cb, datad_added_f)
CHECK_EVENT_SIGNATURE(variable_notebook_varchange_cb, variable_added_f)
CHECK_EVENT_SIGNATURE(variable_notebook_list_changed_cb, variable_list_changed_f)

GtkWidget *
create_variable_notebook (GtkWidget *box, GtkSelectionMode mode, 
  vartyped vtype, datatyped dtype, GtkSignalFunc func, ggobid *gg)
{
  GtkWidget *notebook;
  gint nd = g_slist_length (gg->d);
  GSList *l;
  datad *d;

  /* Create a notebook, set the position of the tabs */
  notebook = gtk_notebook_new ();
  /* gtk_notebook_set_homogeneous_tabs (GTK_NOTEBOOK (notebook), true); */
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), nd > 1);
  gtk_box_pack_start (GTK_BOX (box), notebook, true, true, 2);
  g_object_set_data(G_OBJECT(notebook), "SELECTION", (gpointer) mode);
  g_object_set_data(G_OBJECT(notebook), "vartype", (gpointer) vtype);
  g_object_set_data(G_OBJECT(notebook), "datatype", (gpointer) dtype);

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    if ((dtype == all_datatypes) ||
        (dtype == no_edgesets && d->edge.n == 0) ||
        (dtype == edgesets_only && d->edge.n > 0))
    {
      if (g_slist_length (d->vartable)) {
        variable_notebook_subwindow_add (d, func, notebook, vtype, dtype, gg);
      }
    }
  }

  /*-- listen for variable_added and _list_changed events on main_window --*/
  /*--   ... list_changed would be enough --*/
  g_signal_connect (G_OBJECT (gg),
		      "variable_added", 
		      G_CALLBACK (variable_notebook_varchange_cb),
		      GTK_OBJECT (notebook));
  g_signal_connect (G_OBJECT (gg),
		      "variable_list_changed", 
		      G_CALLBACK (variable_notebook_varchange_cb),
		      GTK_OBJECT (notebook));

  /*-- listen for datad_added events on main_window --*/
  g_signal_connect (G_OBJECT (gg),
		      "datad_added", 
		      G_CALLBACK (variable_notebook_adddata_cb),
		      GTK_OBJECT (notebook));

  return notebook;
}

/*--------------------------------------------------------------------*/
/* These are for the benefit of plugins, though they might have other */
/* uses as well       													*/
/* - might be nice to move to a GtkUIManager paradigm here... mfl 	*/
/*--------------------------------------------------------------------*/

GtkWidget *
GGobi_addDisplayMenuItem (const gchar *label, ggobid *gg)
{
  GtkWidget *entry = NULL;

  GtkWidget *dpy_menu = gg->display_menu;  /*-- this is null --*/
  datad *data;

  if (dpy_menu != NULL) {
    entry = gtk_menu_item_new_with_mnemonic (label);
    data = GGobi_data_get(0, gg);
    g_object_set_data(G_OBJECT(entry), "data", (gpointer) data);

    gtk_widget_show (entry);

    /* Add a separator */
    CreateMenuItem (dpy_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

    gtk_menu_shell_append (GTK_MENU_SHELL (dpy_menu), entry);
  }

  return(entry);
}


gboolean
GGobi_addToolsMenuWidget(GtkWidget *entry, ggobid *gg)
{
  GtkWidget *tools_menu = NULL, *tools_item = NULL;
  GtkUIManager *manager;

  manager = gg->main_menu_manager; /* gtk_item_factory_from_path ("<main>"); */
  tools_item = gtk_ui_manager_get_widget (manager, "/menubar/Tools");
  
  if (tools_item)
	  tools_menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(tools_item));
  
  if (tools_menu) {
      gtk_menu_shell_append (GTK_MENU_SHELL (tools_menu), entry);
  } else
      return(false);

  return(true);
}

GtkWidget *
GGobi_addToolsMenuItem (gchar *lbl, ggobid *gg)
{
  GtkWidget *entry;
  if(!lbl) {
      return(NULL);
  }

    /*-- purify goes crazy here, and I have no idea why -- dfs --*/
  entry = gtk_menu_item_new_with_mnemonic (lbl);
  if(GGobi_addToolsMenuWidget(entry, gg) == false) {
      gtk_widget_destroy(entry);
  } else
      gtk_widget_show (entry);

  return (entry);
}

