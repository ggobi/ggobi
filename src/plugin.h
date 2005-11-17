/* plugin.h */
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

#ifndef GGOBI_PLUGIN_H
#define GGOBI_PLUGIN_H

#include "ggobi.h"

#ifndef WIN32
typedef void * HINSTANCE;
#else
#include <windows.h>
#endif

typedef enum {GENERAL_PLUGIN, INPUT_PLUGIN} GGobiPluginType;

typedef enum {DL_UNLOADED = 0, DL_LOADED, DL_FAILED} PluginLoadStatus;

typedef void (*DLFUNC)();

typedef struct {
    gchar *name;
    gchar *dllName;
    HINSTANCE library;

    gchar *description;
    gchar *author;

    PluginLoadStatus loaded;

    gchar *onLoad; 
    gchar *onUnload; 

    gchar *language;
    GSList *depends;

    GSList *args;
    GHashTable *namedArgs;

} GGobiPluginDetails;

typedef struct {
    gchar *onCreate; 
    gchar *onClose; 

    gchar *onUpdateDisplay; 
} GGobiGeneralPluginInfo;


/*
  The two plugin types should share the common information.
 */
struct   _GGobiInputPluginInfo {
#if 0
    gchar *_modeName;
#endif
    gchar **modeNames;
    guint numModeNames;
    gchar *read_symbol_name;
    gchar *probe_symbol_name;
    gchar *getDescription;

    gboolean interactive;

    InputReader plugin_read_input;
    InputGetDescription get_description_f;
    InputProbe  probe; /* Used when guessing the format. Each plugin is asked whether it supports
                          this file. It can do so by looking at the extension, reading the
                          start of the file, etc. 
                        */
    
    DataMode mode;
};

struct _GGobiPluginInfo {

  GGobiPluginDetails *details;
  GGobiPluginType type;
  union {
    GGobiGeneralPluginInfo *g;
    GGobiInputPluginInfo   *i;
  } info;

  void *data;
};


#ifdef USE_GNOME_XML
#include <gnome-xml/tree.h>
#else
#include <libxml/tree.h>
#endif

typedef gboolean (*ProcessPluginInfo)(xmlNodePtr, GGobiPluginInfo *, GGobiPluginType, GGobiPluginInfo *, GGobiInitInfo *info);


typedef struct {
    gchar *processPluginName;
    gboolean (*processPlugin)();
} GGobiLanguagePluginData;


typedef struct {
  GGobiPluginInfo *info;
  ggobid *gg;
  gboolean active;
  void *data;
} PluginInstance;



typedef gboolean (*OnLoad)(gboolean initializing, GGobiPluginInfo *plugin);
typedef gboolean (*OnCreate)(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst);
typedef gboolean (*OnClose)(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst);
typedef gboolean (*OnUnload)(gboolean quitting, GGobiPluginInfo *plugin);

typedef gboolean (*OnUpdateDisplayMenu)(ggobid *gg, PluginInstance *inst);


typedef struct {

  HINSTANCE (*open)(const gchar *name, GGobiPluginDetails *info);
  int   (*close)(HINSTANCE);
  DLFUNC  (*resolve)(HINSTANCE handle, const gchar *name);
  void  (*getError)(gchar *buf, GGobiPluginDetails *info);

} Dynload;



extern const Dynload *dynload;

HINSTANCE load_plugin_library(GGobiPluginDetails *plugin, gboolean recurse);

DLFUNC getPluginSymbol(const gchar *name, GGobiPluginDetails *plugin);

gboolean registerPlugins(ggobid *gg, GList *plugins);
gboolean pluginsUpdateDisplayMenu(ggobid *gg, GList *plugins);

int      GGOBI_addPluginInstance(PluginInstance *inst, ggobid *gg);
gboolean GGOBI_removePluginInstance(PluginInstance *inst, ggobid *gg);

void closePlugins(ggobid *gg);

GGobiPluginInfo *runInteractiveInputPlugin(ggobid *gg);
GtkWidget *showPluginInfo(GList *plugins, GList *inputPlugins, ggobid *gg);

gboolean loadPluginLibrary(GGobiPluginDetails *plugin, GGobiPluginInfo *realPlugin);
gboolean GGobi_checkPlugin(GGobiPluginDetails *plugin);
gboolean setLanguagePluginInfo(GGobiPluginDetails *details, const gchar *language, GGobiInitInfo *info);

gboolean pluginSupportsInputMode(const gchar *modeName, GGobiPluginInfo *pluginInfo);

GGobiPluginInfo *getInputPluginByModeNameIndex(gint index,gchar **modeName);
InputDescription *callInputPluginGetDescription(const gchar *fileName, const gchar *modeName, GGobiPluginInfo *info, ggobid *gg);


void registerDefaultPlugins(GGobiInitInfo *info);

#endif
