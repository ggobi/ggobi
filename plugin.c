#include "plugin.h"

#include <stdio.h>
#include <string.h>

HINSTANCE ggobi_dlopen(const char *name, GGobiPluginDetails *plugin);
void ggobi_dlerror(char *buf, GGobiPluginDetails *plugin);

#ifdef WIN32

int ggobi_dlclose(HINSTANCE handle);
DLFUNC ggobi_dlsym(HINSTANCE handle, const char *name);

static const Dynload winDynload = { 
                        ggobi_dlopen, 
                        ggobi_dlclose, 
                        ggobi_dlsym,  /* warning because we use const char * */
                        ggobi_dlerror};
const Dynload *dynload = &winDynload;

#else

#include <dlfcn.h>
static const Dynload unixDynload = { ggobi_dlopen, 
				     dlclose, 
				     dlsym,  /* warning because we use const char * */
				     ggobi_dlerror};
const Dynload *dynload = &unixDynload;

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
void addInputPlugin(GGobiPluginInfo *info, GtkWidget *list, ggobid *gg);
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
  val = (access(fileName, ft) == 0);
#endif

  return(val);
}

gboolean
GGobi_checkPlugin(GGobiPluginDetails *plugin)
{
 gboolean (*f)(void);
 gboolean ok = true;
 f = (gboolean (*)(void)) getPluginSymbol("checkGGobiStructSizes", plugin);
 if(f) {
   if(!(ok = f())) 
     g_printerr("Problems with plugin %s. Incosistent view of ggobi's data structures.\n", plugin->name);
   else if(sessionOptions->verbose == GGOBI_VERBOSE)
     g_printerr("plugin %s appears consistent with ggobi structures.\n", plugin->name);   
 } else if(sessionOptions->verbose == GGOBI_VERBOSE)
     g_printerr("plugin %s has no validation mechanism\n", plugin->name);   

 return(ok);
}

char *
installed_file_name(char *name)
{
  char *tmp;
/*  char *dirPtr, *dir; */

  tmp = (char *) g_malloc( (strlen(sessionOptions->ggobiHome)  + strlen(name)+ 3)*sizeof(char));
  sprintf(tmp, "%s%s", sessionOptions->ggobiHome, name);

  return(tmp);
}

HINSTANCE 
load_plugin_library(GGobiPluginDetails *plugin, gboolean recurse)
{
  HINSTANCE handle;
  char *fileName;
  fileName = plugin->dllName;

  if(!fileName || !fileName[0]) {
    plugin->loaded = DL_UNLOADED;  
    return(NULL);
  }

  if(canRead(fileName) == false) {
    fileName = (char *)
      g_malloc((strlen(fileName)+ strlen(DLL_EXTENSION) + 1)*sizeof(char));
    strcpy(fileName, plugin->dllName);
    strcpy(fileName+strlen(plugin->dllName), DLL_EXTENSION);
    fileName[strlen(plugin->dllName) + strlen(DLL_EXTENSION)] = '\0';
  }

  if(canRead(fileName) == false && recurse) {
   char *tmp = plugin->dllName;  
   if(fileName != plugin->dllName)
     g_free(fileName);

   plugin->dllName = installed_file_name(plugin->dllName);

   handle = load_plugin_library(plugin, false);
   if(!handle) {
     g_free(plugin->dllName);
     plugin->dllName = tmp;
   } else {
     g_free(tmp);
   }
   return(handle);
  }

  if(canRead(fileName) == false) {
    if(sessionOptions->verbose != GGOBI_SILENT) {
      fprintf(stderr, "can't locate plugin library %s:\n", plugin->dllName);
      fflush(stderr);
    }
    if(fileName != plugin->dllName)
      g_free(fileName);
    plugin->loaded = DL_LOADED;
    return(NULL);
  }

   handle = dynload->open(fileName, plugin);
   if(!handle) {
    if(sessionOptions->verbose != GGOBI_SILENT) {
     char buf[1000];
     dynload->getError(buf, plugin);
     fprintf(stderr, "error on loading plugin library %s (%s): %s\n",
       plugin->dllName, fileName, buf);
     fflush(stderr);
    }
    plugin->loaded = DL_FAILED;
   } else {
    plugin->loaded = DL_LOADED;
   }

   if(fileName != plugin->dllName)
     g_free(fileName);
   return(handle);
}


