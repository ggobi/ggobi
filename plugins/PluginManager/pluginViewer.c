#include "ggobi.h"
#include "plugin.h"
#include "externs.h"


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

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
                        "clicked", GTK_SIGNAL_FUNC (loadPluginFromFile), gg);

  gtk_signal_connect_object(GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->cancel_button),
                        "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer) file_selector);
                            

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
  gtk_signal_connect(GTK_OBJECT(entry), "activate",
                             GTK_SIGNAL_FUNC (select_plugin_file),
                             (gpointer) gg);


  return(true);
}
