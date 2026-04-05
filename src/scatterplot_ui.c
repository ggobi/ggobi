/* scatterplot_ui.c */
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

/*--------------------------------------------------------------------*/
/*                   Setting the display menubar                      */
/*--------------------------------------------------------------------*/

static GtkWidget *
scatterplot_edges_menu_ensure (displayd *display)
{
  GtkWidget *item = widget_find_by_name (display->menubar,
                                         "DISPLAY:edges_topmenu");
  GtkWidget *menu;

  if (item != NULL)
    return gtk_menu_item_get_submenu (GTK_MENU_ITEM (item));

  item = gtk_menu_item_new_with_mnemonic ("_Edges");
  gtk_widget_set_name (item, "DISPLAY:edges_topmenu");
  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
  gtk_menu_shell_prepend (GTK_MENU_SHELL (display->menubar), item);

  return menu;
}

static void
scatterplot_edge_option_toggled_cb (GtkCheckMenuItem *item, displayd *display)
{
  gint option;

  if (!gtk_check_menu_item_get_active (item))
    return;

  option = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item), "edge-option"));
  set_display_option (true, option, display);
}

static GtkWidget *
scatterplot_edge_radio_item_new (GtkWidget *menu, GSList **group,
                                 const gchar *label, const gchar *name,
                                 const gchar *accel, GtkAccelGroup *accel_group,
                                 gboolean active, gint option,
                                 displayd *display)
{
  GtkWidget *item = gtk_radio_menu_item_new_with_mnemonic (*group, label);
  guint key = 0;
  GdkModifierType modifiers = 0;

  *group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (item));
  gtk_widget_set_name (item, name);
  g_object_set_data (G_OBJECT (item), "edge-option", GINT_TO_POINTER (option));
  g_signal_connect (G_OBJECT (item), "toggled",
                    G_CALLBACK (scatterplot_edge_option_toggled_cb), display);
  if (accel != NULL && accel_group != NULL) {
    gtk_accelerator_parse (accel, &key, &modifiers);
    if (key != 0)
      gtk_widget_add_accelerator (item, "activate", accel_group, key,
                                  modifiers, GTK_ACCEL_VISIBLE);
  }
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), active);

  return item;
}

/*
 * This handles the initialization of the edge menu item and menu,
 * and it should also be called whenever the number of edge sets
 * might have changed:  when an edge set is added or removed, and
 * when a datad is added or removed.
*/
void
scatterplot_display_edge_menu_update (displayd * display, ggobid * gg)
{
  GGobiData *d = display->d;  /*-- this dataset --*/
  gint nd = g_slist_length (gg->d);
  GGobiData *e;
  gint k, ne = 0;
  GtkWidget *edges_menu, *edges_item, *attach_item = NULL, *attach_menu = NULL;
  GtkAccelGroup *accel_group;
  GSList *attach_group = NULL;

  /*-- If this datad has ids, find the number of other datad's with
       edges --*/

  if (d->rowIds) {
    endpointsd *endpoints;
    for (k = 0; k < nd; k++) {
      e = (GGobiData *) g_slist_nth_data (gg->d, k);
      if (e->edge.n > 0) {
        endpoints = resolveEdgePoints (e, d);
        if (endpoints != NULL) {
          ne++;
        }
      }
    }
  }

  if (display->menubar == NULL)
    return;

  edges_menu = scatterplot_edges_menu_ensure (display);
  edges_item = widget_find_by_name (display->menubar, "DISPLAY:edges_topmenu");
  accel_group = display_menu_accel_group_get (display);
  display_menu_clear (edges_menu);

  /*-- then build the new menu if appropriate --*/
  if (ne == 0) {
    gtk_widget_hide (edges_item);
    return;
  }

  gtk_widget_show (edges_item);
  if (ne > 1) {
    attach_item = gtk_menu_item_new_with_mnemonic ("_Attach edge set");
    gtk_widget_set_name (attach_item, "DISPLAY:edgesets_attach");
    attach_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (attach_item), attach_menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (edges_menu), attach_item);
  }

    /*
       When there's only one edge set, indicate that on the menu
       with a single menu item naming the edge set.  Let it behave
       like the other menu items, too, turning on undirected
       edges.  Selecting an edge set is required.
     */

  for (k = 0; k < nd; k++) {
    e = (GGobiData *) g_slist_nth_data (gg->d, k);
    if (e->edge.n > 0 && resolveEdgePoints (e, d) != NULL) {
      GtkWidget *item;
      gchar *lbl;

      if (ne == 1) {
        lbl = g_strdup_printf ("Attach edge set (%s)", e->name);
        item = gtk_menu_item_new_with_label (lbl);
        gtk_widget_set_name (item, "DISPLAY:edges_attach_single");
        gtk_menu_shell_append (GTK_MENU_SHELL (edges_menu), item);
      }
      else {
        lbl = ggobi_data_get_name (e);
        item = gtk_radio_menu_item_new_with_label (attach_group, lbl);
        attach_group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (item));
        gtk_menu_shell_append (GTK_MENU_SHELL (attach_menu), item);
        if (e == display->e)
          gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), true);
      }
      g_object_set_data (G_OBJECT (item), "display", display);
      g_signal_connect (G_OBJECT (item), "activate",
                        G_CALLBACK (edgeset_add_cb), e);
      g_free (lbl);
    }
  }

  if (display->e != NULL) {
    GSList *group = NULL;
    gboolean any_edges =
      display->options.edges_undirected_show_p ||
      display->options.edges_directed_show_p ||
      display->options.edges_arrowheads_show_p;

    gtk_menu_shell_append (GTK_MENU_SHELL (edges_menu),
                           gtk_separator_menu_item_new ());
    scatterplot_edge_radio_item_new (edges_menu, &group,
                                     "Show _lines only",
                                     "DISPLAY:show_undirected_edges",
                                     "<control>L", accel_group,
                                     display->options.edges_undirected_show_p,
                                     DOPT_EDGES_U, display);
    scatterplot_edge_radio_item_new (edges_menu, &group,
                                     "Show lines _with arrowheads",
                                     "DISPLAY:show_directed_edges",
                                     "<control>W", accel_group,
                                     display->options.edges_directed_show_p,
                                     DOPT_EDGES_D, display);
    scatterplot_edge_radio_item_new (edges_menu, &group,
                                     "Show arrowheads _only",
                                     "DISPLAY:show_arrowheads_only",
                                     "<control>O", accel_group,
                                     display->options.edges_arrowheads_show_p,
                                     DOPT_EDGES_A, display);
    scatterplot_edge_radio_item_new (edges_menu, &group,
                                     "_Hide edges",
                                     "DISPLAY:hide_edges",
                                     "<control>H", accel_group,
                                     !any_edges, DOPT_EDGES_H, display);
  }
}
