#include "plugin.h"

#include <stdio.h>

HINSTANCE ggobi_dlopen(const char *name, GGobiPluginDetails *plugin);
void ggobi_dlerror(char *buf, GGobiPluginDetails *plugin);

#ifdef WIN32

#include <string.h>

int ggobi_dlclose(HINSTANCE handle);
DLFUNC ggobi_dlsym(HINSTANCE handle, const char *name);

static Dynload winDynload = { 
                        ggobi_dlopen, 
                        ggobi_dlclose, 
                        ggobi_dlsym,  /* warning because we use const char * */
                        ggobi_dlerror};
Dynload *dynload = &winDynload;

#else

#include <dlfcn.h>
static Dynload unixDynload = { ggobi_dlopen, 
                        dlclose, 
                        dlsym,  /* warning because we use const char * */
                        ggobi_dlerror};
Dynload *dynload = &unixDynload;

#endif


#ifndef WIN32 
#include <unistd.h> 
#include <sys/stat.h> 

#define DLL_EXTENSION ".so"

#else

#include <glib.h> 
# ifdef __STRICT_ANSI__ 
# undef   __STRICT_ANSI__ 
# endif
# include <io.h> 

#define DLL_EXTENSION ".dll"

#endif 

void addPluginDetails(GGobiPluginDetails *info, GtkWidget *list, ggobid *gg, gboolean active);
void addInputPlugin(GGobiInputPluginInfo *info, GtkWidget *list, ggobid *gg);
void addPlugin(GGobiPluginInfo *info, GtkWidget *list, ggobid *gg);

gboolean
canRead(const char * const fileName)
{
  gboolean val = false;
#ifndef WIN32
  struct stat buf;
  val = (stat(fileName, &buf) == 0);
#else
  gint ft=0;
  val = (access(fileName, ft) != 0);
#endif

  return(val);
}

HINSTANCE 
load_plugin_library(GGobiPluginDetails *plugin)
{
  HINSTANCE handle;
  char *fileName;
  fileName = plugin->dllName;
  if(canRead(fileName) == false) {
    fileName = (char *) g_malloc((strlen(fileName)+ strlen(DLL_EXTENSION) + 1)*sizeof(char));
    strcpy(fileName, plugin->dllName);
    strcpy(fileName+strlen(plugin->dllName), DLL_EXTENSION);
    fileName[strlen(plugin->dllName) + strlen(DLL_EXTENSION)] = '\0';
  }

  if(canRead(fileName) == false) {
    fprintf(stderr, "can't locate plugin library %s:\n", plugin->dllName);fflush(stderr);      
    if(fileName != plugin->dllName)
      g_free(fileName);
    return(NULL);
  }

   handle = dynload->open(fileName, plugin);
   if(!handle) {
    char buf[1000];
      dynload->getError(buf, plugin);
      fprintf(stderr, "error on loading plugin library %s: %s\n", plugin->dllName, buf);fflush(stderr);
   }

   if(fileName != plugin->dllName)
      g_free(fileName);
   return(handle);
}

DLFUNC 
getPluginSymbol(const char *name, GGobiPluginDetails *plugin)
{
  if(plugin->library == NULL) {
     plugin->library = load_plugin_library(plugin);   
  }
  return(dynload->resolve(plugin->library, name));
}


gboolean 
registerPlugins(ggobid *gg, GList *plugins)
{
  GList *el = plugins;
  OnCreate f;
  GGobiPluginInfo *plugin;
  PluginInstance *inst;
  gboolean ok = true;

  while(el) {
    plugin = (GGobiPluginInfo *) el->data;
    ok = true;
    if(plugin->onCreate) {
      f = (OnCreate) getPluginSymbol(plugin->onCreate, &plugin->details);
      if(f) {
	  inst = (PluginInstance *) g_malloc(sizeof(PluginInstance));
          inst->data = NULL;
          inst->info = plugin;
          inst->active = true;
	  ok = f(gg, plugin, inst);
	  if(ok) {
	      GGOBI_addPluginInstance(inst, gg);
	  } else
	      g_free(inst);
      }
    } else {
	  inst = (PluginInstance *) g_malloc(sizeof(PluginInstance));
          inst->data = NULL;
          inst->info = plugin;
          inst->gg = gg;
          inst->active = true;
          GGOBI_addPluginInstance(inst, gg);
    }
    el = el->next;
  }

  return(ok);
}

