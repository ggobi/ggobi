#ifndef GGOBI_PLUGIN_H
#define GGOBI_PLUGIN_H

#include "ggobi.h"

#ifndef WIN32
typedef void * HINSTANCE;
#else
#include <windows.h>
#endif

typedef enum {GENERAL_PLUGIN, INPUT_PLUGIN} GGobiPluginType;

typedef void (*DLFUNC)();

typedef struct {
    char *name;
    char *dllName;
    HINSTANCE library;

    char *description;
    char *author;

    gboolean loaded;

    char *onLoad; 
    char *onUnload; 

} GGobiPluginDetails;

typedef struct {

    GGobiPluginDetails details;

    char *onCreate; 
    char *onClose; 

    char *onUpdateDisplay; 

} GGobiPluginInfo;


/*
  The two plugin types should share the common information.
 */
struct   _GGobiInputPluginInfo {
    GGobiPluginDetails details;

    char *modeName;
    char *read_symbol_name;
    char *probe_symbol_name;
    char *getDescription;

    gboolean interactive;

    InputReader read_input;
    InputProbe  probe;
};


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



extern Dynload *dynload;

HINSTANCE load_plugin_library(GGobiPluginDetails *plugin);

DLFUNC getPluginSymbol(const char *name, GGobiPluginDetails *plugin);

gboolean registerPlugins(ggobid *gg, GList *plugins);
gboolean pluginsUpdateDisplayMenu(ggobid *gg, GList *plugins);

int      GGOBI_addPluginInstance(PluginInstance *inst, ggobid *gg);
gboolean GGOBI_removePluginInstance(PluginInstance *inst, ggobid *gg);

void closePlugins(ggobid *gg);

GGobiInputPluginInfo *runInteractiveInputPlugin(ggobid *gg);
GtkWidget *showPluginInfo(GList *plugins, GList *inputPlugins, ggobid *gg);
#endif
