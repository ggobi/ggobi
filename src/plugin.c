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

#include "plugin.h"
#include "externs.h"
#include "read_xml.h"
#include "read_csv.h"

#include <stdio.h>
#include <string.h>

void addPluginDetails (GGobiPluginDetails * info, GtkWidget * list,
                       ggobid * gg, gboolean active);
void addInputPlugin (GGobiPluginInfo * info, GtkWidget * list, ggobid * gg);
void addPlugin (GGobiPluginInfo * info, GtkWidget * list, ggobid * gg);

void plugin_init() {
  // no op
}

gboolean
GGobi_checkPlugin (GGobiPluginDetails * plugin)
{
  gboolean (*f) (const GGobiPluginDetails *);
  gboolean ok = true;
  f =
    (gboolean (*)(const GGobiPluginDetails * plugin))
    getPluginSymbol ("checkGGobiStructSizes", plugin);
  if (f) {
    if (!(ok = f (plugin)))
      g_printerr
        ("Problems with plugin %s. Incosistent view of ggobi's data structures.\n",
         plugin->name);
    else if (sessionOptions->verbose == GGOBI_VERBOSE)
      g_printerr ("plugin %s appears consistent with ggobi structures.\n",
                  plugin->name);
  }
  else if (sessionOptions->verbose == GGOBI_VERBOSE)
    g_printerr ("plugin %s has no validation mechanism\n", plugin->name);

  return (ok);
}

GGobiPluginInfo *
getLanguagePlugin (GList * plugins, const char *name)
{
  GList *el = plugins;

  while (el) {
    GGobiPluginInfo *info;
    info = (GGobiPluginInfo *) el->data;
    if (strcmp (info->details->name, name) == 0)
      return (info);
    el = el->next;
  }
  return (NULL);
}

gboolean
loadPluginLibrary (GGobiPluginDetails * plugin, GGobiPluginInfo * realPlugin)
{
  /* If it has already been loaded, just return. */
  if (plugin->loaded != DL_UNLOADED) {
    return (plugin->loaded == DL_FAILED ? false : true);
  }

  /* Load any plugins on which this one depends. Make certain they 
     are fully loaded and initialized. Potential for inter-dependencies
     that would make this an infinite loop. Hope the user doesn't get this
     wrong as there are no checks at present.
   */
  if (plugin->depends) {
    GSList *el = plugin->depends;
    while (el) {
      gchar *tmp = (gchar *) el->data;
      GGobiPluginInfo *info;
      info = getLanguagePlugin (sessionOptions->info->plugins, tmp);
      if (sessionOptions->verbose == GGOBI_VERBOSE) {
        fprintf (stderr, "Loading dependent plugin %s\n", tmp);
        fflush (stderr);
      }
      if (!loadPluginLibrary (info->details, info))
        return (false);
      el = el->next;
    }
  }

  plugin->library = load_plugin_library (plugin, true);
  plugin->loaded = plugin->library != NULL ? DL_LOADED : DL_FAILED;

  if (plugin->loaded == DL_LOADED && GGobi_checkPlugin (plugin)
      && plugin->onLoad) {
    OnLoad f = (OnLoad) getPluginSymbol (plugin->onLoad, plugin);
    if (f) {
      f (0, realPlugin);
    }
    else {
      g_critical("error loading plugin %s: %s",
                 plugin->dllName, g_module_error());
    }
  }
  return (plugin->loaded == DL_LOADED);
}

GModule *
load_plugin_library (GGobiPluginDetails * plugin, gboolean recurse)
{
  GModule *handle = NULL;
  gchar *fileName = ggobi_find_data_file(plugin->dllName);
  if (fileName) {
    handle = g_module_open(fileName, G_MODULE_BIND_LAZY);
    g_free(fileName);
  }
  if (!handle) {
    if (sessionOptions->verbose != GGOBI_SILENT) {
      g_critical("Error on loading plugin library %s: %s",
               plugin->dllName, g_module_error());
    }
    plugin->loaded = DL_FAILED;
  }
  else {
    plugin->loaded = DL_LOADED;
  }
  return (handle);
}