gboolean 
pluginsUpdateDisplayMenu(ggobid *gg, GList *plugins)
{
  GList *el = plugins;
  OnUpdateDisplayMenu f;
  PluginInstance *plugin;
  gboolean ok = true;

  while(el) {
    plugin = (PluginInstance *) el->data;
    if(plugin->info->onUpdateDisplay) {
      f = (OnUpdateDisplayMenu) getPluginSymbol(plugin->info->onUpdateDisplay,
                                                &plugin->info->details);
      if(f) {
        ok = f(gg, plugin);
      }
    }
    el = el->next;
  }

  return(ok);
}

int 
GGOBI_addPluginInstance(PluginInstance *inst, ggobid *gg)
{
  inst->gg = gg;
  gg->pluginInstances = g_list_append(gg->pluginInstances, inst);
  return(g_list_length(gg->pluginInstances));
}

gboolean
GGOBI_removePluginInstance(PluginInstance *inst, ggobid *gg)
{
  inst->gg = NULL;
  gg->pluginInstances = g_list_remove(gg->pluginInstances, inst);
  /* should return whether the instance was actually there. */
  return(true);
}


/*

 */

void addPlugins(GList *plugins, GtkWidget *list, ggobid *gg, GGobiPluginType);
void addPlugin(GGobiPluginInfo *info, GtkWidget *list, ggobid *gg);

/*
 We should move to an interface more like Gnumeric's plugin
 info list.
 */
GtkWidget *
showPluginInfo(GList *plugins, GList *inputPlugins, ggobid *gg)
{
 GtkWidget *win, *main_vbox, *list;
   /* Number of entries here should be the same as in set_column_width below and 
      as the number of elements in addPlugin().
    */
 static const gchar *titles[] = {"Name", "Description", "Author", "Location", "Loaded", "Active"};

  win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  main_vbox=gtk_vbox_new(FALSE,1);
  gtk_container_set_border_width(GTK_CONTAINER(main_vbox),0); 
  gtk_container_add(GTK_CONTAINER(win), main_vbox);
  gtk_widget_show(main_vbox);


  list = gtk_clist_new_with_titles(sizeof(titles)/sizeof(titles[0]), (gchar **) titles);

  gtk_clist_set_column_width(GTK_CLIST(list), 0, 100); 
  gtk_clist_set_column_width(GTK_CLIST(list), 1, 225); 
  gtk_clist_set_column_width(GTK_CLIST(list), 2, 150); 
  gtk_clist_set_column_width(GTK_CLIST(list), 3, 225); 
  gtk_clist_set_column_width(GTK_CLIST(list), 4,  50); 
  gtk_clist_set_column_width(GTK_CLIST(list), 5,  50); 

  if(plugins)
    addPlugins(plugins, list, gg, GENERAL_PLUGIN);
  if(inputPlugins)
    addPlugins(inputPlugins, list, gg, INPUT_PLUGIN);

  gtk_box_pack_start(GTK_BOX(main_vbox), list, TRUE, TRUE, 0);
  gtk_widget_show(list);
  gtk_widget_show(win);
 
  return(win); 
}

/**
 Determine whether the specified plugin is active 
 for the given GGobi instance.
 */
gboolean
isPluginActive(GGobiPluginInfo *info, ggobid *gg)
{
  GList *el;
  PluginInstance *plugin;

  el =  gg->pluginInstances;
  while(el) { 
    plugin = (PluginInstance *) el->data;
    if(plugin->info == info)
	return(true);
    el = el->next;
  }

  return(false);
}

/**
 Create a summary line for each plugin, adding it to the table widget.

 @see addPlugin()
 */
