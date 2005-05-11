/* plugin.h */

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
    char *name;
    char *dllName;
    HINSTANCE library;

    char *description;
    char *author;

    PluginLoadStatus loaded;

    char *onLoad; 
    char *onUnload; 

    char *language;
    GSList *depends;

    GSList *args;
    GHashTable *namedArgs;

} GGobiPluginDetails;

typedef struct {
    char *onCreate; 
    char *onClose; 

    char *onUpdateDisplay; 
} GGobiGeneralPluginInfo;


/*
  The two plugin types should share the common information.
 */
struct   _GGobiInputPluginInfo {
#if 0
    char *_modeName;
#endif
    char **modeNames;
    guint numModeNames;
    char *read_symbol_name;
    char *probe_symbol_name;
    char *getDescription;

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
    char *processPluginName;
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

  HINSTANCE (*open)(const char *name, GGobiPluginDetails *info);
  int   (*close)(HINSTANCE);
  DLFUNC  (*resolve)(HINSTANCE handle, const char *name);
  void  (*getError)(char *buf, GGobiPluginDetails *info);

} Dynload;



extern const Dynload *dynload;

HINSTANCE load_plugin_library(GGobiPluginDetails *plugin, gboolean recurse);

DLFUNC getPluginSymbol(const char *name, GGobiPluginDetails *plugin);

gboolean registerPlugins(ggobid *gg, GList *plugins);
gboolean pluginsUpdateDisplayMenu(ggobid *gg, GList *plugins);

int      GGOBI_addPluginInstance(PluginInstance *inst, ggobid *gg);
gboolean GGOBI_removePluginInstance(PluginInstance *inst, ggobid *gg);

void closePlugins(ggobid *gg);

GGobiPluginInfo *runInteractiveInputPlugin(ggobid *gg);
GtkWidget *showPluginInfo(GList *plugins, GList *inputPlugins, ggobid *gg);

gboolean loadPluginLibrary(GGobiPluginDetails *plugin, GGobiPluginInfo *realPlugin);
gboolean GGobi_checkPlugin(GGobiPluginDetails *plugin);
gboolean setLanguagePluginInfo(GGobiPluginDetails *details, const char *language, GGobiInitInfo *info);

gboolean pluginSupportsInputMode(const gchar *modeName, GGobiPluginInfo *pluginInfo);

GGobiPluginInfo *getInputPluginByModeNameIndex(gint index);
InputDescription *callInputPluginGetDescription(const gchar *fileName, const gchar *modeName, GGobiPluginInfo *info, ggobid *gg);

#endif