gpointer
getPluginSymbol (const char *name, GGobiPluginDetails * plugin)
{
  GModule *lib;
  gpointer sym;

  if (!plugin)
    return(NULL);
  else if (plugin->library == NULL && plugin->loaded != DL_LOADED) {
    lib = plugin->library = load_plugin_library (plugin, true);
  }
  else
    lib = plugin->library;

  g_module_symbol(lib, name, &sym);
  
  return sym; 
}


gboolean
registerPlugin (ggobid * gg, GGobiPluginInfo * plugin)
{
  gboolean ok = true;
  OnCreate f;
  PluginInstance *inst;

  if (plugin->type != GENERAL_PLUGIN)
    return (false);

  if (!plugin->details->loaded) {
    loadPluginLibrary (plugin->details, plugin);
  }

  if (plugin->info.g->onCreate) {
    f =
      (OnCreate) getPluginSymbol (plugin->info.g->onCreate, plugin->details);
    if (f) {
      inst = (PluginInstance *) g_malloc (sizeof (PluginInstance));
      inst->data = NULL;
      inst->info = plugin;
      inst->active = true;
      ok = f (gg, plugin, inst);
      if (ok) {
        GGOBI_addPluginInstance (inst, gg);
      }
      else
        g_free (inst);
    }
    else {
      g_critical("can't locate required plugin routine %s in %s", 
        plugin->info.g->onCreate, plugin->details->name);
    }
  }
  else {
    inst = (PluginInstance *) g_malloc (sizeof (PluginInstance));
    inst->data = NULL;
    inst->info = plugin;
    inst->gg = gg;
    inst->active = true;
    GGOBI_addPluginInstance (inst, gg);
  }
  return (ok);
}

gboolean
registerPlugins (ggobid * gg, GList * plugins)
{
  GList *el = plugins;
  gboolean ok = false;
  GGobiPluginInfo *plugin;

  while (el) {
    plugin = (GGobiPluginInfo *) el->data;
    ok = registerPlugin (gg, plugin) || ok;
    el = el->next;
  }

  return (ok);
}


gboolean
pluginsUpdateDisplayMenu (ggobid * gg, GList * plugins)
{
  GList *el = plugins;
  OnUpdateDisplayMenu f;
  PluginInstance *plugin;
  gboolean ok = true;

  while (el) {
    plugin = (PluginInstance *) el->data;
    if (plugin->info->type == GENERAL_PLUGIN
        && plugin->info->info.g->onUpdateDisplay) {
      f =
        (OnUpdateDisplayMenu) getPluginSymbol (plugin->info->info.g->
                                               onUpdateDisplay,
                                               plugin->info->details);
      if (f) {
        ok = f (gg, plugin);
      }
    }
    el = el->next;
  }

  return (ok);
}

int
GGOBI_addPluginInstance (PluginInstance * inst, ggobid * gg)
{
  inst->gg = gg;
  gg->pluginInstances = g_list_append (gg->pluginInstances, inst);
  return (g_list_length (gg->pluginInstances));
}

gboolean
GGOBI_removePluginInstance (PluginInstance * inst, ggobid * gg)
{
  inst->gg = NULL;
  gg->pluginInstances = g_list_remove (gg->pluginInstances, inst);
  /* should return whether the instance was actually there. */
  return (true);
}


/*

 */

void addPlugins (GList * plugins, GtkWidget * list, ggobid * gg,
                 GGobiPluginType);
void addPlugin (GGobiPluginInfo * info, GtkWidget * list, ggobid * gg);

