/*-- utils_ui.c --*/
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
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

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
  GtkWidget *menuitem;

  /* --- If there's a name, create the item and add the signal handler --- */
  if (szName && strlen (szName)) {
    menuitem = gtk_menu_item_new_with_label (szName);
    if(func)
      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			  GTK_SIGNAL_FUNC (func), data);

    GGobi_widget_set (GTK_WIDGET (menuitem), gg,  true);

  } else {
    /* --- Create a separator --- */
    menuitem = gtk_menu_item_new ();
  }

  /* --- Add menu item to the menu and show it. --- */
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);

  if (szName && szAccel && accel_group == NULL) {
/*
    accel_group = gtk_accel_group_new ();
    gtk_accel_group_attach (accel_group, GTK_OBJECT (win_main));
*/
  }

  /* --- If there was an accelerator --- */
  if (szAccel && accel_group) {
    if (szAccel[0] == '^') {  /* control-keypress */
      gtk_widget_add_accelerator (menuitem, "activate", accel_group,
        szAccel[1], GDK_CONTROL_MASK,
        GTK_ACCEL_VISIBLE);
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
    gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (menuitem), true);

    GGobi_widget_set(GTK_WIDGET(menuitem), gg, true);

    /* --- set its state --- */
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem), state);

    /* --- Add it to the menu --- */
    gtk_menu_append (GTK_MENU (menu), menuitem);
    gtk_widget_show (menuitem);

    /* --- Listen for "toggled" messages --- */
    gtk_signal_connect (GTK_OBJECT (menuitem), "toggled",
                        GTK_SIGNAL_FUNC (func), data);

    return (menuitem);
}

/*
 * Function to open a dialog box displaying the message provided.
 * (Taken from the gtk documentation)
*/

void quick_message (const gchar * const message, gboolean modal) {

  GtkWidget *dialog, *label, *okay_button;
    
  /* Create the widgets */
    
  dialog = gtk_dialog_new ();

  if (modal)
    gtk_window_set_modal (GTK_WINDOW (dialog), true);

  label = gtk_label_new (message);
  okay_button = gtk_button_new_with_label ("Okay");
    
  /* Ensure that the dialog box is destroyed when the user clicks ok. */
    
  gtk_signal_connect_object (GTK_OBJECT (okay_button), "clicked",
    GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (dialog));
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
                     okay_button);

  /* Add the label, and show everything we've added to the dialog. */

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox),
                     label);
  gtk_widget_show_all (dialog);
}


GtkItemFactory *
get_main_menu (const GtkItemFactoryEntry menu_items [],
               gint nmenu_items,
               GtkAccelGroup *accel_group,
               GtkWidget  *window,
               GtkWidget **mbar,
               gpointer cbdata)
{
  GtkItemFactory *item_factory;

  /* This function initializes the item factory.
     Param 1: The type of menu - can be GTK_TYPE_MENU_BAR, GTK_TYPE_MENU,
              or GTK_TYPE_OPTION_MENU.
     Param 2: The path of the menu.
     Param 3: A pointer to a gtk_accel_group.  The item factory sets up
              the accelerator table while generating menus.
  */

  item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
                                       accel_group);

  /* This function generates the menu items. Pass the item factory,
     the number of items in the array, the array itself, and any
     callback data for the menu items. */
  gtk_item_factory_create_items (item_factory, nmenu_items,
    (GtkItemFactoryEntry *) menu_items, cbdata);

  /* Attach the new accelerator group to the window. */
  gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

  if (mbar)
    /* Finally, return the actual menu bar created by the item factory. */
    *mbar = gtk_item_factory_get_widget (item_factory, "<main>");

  return(item_factory);
}

void
populate_option_menu (GtkWidget *opt_menu, gchar **lbl, gint nitems,
  GtkSignalFunc func, gchar *key, gpointer obj)
{
  GtkWidget *menu = gtk_menu_new ();
  GtkWidget *menuitem;
  gint i;

  for (i=0; i<nitems; i++) {

    menuitem = gtk_menu_item_new_with_label (lbl[i]);
    gtk_menu_append (GTK_MENU (menu), menuitem);
    gtk_widget_show (menuitem) ;

    gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
      GTK_SIGNAL_FUNC (func), GINT_TO_POINTER (i));

    gtk_object_set_data (GTK_OBJECT(menuitem), key, obj);
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_menu), menu);
}


GtkWidget *
submenu_make (gchar *lbl, guint key, GtkAccelGroup *accel_group) {
  GtkWidget *item;
  gint tmp_key;

  /* This gets me the underline, but the accelerator doesn't always work */
  item = gtk_menu_item_new_with_label (lbl);
  tmp_key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child), lbl);

  /*-- V -> ViewMode works but O -> Options doesn't --*/
#ifndef GTK_2_0
  gtk_widget_add_accelerator (item,
    "activate_item", accel_group, tmp_key, GDK_MOD1_MASK, GTK_ACCEL_LOCKED);
#endif

  gtk_widget_show (item);
  return item;
}

