/*-- display_ui.c --*/
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Eclipse Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"
#include "plugin.h"

void buildExtendedDisplayMenu (ggobid * gg, int nd, GGobiData * d0);

void
display_set_position (windowDisplayd * display, ggobid * gg)
{
  gint x, y, width, height;
  gint posx, posy;
  GdkWindow *main_window;

  /*-- get the size and position of the gg->main_window) --*/
  main_window = gtk_widget_get_window (gg->main_window);
  gdk_window_get_root_origin (main_window, &x, &y);
  gdk_window_get_geometry (main_window, NULL, NULL, &width, &height);

  gtk_widget_realize (display->window);
  if (x == 0 && y == 0) {
                       /*-- can't get any info for the first display --*/
    ggobi_get_primary_monitor_size (&posx, &posy);
    posx /= 4;
    posy /= 4;
  }
  else {
    posx = x + (3 * width) / 4;
    posy = y + (3 * height) / 4;
  }
  gtk_window_move (GTK_WINDOW (display->window), posx, posy);
}

void
display_menu_build (ggobid * gg)
{
  gint nd;
  GGobiData *d0;
  GtkWidget *item;
  if (gg == NULL || gg->d == NULL)
    return;

  nd = ndatad_with_vars_get (gg);

  d0 = (GGobiData *) gg->d->data;
  if (gg->display_menu != NULL)
    gtk_widget_destroy (gg->display_menu);

  if (nd > 0) {
    gg->display_menu = gtk_menu_new ();

    if (g_slist_length (ExtendedDisplayTypes)) {
      buildExtendedDisplayMenu (gg, nd, d0);
    }
  }

  /* Experiment: move the DisplayTree to the Display menu -- dfs */
  /* Add a separator before the mode-specific items */
  CreateMenuItem (gg->display_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  item = gtk_menu_item_new_with_label ("Show Display Tree");
  g_signal_connect (G_OBJECT (item), "activate",
                    G_CALLBACK (show_display_tree_cb), (gpointer) gg);
  gtk_menu_shell_append (GTK_MENU_SHELL (gg->display_menu), item);

  if (sessionOptions->info != NULL) {
    pluginsUpdateDisplayMenu (gg, gg->pluginInstances);
  }

  /*-- these two lines replace gtk_menu_popup --*/
  if (nd) {
    GtkWidget *display_item =
      widget_find_by_name (gg->main_menubar, "MAIN:display_topmenu");

    gtk_widget_show_all (gg->display_menu);
    if (display_item != NULL)
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (display_item),
                                 gg->display_menu);
  }
}

void
display_menu_init (ggobid * gg)
{                               /*
                                   gg->display_menu_item = submenu_make ("_Display", 'D',
                                   gg->main_accel_group);

                                   gtk_widget_show (gg->display_menu_item);

                                   submenu_insert (gg->display_menu_item, gg->main_menubar, 1);
                                 */
}


typedef struct
{
  GGobiExtendedDisplayClass *klass;
  GGobiData *d;
} ExtendedDisplayCreateData;

static void
extended_display_open_cb (GtkWidget * w, ExtendedDisplayCreateData * data)
{
  ggobid *gg = data->d->gg;
  displayd *dpy;

  if (data->d->nrows == 0)
    return;

  splot_set_current (gg->current_splot, off, gg);
  if (data->klass->create) {
    dpy = data->klass->create (true, false, NULL, data->d, gg);
  }
  else if (data->klass->createWithVars) {
    gint *selected_vars, nselected_vars = 0;

    selected_vars = (gint *) g_malloc (data->d->ncols * sizeof (gint));
    nselected_vars = selected_cols_get (selected_vars, data->d, gg);
    dpy =
      data->klass->createWithVars (true, false, nselected_vars, selected_vars,
                                   data->d, gg);
    g_free (selected_vars);
  }
  else {
    /* How to get the name of the class from the class! GTK_OBJECT_CLASS(gtk_type_name(data->klass)->type) 
       Close.. */
    g_printerr
      ("Real problems! An extended display (%s) without a create routine!\n",
       g_type_name (G_TYPE_FROM_CLASS (data->klass)));
    return;
  }

  if (!dpy) {
    g_printerr ("Failed to create display of type %s\n",
                data->klass->titleLabel);
    return;
  }


  display_add (dpy, gg);
  varpanel_refresh (dpy, gg);
}