void
addPlugins(GList *plugins, GtkWidget *list, ggobid *gg, GGobiPluginType type)
{
 int n = g_list_length(plugins), i;
 GGobiPluginInfo *plugin;
 GGobiInputPluginInfo *iplugin;

 for(i = 0; i < n ; i++) {
     switch(type) {
      case GENERAL_PLUGIN:
	 plugin = (GGobiPluginInfo*) g_list_nth_data(plugins, i);
	 addPlugin(plugin, list, gg);
	 break;
     case INPUT_PLUGIN:
	 iplugin = (GGobiInputPluginInfo*) g_list_nth_data(plugins, i);
	 addInputPlugin(iplugin, list, gg);
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
addPlugin(GGobiPluginInfo *info, GtkWidget *list, ggobid *gg)
{
    addPluginDetails(&info->details, list, gg, isPluginActive(info, gg));
}

void
addInputPlugin(GGobiInputPluginInfo *info, GtkWidget *list, ggobid *gg)
{
    addPluginDetails(&info->details, list, gg, true);
}

void
addPluginDetails(GGobiPluginDetails *info, GtkWidget *list, ggobid *gg, gboolean active)
{
  gchar **els = (gchar **) g_malloc(6*sizeof(gchar*));
  els[0] = info->name;
  els[1] = info->description;
  els[2] = info->author;
  els[3] = info->dllName;
  els[4] = info->loaded ? "yes" : "no";

  els[5] = active ? "yes" : "no";

  gtk_clist_append(GTK_CLIST(list), els);
}

/**
  Close each of the plugins within the specified GGobi instance.
  This doesn't unload the plugin.
 */
void
closePlugins(ggobid *gg)
{
  GList *el, *tmp;
  PluginInstance *plugin;

  el = gg->pluginInstances;
  while(el) { 
    plugin = (PluginInstance *) el->data;
    if(plugin->info->onClose) {
      DLFUNC f =  getPluginSymbol(plugin->info->onClose, &plugin->info->details);
      f(gg, plugin, el);
    }
    tmp = el;
    el = el->next;
    g_free(plugin);    
/*  g_free(tmp); */
  }
  gg->pluginInstances = NULL;
}

GGobiInputPluginInfo *
runInteractiveInputPlugin(ggobid *gg)
{
    GGobiInputPluginInfo* plugin = NULL;
    GList *l = sessionOptions->info->inputPlugins;

    for(; l; l = l->next) {
	plugin =  (GGobiInputPluginInfo*) l->data;
        if(plugin->interactive) {
	    if(!sessionOptions->data_type || strcmp(sessionOptions->data_type, plugin->modeName) == 0) {
		InputGetDescription f;
		f = (InputGetDescription) getPluginSymbol(plugin->getDescription, &plugin->details);
		if(f) {
		    InputDescription *desc;
		    desc = f(NULL, NULL, gg, plugin);
		    if(desc && desc->read_input) {
			gg->input = desc;
			desc->read_input(desc, gg);
			break;
		    }
		}
	}
	}
    }

    return(plugin);
}



/***************************************************************************/

#ifdef WIN32

HINSTANCE
ggobi_dlopen(const char *name, GGobiPluginDetails *plugin)
{
  return(LoadLibrary(name));
}

void
ggobi_dlerror(char *buf, GGobiPluginDetails *plugin)
{
    LPVOID lpMsgBuf;
	FormatMessage( 
	    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
	    FORMAT_MESSAGE_FROM_SYSTEM | 
	    FORMAT_MESSAGE_IGNORE_INSERTS,
	    NULL,
	    GetLastError(),
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	    (LPTSTR) &lpMsgBuf,
	    0,
	    NULL 
	    );
	strcpy(buf, "Failure in LoadLibrary:  ");
	strcat(buf, lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int
ggobi_dlclose(HINSTANCE handle)
{
  FreeLibrary(handle);
  return(0);
}


DLFUNC
ggobi_dlsym(HINSTANCE handle, const char *name)
{
  return((DLFUNC) GetProcAddress(handle, name));
}


#else

HINSTANCE
ggobi_dlopen(const char *name, GGobiPluginDetails *plugin)
{
  return(dlopen(name, RTLD_LOCAL | RTLD_NOW ));
}

void
ggobi_dlerror(char *buf, GGobiPluginDetails *plugin)
{
  sprintf(buf, dlerror());
}

#endif