void
submenu_insert (GtkWidget *item, GtkWidget * mbar, gint pos) {

  if (pos == -1) {  /*-- append at the end? --*/
    GSList *children;
    children = (GSList *) gtk_container_children (GTK_CONTAINER (mbar));
    pos = g_slist_length (children) - 1;
    g_slist_free (children);
  }

  gtk_menu_bar_insert (GTK_MENU_BAR (mbar), item, pos);
}

void
submenu_append (GtkWidget *item, GtkWidget * mbar) {
  gint pos;
  GSList *children;

  children = (GSList *) gtk_container_children (GTK_CONTAINER (mbar));
  pos = g_slist_length (children);
  gtk_menu_bar_insert (GTK_MENU_BAR (mbar), item, pos);
}

void
submenu_destroy (GtkWidget *item)
{
  if (item != NULL) {
    gtk_menu_item_remove_submenu (GTK_MENU_ITEM (item));
    gtk_widget_hide (item);
    gtk_widget_destroy (item);
  }
}

void
position_popup_menu (GtkMenu *menu, gint *px, gint *py, gpointer data)
{
  gint w, h;
  GtkWidget *top = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT (menu), "top");

  gdk_window_get_size (top->window, &w, &h);
  gdk_window_get_origin (top->window, px, py);

  *py += h;
}

void scale_set_default_values (GtkScale *scale)
{
  gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_draw_value (scale, false);
}

/*--------------------------------------------------------------------*/
/*      Notebook containing the variable list for each datad          */
/*--------------------------------------------------------------------*/

void
variable_notebook_subwindow_add (datad *d, GtkSignalFunc func,
  GtkWidget *notebook, vartyped vtype, datatyped dtype, ggobid *gg)
{
  GtkWidget *swin, *clist;
  gint j;
  gchar *row[1];
  vartabled *vt;
  GtkSelectionMode mode = (GtkSelectionMode)
          gtk_object_get_data(GTK_OBJECT(notebook), "SELECTION");

#ifdef GTK_2_0
/* It appears (simple test) that the default mode is GTK_SELECTION_NONE. 
   And then calling gtk_clist_set_selection_mode will cause the
   default: case to assert a failure.
   In Gtk 1.2, the default value is GTK_SELECTION_SINGLE (on my machines). */
  if(mode == GTK_SELECTION_NONE)
     mode = GTK_SELECTION_SINGLE;
#endif

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

  /* Create a scrolled window to pack the CList widget into */
  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  gtk_object_set_data(GTK_OBJECT(swin), "datad", d);  /*setdata*/
/*
 * name or nickname?  Which one we'd prefer to use depends on the
 * size of the space we're working in -- maybe this will become an
 * argument.
*/
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), swin,
    (d->nickname != NULL) ?
      gtk_label_new (d->nickname) : gtk_label_new (d->name)); 

  /* add the CList */
  clist = gtk_clist_new (1);
  gtk_clist_set_selection_mode (GTK_CLIST (clist), mode);
  gtk_object_set_data (GTK_OBJECT (clist), "datad", d);
  if(func)
     gtk_signal_connect (GTK_OBJECT (clist), "select_row", GTK_SIGNAL_FUNC (func), gg);

  for (j=0; j<d->ncols; j++) {
    vt = vartable_element_get (j, d);
    if (vtype == all_vartypes ||
        (vtype == categorical && vt->vartype == categorical) ||
        (vtype == integer && vt->vartype == integer) ||
        (vtype == real && vt->vartype == real))
    {
      /*
       * I can't use collab_tform, because then I can't find
       * the variable indices later using only the name.
      */
      row[0] = g_strdup (vt->collab);
      gtk_clist_append (GTK_CLIST (clist), row);
      g_free (row[0]);
    }
  }

  /*-- suggested by Gordon Deane; causes no change under linux --*/
  gtk_clist_set_column_width(GTK_CLIST(clist), 0,
    gtk_clist_optimal_column_width (GTK_CLIST(clist), 0));
  /*--                           --*/

  gtk_container_add (GTK_CONTAINER (swin), clist);
  gtk_widget_show_all (swin);
}

