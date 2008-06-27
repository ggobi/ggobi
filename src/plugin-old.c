/* plugin.c */
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

#include "plugin-old.h"
#include "externs.h"

#include <stdio.h>
#include <string.h>
#include "utils_ui.h"

/* This file contains utilities and GUI stuff related to plugins */

// FIXME: there needs to be a signal emitted from GGobiApp when a new GGobi
// is created, so that the plugins can listen to that
// That's pretty much all this function does now.
gboolean
registerPlugin (GGobiSession * gg, GGobiPlugin * plugin)
{
  gboolean ok = true;
  
  if (!g_type_module_use(G_TYPE_MODULE(plugin))) {
    return false;
  }
  
  g_type_module_unuse(G_TYPE_MODULE(plugin));
  
  return (ok);
}

gboolean
registerPlugins (GGobiSession * gg, GList * plugins)
{
  GList *el = plugins;
  gboolean ok = false;
  GGobiPlugin *plugin;

  while (el) {
    plugin = (GGobiPlugin *) el->data;
    ok = registerPlugin (gg, plugin) || ok;
    el = el->next;
  }

  return (ok);
}

/*

 */

void addPlugins (GList * plugins, GtkWidget * list, GGobiSession * gg);
void addPlugin (GGobiPlugin * info, GtkWidget * list, GGobiSession * gg);

GtkWidget *
createPluginList ()
{
  /* Number of entries here should be the same as in set_column_width below and 
     as the number of elements in addPlugin().
   */
  static gchar *titles[] =
    { "Name", "Description", "Author", "Location", "Active" };
  static const gint widths[] = { 100, 225, 150, 225, 50 };

  gint i;
  GtkWidget *list;
  GList *cols, *l;
  GtkListStore *model = gtk_list_store_new (5, G_TYPE_STRING, G_TYPE_STRING,
                                            G_TYPE_STRING, G_TYPE_STRING,
                                            G_TYPE_BOOLEAN);

  list = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
  populate_tree_view (list, titles, G_N_ELEMENTS (titles), true,
                      GTK_SELECTION_SINGLE, NULL, NULL);
  cols = gtk_tree_view_get_columns (GTK_TREE_VIEW (list));

  for (i = 0, l = cols; l; l = l->next, i++) {
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (l->data),
                                     GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (l->data),
                                          widths[i]);
  }
  return (list);
}

/*
 We should move to an interface more like Gnumeric's plugin
 info list.
 */
GtkWidget *
showPluginInfo (GList * plugins, GGobiSession * gg)
{
  GtkWidget *win, *list, *swin;

  win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (win), 850, 200);
  gtk_window_set_title (GTK_WINDOW (win), "About Plugins");

  swin = gtk_scrolled_window_new (NULL, NULL);
  list = createPluginList ();
  gtk_container_add (GTK_CONTAINER (swin), list);
  addPlugins (plugins, list, gg);
  gtk_container_add (GTK_CONTAINER (win), swin);

  gtk_widget_show_all (win);

  return (win);
}


/**
 Create a summary line for each plugin, adding it to the table widget.

 @see addPlugin()
 */
void
addPlugins (GList * plugins, GtkWidget * list, GGobiSession * gg)
{
  gint n = g_list_length (plugins), i;
  GGobiPlugin *plugin;

  for (i = 0; i < n; i++) {
    plugin = (GGobiPlugin *) g_list_nth_data (plugins, i);
    addPlugin (plugin, list, gg);
  }
}


/**
  Create the summary information line for a given plugin,
  giving the name, description, author, shared library/DLL,
  whether it is loaded and if it is active.
  @see addPlugins() 
 */
void
addPlugin (GGobiPlugin * plugin, GtkWidget * list, GGobiSession * gg)
{
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (list));
  GtkTreeIter iter;
  GGobiPluginDescription *description = ggobi_plugin_get_description(plugin);
  const gchar *name = ggobi_plugin_description_get_name(description);
  const gchar *desc = ggobi_plugin_description_get_description(description);
  const gchar *author = ggobi_plugin_description_get_author(description);
  const gchar *dll_name = ggobi_plugin_description_get_uri(description);
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
    0, name, 1, description, 2, author, 3, dll_name, -1);
}