GtkWidget *
createPluginList ()
{
  /* Number of entries here should be the same as in set_column_width below and 
     as the number of elements in addPlugin().
   */
  static gchar *titles[] =
    { "Name", "Description", "Author", "Location", "Loaded", "Active" };
  static const gint widths[] = { 100, 225, 150, 225, 50, 50 };

  gint i;
  GtkWidget *list;
  GList *cols, *l;
  //list = gtk_clist_new_with_titles(sizeof(titles)/sizeof(titles[0]), (gchar **) titles);
  GtkListStore *model = gtk_list_store_new (6, G_TYPE_STRING, G_TYPE_STRING,
                                            G_TYPE_STRING, G_TYPE_STRING,
                                            G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);

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
  /*
     gtk_clist_set_column_width(GTK_CLIST(list), 0, 100); 
     gtk_clist_set_column_width(GTK_CLIST(list), 1, 225); 
     gtk_clist_set_column_width(GTK_CLIST(list), 2, 150); 
     gtk_clist_set_column_width(GTK_CLIST(list), 3, 225); 
     gtk_clist_set_column_width(GTK_CLIST(list), 4,  50); 
     gtk_clist_set_column_width(GTK_CLIST(list), 5,  50); 
   */
  return (list);
}

/*
 We should move to an interface more like Gnumeric's plugin
 info list.
 */
GtkWidget *
showPluginInfo (GList * plugins, GList * inputPlugins, ggobid * gg)
{
  GtkWidget *win, *main_vbox, *list, *swin, *lbl = NULL;

  win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (win), 850, 200);
  gtk_window_set_title (GTK_WINDOW (win), "About Plugins");

  main_vbox = gtk_notebook_new ();

  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 0);
  gtk_container_add (GTK_CONTAINER (win), main_vbox);

  if (plugins) {
    swin = gtk_scrolled_window_new (NULL, NULL);
    list = createPluginList ();
    gtk_container_add (GTK_CONTAINER (swin), list);
    addPlugins (plugins, list, gg, GENERAL_PLUGIN);
    lbl = gtk_label_new_with_mnemonic ("_General");
    gtk_notebook_append_page (GTK_NOTEBOOK (main_vbox), swin, lbl);
  }
  if (inputPlugins) {
    swin = gtk_scrolled_window_new (NULL, NULL);
    list = createPluginList ();
    gtk_container_add (GTK_CONTAINER (swin), list);
    addPlugins (inputPlugins, list, gg, INPUT_PLUGIN);
    lbl = gtk_label_new_with_mnemonic ("_Input Readers");
    gtk_notebook_append_page (GTK_NOTEBOOK (main_vbox), swin, lbl);
  }

  gtk_widget_show_all (win);

  return (win);
}



/**
 Determine whether the specified plugin is active 
 for the given GGobi instance.
 */
gboolean
isPluginActive (GGobiPluginInfo * info, ggobid * gg)
{
  GList *el;
  PluginInstance *plugin;

  el = gg->pluginInstances;
  while (el) {
    plugin = (PluginInstance *) el->data;
    if (plugin->info == info)
      return (true);
    el = el->next;
  }

  return (false);
}

/**
 Create a summary line for each plugin, adding it to the table widget.

 @see addPlugin()
 */
void
addPlugins (GList * plugins, GtkWidget * list, ggobid * gg,
            GGobiPluginType type)
{
  gint n = g_list_length (plugins), i;
  GGobiPluginInfo *plugin;

  for (i = 0; i < n; i++) {
    plugin = (GGobiPluginInfo *) g_list_nth_data (plugins, i);
    switch (type) {
    case GENERAL_PLUGIN:
      addPlugin (plugin, list, gg);
      break;
    case INPUT_PLUGIN:
      addInputPlugin (plugin, list, gg);
      break;
    default:
      break;
    }
  }
}


/**
  Create the summary information line for a given plugin,
  giving the name, description, author, shared library/DLL,
  whether it is loaded and if it is active.
  @see addPlugins() 
 */
void
addPlugin (GGobiPluginInfo * info, GtkWidget * list, ggobid * gg)
{
  addPluginDetails (info->details, list, gg, isPluginActive (info, gg));
}

void
addInputPlugin (GGobiPluginInfo * info, GtkWidget * list, ggobid * gg)
{
  addPluginDetails (info->details, list, gg, true);
}

