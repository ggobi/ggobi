#include "plugin.h"
#include <dlfcn.h>

#include <stdio.h>

HINSTANCE ggobi_dlopen(const char *name, GGobiPluginInfo *plugin);
void ggobi_dlerror(char *buf, GGobiPluginInfo *plugin);

static Dynload unixDynload = { ggobi_dlopen, 
                        dlclose, 
                        dlsym,  /* warning because we use const char * */
                        ggobi_dlerror};
Dynload *dynload = &unixDynload;



HINSTANCE 
load_plugin_library(GGobiPluginInfo *plugin)
{
  HINSTANCE handle;

   handle = dynload->open(plugin->dllName, plugin);
   if(!handle) {
    char buf[1000];
      dynload->getError(buf, plugin);
      fprintf(stderr, "error on loading plugin library %s: %s", plugin->dllName, buf);fflush(stderr);
   }

   return(handle);
}

DLFUNC 
getPluginSymbol(const char *name, GGobiPluginInfo *plugin)
{
  return(dynload->resolve(plugin->library, name));
}


gboolean 
registerPlugins(ggobid *gg, GList *plugins)
{
  GList *el = plugins;
  OnCreate f;
  GGobiPluginInfo *plugin;
  gboolean ok = true;

    while(el) {
	plugin = (GGobiPluginInfo *) el->data;
        if(plugin->onCreate) {
	  f = (OnCreate) getPluginSymbol(plugin->onCreate, plugin);
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

void addPlugins(GList *plugins, GtkWidget *list);
void addPlugin(GGobiPluginInfo *info, GtkWidget *list);

/*
 We should move to an interface more like Gnumeric's plugin
 info list.
 */
GtkWidget *
showPluginInfo(GList *plugins)
{
 GtkWidget *win, *main_vbox, *list;
 static const gchar *titles[] = {"Name", "Description", "Author","Location"};

  win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  main_vbox=gtk_vbox_new(FALSE,1);
  gtk_container_set_border_width(GTK_CONTAINER(main_vbox),0); 
  gtk_container_add(GTK_CONTAINER(win), main_vbox);
  gtk_widget_show(main_vbox);


  list = gtk_clist_new_with_titles(sizeof(titles)/sizeof(titles[0]),
    (gchar **) titles);

  gtk_clist_set_column_width(GTK_CLIST(list), 0, 100); 
  gtk_clist_set_column_width(GTK_CLIST(list), 1, 225); 
  gtk_clist_set_column_width(GTK_CLIST(list), 2, 150); 
  gtk_clist_set_column_width(GTK_CLIST(list), 3, 225); 


  addPlugins(plugins, list);
  gtk_box_pack_start(GTK_BOX(main_vbox), list, TRUE, TRUE, 0);
  gtk_widget_show(list);
  gtk_widget_show(win);
 
  return(win); 
}

void
addPlugins(GList *plugins, GtkWidget *list)
{
 int n = g_list_length(plugins), i;
 GGobiPluginInfo *plugin;

 for(i = 0; i < n ; i++) {
   plugin = (GGobiPluginInfo*) g_list_nth_data(plugins, i);
   addPlugin(plugin, list);
 }
}

void
addPlugin(GGobiPluginInfo *info, GtkWidget *list)
{
  gchar **els = (gchar **) g_malloc(4*sizeof(gchar*));
  els[0] = info->name;
  els[1] = info->description;
  els[2] = info->author;
  els[3] = info->dllName;
  gtk_clist_append(GTK_CLIST(list), els);
}



void
closePlugins(ggobid *gg)
{
    GList *el, *tmp;
    PluginInstance *plugin;

    el = gg->pluginInstances;
    while(el) { 
        plugin = (PluginInstance *) el->data;
        if(plugin->info->onClose) {
	  DLFUNC f =  getPluginSymbol(plugin->info->onClose, plugin->info);
          f(gg, plugin);
	}
        tmp = el;
	el = el->next;
        g_free(plugin);    
/*        g_free(tmp); */
    }
    gg->pluginInstances = NULL;
}



/***************************************************************************/



HINSTANCE
ggobi_dlopen(const char *name, GGobiPluginInfo *plugin)
{
  return(dlopen(name, RTLD_LOCAL | RTLD_NOW ));
}

void
ggobi_dlerror(char *buf, GGobiPluginInfo *plugin)
{
  sprintf(buf, dlerror());
}