DLFUNC 
getPluginSymbol(const char *name, GGobiPluginDetails *plugin)
{
  char tmp[100];
  HINSTANCE lib;
#ifdef HAVE_UNDERSCORE_SYMBOL_PREFIX
  sprintf(tmp, "_%s", name);
#else
  sprintf(tmp, "%s", name);
#endif

  if(!plugin)
     lib = NULL;
  else if(plugin->library == NULL && plugin->loaded != DL_LOADED) {
     lib = plugin->library = load_plugin_library(plugin, true);   
  }  else
     lib = plugin->library;

  return(dynload->resolve(lib, tmp));
}


gboolean
registerPlugin(ggobid *gg, GGobiPluginInfo *plugin)
{
  gboolean ok = true;
  OnCreate f;
  PluginInstance *inst;

  if(plugin->type != GENERAL_PLUGIN) 
     return(false);

    if(!plugin->details->loaded) {
      loadPluginLibrary(plugin->details, plugin);
    }

    if(plugin->info.g->onCreate) {
      f = (OnCreate) getPluginSymbol(plugin->info.g->onCreate, plugin->details);
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
      } else {
    fprintf(stderr, "can't locate routine %s\n", plugin->info.g->onCreate);fflush(stderr);
      }
    } else {
      inst = (PluginInstance *) g_malloc(sizeof(PluginInstance));
      inst->data = NULL;
      inst->info = plugin;
      inst->gg = gg;
      inst->active = true;
      GGOBI_addPluginInstance(inst, gg);
    }
    return(ok);
}