void
addPluginDetails (GGobiPluginDetails * info, GtkWidget * list, ggobid * gg,
                  gboolean active)
{
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (list));
  GtkTreeIter iter;
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, info->name, 1,
                      info->description, 2, info->author, 3, info->dllName, 4,
                      info->loaded == DL_LOADED, 5, active, -1);
  /*
     gchar **els = (gchar **) g_malloc(6*sizeof(gchar*));
     els[0] = info->name;
     els[1] = info->description;
     els[2] = info->author;
     els[3] = info->dllName;
     els[4] = info->loaded ? "yes" : "no";

     els[5] = active ? "yes" : "no";

     gtk_clist_append(GTK_CLIST(list), els); */
}

/**
  Close each of the plugins within the specified GGobi instance.
  This doesn't unload the plugin.
 */
void
closePlugins (ggobid * gg)
{
  GList *el, *tmp;
  PluginInstance *plugin;

  el = gg->pluginInstances;
  if (!el || g_list_length (el) == 0) {
    return;
  }

  while (el) {
    plugin = (PluginInstance *) el->data;
    if (plugin->info->info.g->onClose) {
      DLFUNC f =
        getPluginSymbol (plugin->info->info.g->onClose,
                         plugin->info->details);
      if (f)
        f (gg, plugin->info, plugin);
    }
    tmp = el;
    el = el->next;
    g_free (plugin);
/*  g_free(tmp); */
  }
  gg->pluginInstances = NULL;
}



/*
 Determine if the plugin handles this mode.
 */
gboolean
pluginSupportsInputMode (const gchar * modeName, GGobiPluginInfo * pluginInfo)
{
  int i;

  if (!modeName)
    return (false);

  for (i = 0; i < pluginInfo->info.i->numModeNames; i++) {
    if (strcmp (modeName, pluginInfo->info.i->modeNames[i]) == 0)
      return (true);
  }

  return (false);
}

GGobiPluginInfo *
runInteractiveInputPlugin (ggobid * gg)
{
  GGobiPluginInfo *plugin = NULL;
  GList *l = sessionOptions->info->inputPlugins;

  for (; l; l = l->next) {
    plugin = (GGobiPluginInfo *) l->data;
    if (plugin->info.i->interactive) {
      if (!sessionOptions->data_type ||
          pluginSupportsInputMode (sessionOptions->data_type, plugin)) {
        InputGetDescription f;
        if (!loadPluginLibrary (plugin->details, plugin)) {
          g_printerr ("Failed to load plugin %s\n", plugin->details->name);
          continue;
        }
        f =
          (InputGetDescription) getPluginSymbol (plugin->info.i->
                                                 getDescription,
                                                 plugin->details);
        if (f) {
          InputDescription *desc;
          desc = f (NULL, sessionOptions->data_type, gg, plugin);
          if (desc && desc->desc_read_input) {
            gg->input = desc;
            desc->desc_read_input (desc, gg, plugin);
            break;
          }
        }
      }
    }
  }

  return (plugin);
}

/***************************************************************************/

gchar *XMLModeNames[] = { "xml", "url" };
GGobiInputPluginInfo XMLInputPluginInfo = {
  NULL,
  0,
  "",
  "",
  "read_xml_input_description",
  false,
  read_xml,
  &read_xml_input_description,
  isXMLFile,
  xml_data
};

GGobiPluginDetails XMLDetails = {
  "XML reader",
  NULL,
  NULL,
  "Reads XML URLs (http, ftp, local files or zipped local files)",
  "GGobi core",
  TRUE
};

gchar *CSVModeNames[] = { "csv" };
GGobiInputPluginInfo CSVInputPluginInfo = {
  NULL,
  0,
  "",
  "",
  "read_csv_input_description",
  false,
  read_csv,
  read_csv_input_description,
  isCSVFile,
  csv_data
};


GGobiPluginDetails CSVDetails = {
  "CSV reader",
  NULL,
  NULL,
  "Reads Comma-separated data from local files",
  "Michael Lawrence",
  TRUE
};