static void 
variable_notebook_adddata_cb (ggobid *gg, datad *d, void *notebook)
{
  GtkSignalFunc func = NULL;
  vartyped vtype;
  datatyped dtype;

  vtype = (vartyped) gtk_object_get_data (GTK_OBJECT(notebook), "vartype");
  dtype = (vartyped) gtk_object_get_data (GTK_OBJECT(notebook), "datatype");

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
get_clist_from_object (GtkObject *obj)
{
  GtkWidget *notebook = NULL, *swin = NULL, *clist = NULL;
  gint page;

  if (obj != NULL) {

    /*-- find the current notebook page, then get the current clist --*/
    notebook = (GtkWidget *) gtk_object_get_data (obj, "notebook");
    if (notebook && GTK_IS_NOTEBOOK(notebook)) {
      page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
      swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page);
      if (swin) {
        clist = GTK_BIN (swin)->child;
      }
    }
  }

  return clist;
}
gint  /*-- assumes GTK_SELECTION_SINGLE --*/
get_one_selection_from_clist (GtkWidget *clist, datad *d)
{
  gint jrow, selected_var = -1;
  gchar *varname;
  if (clist) {
    GList *selection = GTK_CLIST (clist)->selection;
    if (selection) {
      jrow = (gint) selection->data;
      gtk_clist_get_text (GTK_CLIST(clist), jrow, 0, &varname);
      selected_var = vartable_index_get_by_name (varname, d);
    }
  }

  return selected_var;
}
gint /*-- assumes multiple selection is possible --*/
get_selections_from_clist (gint maxnvars, gint *vars, GtkWidget *clist,
  datad *d)
{
  gint nselected_vars = 0;
  GList *l;
  gint jrow, selected_var;
  gchar *varname;

  for (l = GTK_CLIST (clist)->selection; l; l=l->next) {
    jrow = GPOINTER_TO_INT (l->data);
    if (jrow >= maxnvars)  break;

    gtk_clist_get_text (GTK_CLIST(clist), jrow, 0, &varname);
    selected_var = vartable_index_get_by_name (varname, d);

    vars[nselected_vars] = selected_var;
    nselected_vars++;
  }

  return nselected_vars;
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
  GtkWidget *swin, *clist;

  /*-- add one or more variables to this datad --*/
  datad *d = (datad *) datad_get_from_notebook (GTK_WIDGET(notebook), gg);
  gint kd = g_slist_index (gg->d, d);

  /*-- get the clist associated with this data; clear and rebuild --*/
  swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (GTK_WIDGET(notebook)), kd);
  if (swin) {
    gint j;
    gchar *row[1];
    vartabled *vt;
    clist = GTK_BIN (swin)->child;

    gtk_clist_freeze (GTK_CLIST(clist));
    gtk_clist_clear (GTK_CLIST (clist));
    for (j=0; j<d->ncols; j++) {
      vt = vartable_element_get (j, d);
      if (vt) {
        /*
         * I can't use collab_tform, because then I can't find
         * the variable indices later using only the name.
        */
        row[0] = g_strdup_printf (vt->collab);
        gtk_clist_append (GTK_CLIST (clist), row);
      }
    }
    gtk_clist_thaw (GTK_CLIST(clist));
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
  gtk_object_set_data (GTK_OBJECT(notebook), "SELECTION", (gpointer) mode);
  gtk_object_set_data (GTK_OBJECT(notebook), "vartype", (gpointer) vtype);
  gtk_object_set_data (GTK_OBJECT(notebook), "datatype", (gpointer) dtype);

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
  gtk_signal_connect (GTK_OBJECT (gg),
		      "variable_added", 
		      GTK_SIGNAL_FUNC (variable_notebook_varchange_cb),
		      GTK_OBJECT (notebook));
  gtk_signal_connect (GTK_OBJECT (gg),
		      "variable_list_changed", 
		      GTK_SIGNAL_FUNC (variable_notebook_varchange_cb),
		      GTK_OBJECT (notebook));

  /*-- listen for datad_added events on main_window --*/
  gtk_signal_connect (GTK_OBJECT (gg),
		      "datad_added", 
		      GTK_SIGNAL_FUNC (variable_notebook_adddata_cb),
		      GTK_OBJECT (notebook));

  return notebook;
}

/*--------------------------------------------------------------------*/
/* These are for the benefit of plugins, though they might have other */
/* uses as well                                                       */
/*--------------------------------------------------------------------*/

GtkWidget *
GGobi_addDisplayMenuItem (const gchar *label, ggobid *gg)
{
  GtkWidget *entry = NULL;

  GtkWidget *dpy_menu = gg->display_menu;  /*-- this is null --*/
  datad *data;

  if (dpy_menu != NULL) {
    entry = gtk_menu_item_new_with_label (label);
    data = GGobi_data_get(0, gg);
    gtk_object_set_data(GTK_OBJECT(entry), "data", (gpointer) data);

    gtk_widget_show (entry);

    /* Add a separator */
    CreateMenuItem (dpy_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

    gtk_menu_append (GTK_MENU (dpy_menu), entry);
  }

  return(entry);
}

GtkWidget *
GGobi_addToolsMenuItem (gchar *lbl, ggobid *gg)
{
  GtkWidget *entry = NULL, *tools_menu = NULL;
  GtkItemFactory *factory;

  factory = gg->main_menu_factory; /* gtk_item_factory_from_path ("<main>"); */
  tools_menu = gtk_item_factory_get_widget (factory, "<main>/Tools");

  if (tools_menu != NULL) {
    /*-- purify goes crazy here, and I have no idea why -- dfs --*/
    if (lbl != (gchar *)NULL) {
      if (strlen(lbl) > 0) {
        entry = gtk_menu_item_new_with_label (lbl);
        gtk_widget_show (entry);

        gtk_menu_append (GTK_MENU (tools_menu), entry);
      }
    }
  }

  return (entry);
}