gboolean 
registerPlugins(ggobid *gg, GList *plugins)
{
  GList *el = plugins;
  gboolean ok = false;
  GGobiPluginInfo *plugin;

  while(el) {
    plugin = (GGobiPluginInfo *) el->data;
    ok = registerPlugin(gg, plugin) || ok;
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
    if(plugin->info->type == GENERAL_PLUGIN && plugin->info->info.g->onUpdateDisplay) {
      f = (OnUpdateDisplayMenu) getPluginSymbol(plugin->info->info.g->onUpdateDisplay,
                                                plugin->info->details);
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

GtkWidget *
createPluginList() 
{
   /* Number of entries here should be the same as in set_column_width below and 
      as the number of elements in addPlugin().
   */
  static const gchar * const titles[] = {"Name", "Description", "Author", "Location", "Loaded", "Active"};
  GtkWidget *list;
  list = gtk_clist_new_with_titles(sizeof(titles)/sizeof(titles[0]), (gchar **) titles);

  gtk_clist_set_column_width(GTK_CLIST(list), 0, 100); 
  gtk_clist_set_column_width(GTK_CLIST(list), 1, 225); 
  gtk_clist_set_column_width(GTK_CLIST(list), 2, 150); 
  gtk_clist_set_column_width(GTK_CLIST(list), 3, 225); 
  gtk_clist_set_column_width(GTK_CLIST(list), 4,  50); 
  gtk_clist_set_column_width(GTK_CLIST(list), 5,  50); 

  return(list);
}

/*
 We should move to an interface more like Gnumeric's plugin
 info list.
 */
GtkWidget *
showPluginInfo(GList *plugins, GList *inputPlugins, ggobid *gg)
{
 GtkWidget *win, *main_vbox, *list;

  win = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  main_vbox = gtk_notebook_new();

  gtk_container_set_border_width(GTK_CONTAINER(main_vbox),0); 
  gtk_container_add(GTK_CONTAINER(win), main_vbox);
  gtk_widget_show(main_vbox);


  if(plugins) {
    list = createPluginList();
    addPlugins(plugins, list, gg, GENERAL_PLUGIN);
    gtk_notebook_append_page(GTK_NOTEBOOK(main_vbox), list, gtk_label_new("General"));
  }
  if(inputPlugins) {
    list = createPluginList();
    addPlugins(inputPlugins, list, gg, INPUT_PLUGIN);
    gtk_notebook_append_page(GTK_NOTEBOOK(main_vbox), list, gtk_label_new("Input Readers"));
  }


  gtk_widget_show_all(main_vbox);
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
  gint n = g_list_length(plugins), i;
  GGobiPluginInfo *plugin;

  for(i = 0; i < n ; i++) {
    plugin = (GGobiPluginInfo*) g_list_nth_data(plugins, i);
    switch(type) {
      case GENERAL_PLUGIN:
         addPlugin(plugin, list, gg);
       break;
       case INPUT_PLUGIN:
         addInputPlugin(plugin, list, gg);
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
    addPluginDetails(info->details, list, gg, isPluginActive(info, gg));
}

void
addInputPlugin(GGobiPluginInfo *info, GtkWidget *list, ggobid *gg)
{
    addPluginDetails(info->details, list, gg, true);
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
  if(!el || g_list_length(el) == 0) {
    return;
  }

  while(el) { 
    plugin = (PluginInstance *) el->data;
    if(plugin->info->info.g->onClose) {
      DLFUNC f =  getPluginSymbol(plugin->info->info.g->onClose, plugin->info->details);
      f(gg, plugin->info, plugin);
    }
    tmp = el;
    el = el->next;
    g_free(plugin);    
/*  g_free(tmp); */
  }
  gg->pluginInstances = NULL;
}



/*
 Determine if the plugin handles this mode.
 */
gboolean
pluginSupportsInputMode(const gchar *modeName, GGobiPluginInfo *pluginInfo)
{
   int i;
   for(i = 0; i < pluginInfo->info.i->numModeNames; i++) {
     if(strcmp(modeName, pluginInfo->info.i->modeNames[i]) == 0)
       return(true);
   }

   return(false);
}

GGobiPluginInfo *
runInteractiveInputPlugin(ggobid *gg)
{
  GGobiPluginInfo* plugin = NULL;
  GList *l = sessionOptions->info->inputPlugins;

  for(; l; l = l->next) {
    plugin =  (GGobiPluginInfo*) l->data;
    if(plugin->info.i->interactive) {
      if(!sessionOptions->data_type ||
	 pluginSupportsInputMode(sessionOptions->data_type, plugin))
      {
        InputGetDescription f;
        if(!loadPluginLibrary(plugin->details, plugin)) {
  	   g_printerr("Failed to load plugin %s\n", plugin->details->name);
   	   continue;
	}
        f = (InputGetDescription) getPluginSymbol(plugin->info.i->getDescription,
                                                  plugin->details);
        if(f) {
          InputDescription *desc;
          desc = f(NULL, sessionOptions->data_type, gg, plugin);
          if(desc && desc->desc_read_input) {
            gg->input = desc;
            desc->desc_read_input(desc, gg, plugin);
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

void
checkDLL()
{
  HINSTANCE handle;
  DLFUNC f;
  char *fileName = "testDLL.dll";
  fprintf(stderr, "Checking plugins on Windows\n");fflush(stderr);
  handle = dynload->open(fileName, NULL);
  if(!handle) {
    fprintf(stderr, "Cannot load %s\n", fileName);
    return;
  }

  f = (DLFUNC) dynload->resolve(handle, "testRun");
  if(!f) {
    fprintf(stderr, "Cannot find testRun routine\n");
    return;
  }

  f();
}


#include "externs.h"

gchar *XMLModeNames[] = {"xml", "url"};
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
  "GGobi core"
};

gchar *CSVModeNames[] = {"csv"};
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
  "Dongshin Kim (Iowa State University) & GGobi core"
};

gchar *ASCIIModeNames[] = {"ascii"};

GGobiInputPluginInfo ASCIIInputPluginInfo = {
	NULL,
	0,
	"",
	"",
	"read_ascii_input_description",
	false,
	read_ascii,
	read_ascii_input_description,
	isASCIIFile,
	ascii_data
};


GGobiPluginDetails ASCIIDetails = {
  "ASCII data reader",
  NULL,
  NULL,
  "Reads ASCII data in XGobi format of separate data, variable name, glyph, ... files",
  "GGobi core"
};



GGobiPluginInfo  *
createGGobiInputPluginInfo(GGobiInputPluginInfo *info, GGobiPluginDetails *details, gchar **modeNames, guint numModes)
{
  GGobiPluginInfo  *plugin; 
#ifdef WIN32
  static HINSTANCE ggobiLibrary = NULL;
 
  if(!details->dllName && !details->library) {

    if(!ggobiLibrary) {
      ggobiLibrary = ggobi_dlopen(sessionOptions->cmdArgs[0], details);

      if(!ggobiLibrary) {
	char buf[1000];
	g_printerr("Failed to load ggobi as library\n");
	ggobi_dlerror(buf, plugin);
	g_printerr(buf);
      }
    }

    details->library = ggobiLibrary;
  }
#endif

  plugin = (GGobiPluginInfo *) g_malloc(sizeof(GGobiPluginInfo));
  memset(plugin, '\0', sizeof(GGobiPluginInfo));

  plugin->type = INPUT_PLUGIN;
  plugin->info.i = info;
  plugin->details = details;

  if(modeNames) {
     guint i;
     plugin->info.i->modeNames = (gchar**) g_malloc(sizeof(gchar*) * numModes);
     plugin->info.i->numModeNames = numModes;
     for(i = 0; i < numModes; i++)
        plugin->info.i->modeNames[i] = g_strdup(modeNames[i]);
  }

  return(plugin);
}

/*
  Register the basic, built-in "plugins", specifically
  the input plugins for XML, CSV, ASCII data formats.
*/
void
registerDefaultPlugins(GGobiInitInfo *info)
{
  GGobiPluginInfo  *plugin; 
  
  plugin = createGGobiInputPluginInfo(&XMLInputPluginInfo, &XMLDetails, XMLModeNames, sizeof(XMLModeNames)/sizeof(XMLModeNames[0]));
  info->inputPlugins = g_list_append(info->inputPlugins, plugin);

  plugin = createGGobiInputPluginInfo(&CSVInputPluginInfo, &CSVDetails, CSVModeNames, sizeof(CSVModeNames)/sizeof(CSVModeNames[0]));
  info->inputPlugins = g_list_append(info->inputPlugins, plugin);

  plugin = createGGobiInputPluginInfo(&ASCIIInputPluginInfo, &ASCIIDetails, ASCIIModeNames, sizeof(ASCIIModeNames)/sizeof(ASCIIModeNames[0]));
  info->inputPlugins = g_list_append(info->inputPlugins, plugin);
}

const gchar DefaultUnknownInputModeName[] =  "unknown";

GList *
getInputPluginSelections(ggobid *gg)
{
       GList *els = NULL, *plugins;
       GGobiPluginInfo *plugin;
       int i, n, k;

       els = g_list_append(els, DefaultUnknownInputModeName);
       plugins = sessionOptions->info->inputPlugins;
       n = g_list_length(plugins);
       for(i = 0; i < n; i++) {
           char buf[5000];
	   plugin = g_list_nth_data(plugins, i);

	   for(k = 0; k < plugin->info.i->numModeNames; k++) {
/*XXX Need to free this. Catch destruction of the associated GtkList and free the elements of this list. */
   	       sprintf(buf, "%s (%s)", plugin->info.i->modeNames[k], plugin->details->name);
  	       els = g_list_append(els, g_strdup(buf));
	   }
       }

       return(els);
}

GGobiPluginInfo *
getInputPluginByModeNameIndex(gint which)
{
   gint ctr = 1, numPlugins, i; /* Start at 1 since guess/unknown is 0. */
   GList *plugins = sessionOptions->info->inputPlugins;
   GGobiPluginInfo *plugin;

   if(which < ctr)
      return(NULL);

   numPlugins = g_list_length(plugins);
   for(i = 0; i < numPlugins ; i++) {
       plugin = g_list_nth_data(plugins, i);
       if(which >= ctr && which < ctr + plugin->info.i->numModeNames)
          return(plugin);
       ctr += plugin->info.i->numModeNames;
   }
   
   return(NULL); /* Should never happen */
}

InputDescription *
callInputPluginGetDescription(const gchar *fileName, const gchar *modeName, GGobiPluginInfo *plugin, ggobid *gg)
{
        GGobiInputPluginInfo *info;
        InputGetDescription f;

	if(sessionOptions->verbose == GGOBI_VERBOSE) { 
	  g_printerr("Checking input plugin %s.\n", plugin->details->name);   
	}

	info = plugin->info.i;
	if(info->get_description_f)
    	      f = info->get_description_f;
	else
	      f = (InputGetDescription) getPluginSymbol(info->getDescription,
							plugin->details);

	if (f) {
            InputDescription *desc;
            desc = f(fileName, modeName, gg, plugin);
            if (desc)
              return (desc);
	} else if(sessionOptions->verbose == GGOBI_VERBOSE) { 
		g_printerr("No handler routine for plugin %s.: %s\n", plugin->details->name, info->getDescription);   
	}

	return(NULL);
}