void
buildExtendedDisplayMenu (ggobid * gg, gint nd, GGobiData * d0)
{
  gchar label[200], *lbl;
  GGobiExtendedDisplayClass *klass;
  GSList *el = ExtendedDisplayTypes;
  const gchar *desc;
  GtkWidget *item, *submenu, *anchor;
  gint k;
  ExtendedDisplayCreateData *cbdata;
  while (el) {
    klass = GGOBI_EXTENDED_DISPLAY_CLASS (el->data);
    desc = klass->titleLabel;
    sprintf (label, "New %s", desc);

    if (nd == 1) {
      cbdata = (ExtendedDisplayCreateData *)
        g_malloc (sizeof (ExtendedDisplayCreateData));
      cbdata->d = d0;
      cbdata->klass = klass;

      item = CreateMenuItem (gg->display_menu, label,
                             NULL, NULL, gg->main_menubar,
                             gg->main_accel_group,
                             G_CALLBACK (extended_display_open_cb),
                             (gpointer) cbdata, gg);
      g_object_set_data (G_OBJECT (item), "missing_p", GINT_TO_POINTER (0));
    }
    else {
      submenu = gtk_menu_new ();
      anchor = CreateMenuItem (gg->display_menu, label,
                               NULL, NULL, gg->main_menubar, NULL, NULL, NULL,
                               NULL);

      k = 0;
      for (k = 0; k < g_slist_length (gg->d); k++) {
        GGobiData *d = (GGobiData *) g_slist_nth_data (gg->d, k);

        /*-- add an item for each datad with variables --*/
        if (g_slist_length (d->vartable) > 0) {
          lbl = ggobi_data_get_name (d);
          cbdata = (ExtendedDisplayCreateData *)
            g_malloc (sizeof (ExtendedDisplayCreateData));
          cbdata->d = d;
          cbdata->klass = klass;
          item = CreateMenuItem (submenu, lbl,
                                 NULL, NULL, gg->display_menu,
                                 gg->main_accel_group,
                                 G_CALLBACK (extended_display_open_cb),
                                 cbdata, gg);

          g_object_set_data (G_OBJECT (item),
                             "displaytype", (gpointer) klass);
          g_object_set_data (G_OBJECT (item),
                             "missing_p", GINT_TO_POINTER (0));
          g_free (lbl);
        }
      }
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (anchor), submenu);
    }

    el = el->next;
  }
}
static void
display_close_menu_cb (GtkWidget *item, displayd *display)
{
  display_close (display);
}

static void
display_option_menu_toggled_cb (GtkCheckMenuItem *item, displayd *display)
{
  guint option = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (item),
                                                      "display-option"));
  set_display_option (gtk_check_menu_item_get_active (item), option, display);
}

GtkAccelGroup *
display_menu_accel_group_get (displayd *display)
{
  if (display == NULL || display->menubar == NULL)
    return NULL;

  return g_object_get_data (G_OBJECT (display->menubar), "DISPLAY:accel_group");
}

void
display_menu_clear (GtkWidget *menu)
{
  GList *children, *l;

  children = gtk_container_get_children (GTK_CONTAINER (menu));
  for (l = children; l != NULL; l = l->next)
    gtk_widget_destroy (GTK_WIDGET (l->data));
  g_list_free (children);
}

GtkWidget *
display_menu_ensure (displayd *display, const gchar *label, const gchar *name)
{
  GtkWidget *item, *menu;

  item = widget_find_by_name (display->menubar, name);
  if (item != NULL)
    return gtk_menu_item_get_submenu (GTK_MENU_ITEM (item));

  item = gtk_menu_item_new_with_mnemonic (label);
  gtk_widget_set_name (item, name);
  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (display->menubar), item);
  gtk_widget_show (item);

  return menu;
}

