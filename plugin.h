#ifndef GGOBI_PLUGIN_H
#define GGOBI_PLUGIN_H

#include "ggobi.h"

#ifndef WIN32
typedef void * HINSTANCE;
#else
#include <windows.h>
#endif

typedef void (*DLFUNC)();

typedef struct {
    char *name;
    char *dllName;
    HINSTANCE library;

    char *onLoad; 
    char *onCreate; 
    char *onClose; 
    char *onUnload; 
    char *onUpdateDisplay; 

    char *description;
    char *author;

    gboolean loaded;
} GGobiPluginInfo;

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

  HINSTANCE (*open)(const char *name, GGobiPluginInfo *info);
  int   (*close)(HINSTANCE);
  DLFUNC  (*resolve)(HINSTANCE handle, const char *name);
  void  (*getError)(char *buf, GGobiPluginInfo *info);

} Dynload;



extern Dynload *dynload;

HINSTANCE load_plugin_library(GGobiPluginInfo *plugin);

DLFUNC getPluginSymbol(const char *name, GGobiPluginInfo *plugin);

gboolean registerPlugins(ggobid *gg, GList *plugins);
gboolean pluginsUpdateDisplayMenu(ggobid *gg, GList *plugins);

int      GGOBI_addPluginInstance(PluginInstance *inst, ggobid *gg);
gboolean GGOBI_removePluginInstance(PluginInstance *inst, ggobid *gg);

void closePlugins(ggobid *gg);
#endif