GGobiPluginInfo *
createGGobiInputPluginInfo (GGobiInputPluginInfo * info,
                            GGobiPluginDetails * details, gchar ** modeNames,
                            guint numModes)
{
  GGobiPluginInfo *plugin;

  plugin = (GGobiPluginInfo *) g_malloc0 (sizeof (GGobiPluginInfo));

  plugin->type = INPUT_PLUGIN;
  plugin->info.i = info;
  plugin->details = details;

  if (modeNames) {
    guint i;
    plugin->info.i->modeNames =
      (gchar **) g_malloc (sizeof (gchar *) * numModes);
    plugin->info.i->numModeNames = numModes;
    for (i = 0; i < numModes; i++)
      plugin->info.i->modeNames[i] = g_strdup (modeNames[i]);
  }

  return (plugin);
}

/*
  Register the basic, built-in "plugins", specifically
  the input plugins for XML, CSV.
*/
void
registerDefaultPlugins (GGobiInitInfo * info)
{
  GGobiPluginInfo *plugin;

  plugin =
    createGGobiInputPluginInfo (&XMLInputPluginInfo, &XMLDetails,
                                XMLModeNames,
                                sizeof (XMLModeNames) /
                                sizeof (XMLModeNames[0]));
  info->inputPlugins = g_list_append (info->inputPlugins, plugin);

  plugin =
    createGGobiInputPluginInfo (&CSVInputPluginInfo, &CSVDetails,
                                CSVModeNames,
                                sizeof (CSVModeNames) /
                                sizeof (CSVModeNames[0]));
  info->inputPlugins = g_list_append (info->inputPlugins, plugin);

}

const gchar *DefaultUnknownInputModeName = "unknown";

GList *
getInputPluginSelections (ggobid * gg)
{
  GList *els = NULL, *plugins;
  GGobiPluginInfo *plugin;
  int i, n, k;
  gchar *buf;

  els = g_list_append (els, g_strdup (DefaultUnknownInputModeName));
  plugins = sessionOptions->info->inputPlugins;
  n = g_list_length (plugins);
  for (i = 0; i < n; i++) {
    plugin = g_list_nth_data (plugins, i);
    for (k = 0; k < plugin->info.i->numModeNames; k++) {
      buf =
        g_strdup_printf ("%s (%s)", plugin->info.i->modeNames[k],
                         plugin->details->name);
      els = g_list_append (els, buf);
    }
  }

  return (els);
}

GGobiPluginInfo *
getInputPluginByModeNameIndex (gint which, gchar ** modeName)
{
  gint ctr = 1, numPlugins, i;  /* Start at 1 since guess/unknown is 0. */
  GList *plugins = sessionOptions->info->inputPlugins;
  GGobiPluginInfo *plugin;

  if (which == 0) {
    *modeName = g_strdup (DefaultUnknownInputModeName);
    return (NULL);
  }

  numPlugins = g_list_length (plugins);
  for (i = 0; i < numPlugins; i++) {
    plugin = g_list_nth_data (plugins, i);
    if (which >= ctr && which < ctr + plugin->info.i->numModeNames) {
      *modeName = g_strdup (plugin->info.i->modeNames[which - ctr]);
      return (plugin);
    }
    ctr += plugin->info.i->numModeNames;
  }

  return (NULL);                /* Should never happen */
}

InputDescription *
callInputPluginGetDescription (const gchar * fileName, const gchar * modeName,
                               GGobiPluginInfo * plugin, ggobid * gg)
{
  GGobiInputPluginInfo *info;
  InputGetDescription f;

  if (sessionOptions->verbose == GGOBI_VERBOSE) {
    g_printerr ("Checking input plugin %s.\n", plugin->details->name);
  }

  info = plugin->info.i;
  if (info->get_description_f)
    f = info->get_description_f;
  else
    f = (InputGetDescription) getPluginSymbol (info->getDescription,
                                               plugin->details);

  if (f) {
    InputDescription *desc;
    desc = f (fileName, modeName, gg, plugin);
    if (desc)
      return (desc);
  }
  else if (sessionOptions->verbose == GGOBI_VERBOSE) {
    g_printerr ("No handler routine for plugin %s.: %s\n",
                plugin->details->name, info->getDescription);
  }

  return (NULL);
}