GtkWidget *
display_menu_append_item (GtkWidget *menu, const gchar *label,
                          const gchar *name, const gchar *tooltip,
                          const gchar *accel, GCallback func, gpointer data,
                          GtkAccelGroup *accel_group)
{
  GtkWidget *item = gtk_menu_item_new_with_mnemonic (label);
  guint key = 0;
  GdkModifierType modifiers = 0;

  if (name != NULL)
    gtk_widget_set_name (item, name);
  if (tooltip != NULL)
    gtk_widget_set_tooltip_text (item, tooltip);
  if (func != NULL)
    g_signal_connect (G_OBJECT (item), "activate", func, data);
  if (accel != NULL && accel_group != NULL) {
    gtk_accelerator_parse (accel, &key, &modifiers);
    if (key != 0)
      gtk_widget_add_accelerator (item, "activate", accel_group, key,
                                  modifiers, GTK_ACCEL_VISIBLE);
  }

  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  gtk_widget_show (item);

  return item;
}

GtkWidget *
display_menu_append_toggle_item (GtkWidget *menu, const gchar *label,
                                 const gchar *name, const gchar *tooltip,
                                 const gchar *accel, gboolean active,
                                 GCallback func, gpointer data,
                                 GtkAccelGroup *accel_group)
{
  GtkWidget *item = gtk_check_menu_item_new_with_mnemonic (label);
  guint key = 0;
  GdkModifierType modifiers = 0;

  if (name != NULL)
    gtk_widget_set_name (item, name);
  if (tooltip != NULL)
    gtk_widget_set_tooltip_text (item, tooltip);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), active);
  if (func != NULL)
    g_signal_connect (G_OBJECT (item), "toggled", func, data);
  if (accel != NULL && accel_group != NULL) {
    gtk_accelerator_parse (accel, &key, &modifiers);
    if (key != 0)
      gtk_widget_add_accelerator (item, "activate", accel_group, key,
                                  modifiers, GTK_ACCEL_VISIBLE);
  }

  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  gtk_widget_show (item);

  return item;
}

GtkWidget *
display_menu_append_display_option_item (GtkWidget *menu, guint option,
                                         displayd *display)
{
  const gchar *label = NULL, *name = NULL, *tooltip = NULL, *accel = NULL;
  gboolean active = false;
  GtkWidget *item;

  switch (option) {
  case DOPT_POINTS:
    label = "Show P_oints";
    name = "DISPLAY:show_points";
    tooltip = "Toggle visibility of points on this display";
    accel = "<control>O";
    active = display->options.points_show_p;
    break;
  case DOPT_WHISKERS:
    label = "Show _Lines";
    name = "DISPLAY:show_lines";
    tooltip = "Toggle visibility of lines on this display";
    accel = "<control>L";
    active = display->options.whiskers_show_p;
    break;
  case DOPT_AXES:
    label = "Show _Axes";
    name = "DISPLAY:show_axes";
    tooltip = "Toggle visibility of axes on this display";
    accel = "<control>A";
    active = display->options.axes_show_p;
    break;
  case DOPT_AXESLAB:
    label = "Show Axes _Labels";
    name = "DISPLAY:show_axes_labels";
    tooltip = "Toggle display of the axes labels";
    accel = "<control>L";
    active = display->options.axes_label_p;
    break;
  case DOPT_AXESVALS:
    label = "Show Projection _Vals";
    name = "DISPLAY:show_axes_vals";
    tooltip = "Toggle display of the projection values";
    accel = "<control>V";
    active = display->options.axes_values_p;
    break;
  default:
    g_return_val_if_reached (NULL);
  }

  item = display_menu_append_toggle_item (menu, label, name, tooltip, accel,
                                          active,
                                          G_CALLBACK (display_option_menu_toggled_cb),
                                          display,
                                          display_menu_accel_group_get (display));
  g_object_set_data (G_OBJECT (item), "display-option",
                     GUINT_TO_POINTER (option));

  return item;
}

GtkWidget *
display_menu_bar_create (displayd *display, GtkWidget *window)
{
  GtkWidget *menubar;
  GtkWidget *file_menu;
  GtkAccelGroup *accel_group = NULL;

  menubar = gtk_menu_bar_new ();
  if (window != NULL) {
    accel_group = ggobi_window_add_accel_group (window);
    g_object_set_data (G_OBJECT (menubar), "DISPLAY:accel_group", accel_group);
  }

  display->menubar = menubar;
  file_menu = display_menu_ensure (display, "_File", "DISPLAY:file_topmenu");
  display_menu_append_item (file_menu, "_Close", "DISPLAY:close_display",
                            "Close this display", "<control>C",
                            G_CALLBACK (display_close_menu_cb), display,
                            accel_group);

  return menubar;
}
