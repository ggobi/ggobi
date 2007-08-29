#include "session.h"
#include "plugin.h"
#include "externs.h"

#include "GGStructSizes.c"

gboolean
onLoadPluginManager(gboolean init, GGobiPluginInfo *plugin)
{
  gboolean ok;
  ok = checkGGobiStructSizes();
  if(!ok) {
     g_printerr("Plugin %s (in %s) needs to be recompiled\n", plugin->details->name, plugin->details->dllName);
  }

  return(ok);
}

void
loadPluginFromFile(GtkWidget *btn, ggobid *gg)
{
 char *fileName;
 GtkWidget *fileSelect;
 GGobiPluginInfo *plugin;

 fileSelect = gtk_widget_get_toplevel(btn);
 fileName = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fileSelect));

 fprintf(stderr, "Reading plugin file %s\n", fileName);

 plugin = readPluginFile(fileName, sessionOptions->info);
 if(plugin) {
   registerPlugin(gg, plugin); 
 }

 gtk_widget_hide(fileSelect);
 gtk_widget_destroy(fileSelect);
}

void
select_plugin_file(GtkWidget *mitem, ggobid *gg)
{
  GtkWidget *file_selector;
  file_selector = gtk_file_selection_new("Select plugin description file");

  gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(file_selector));

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
                        "clicked", G_CALLBACK (loadPluginFromFile), gg);

  g_signal_connect_swapped(G_OBJECT (GTK_FILE_SELECTION(file_selector)->cancel_button),
                        "clicked", G_CALLBACK (gtk_widget_destroy), (gpointer) file_selector);
                            

  gtk_widget_show(file_selector);
}

#if 0
  win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(win), "GGobi plugin");
  gtk_widget_set_usize(GTK_WIDGET(win), 500, 400);
#endif

gboolean
addToMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{

  GtkWidget *entry;
  extern GtkWidget *GGobi_addToolsMenuItem (gchar *label, ggobid *gg);

  inst->data = NULL;
  inst->info = plugin;

   /* These could be done in an onLoad routine rather than each time 
      we create a new plugin instance of this type. No big deal at all.
    */

  entry = GGobi_addToolsMenuItem ("Load plugin", gg);
  g_signal_connect(G_OBJECT(entry), "activate",
                             G_CALLBACK (select_plugin_file),
                             (gpointer) gg);


  return(true);
}
