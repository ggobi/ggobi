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
    char *modeName;
    char *read_symbol_name;
    char *probe_symbol_name;
    char *getDescription;

    gboolean interactive;

    InputReader plugin_read_input;
    InputProbe  probe;

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


#define GTK_TYPE_GGOBI_PLUGIN_INFO       (gtk_ggobi_plugin_info_get_type ())
#define GTK_GGOBI_PLUGIN_INFO(obj)	 (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_INFO_PLUGIN, ))
#define GTK_GGOBI_PLUGIN_INFO_CLASS(klass)	 (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_SPLOT, GGobiPluginInfoClass))
#define GTK_IS_GGOBI_PLUGIN_INFO(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_PLUGIN_INFO))
#define GTK_IS_GGOBI_PLUGIN_INFO_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_PLUGIN_INFO))

GtkType gtk_ggobi_plugin_info_get_type(void);


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

HINSTANCE load_plugin_library(GGobiPluginDetails *plugin);

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

#endif
