#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"

#include "ggvis.h"


void       close_ggvis_window(GtkWidget *w, PluginInstance *inst);
GtkWidget *create_ggvis_window(ggobid *gg, PluginInstance *inst);
void       show_ggvis_window(PluginInstance *inst, GtkWidget *widget);

gboolean
addToToolsMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  GtkWidget *entry;

  GtkItemFactory *factory;
  GtkWidget *tool_menu;

  inst->data = NULL;
  inst->info = plugin;

  entry = gtk_menu_item_new_with_label ("Graph Layout ...");
  gtk_widget_show (entry);
  gtk_signal_connect_object (GTK_OBJECT(entry), "activate",
                             GTK_SIGNAL_FUNC (show_ggvis_window),
                             (gpointer) inst);

  factory = gtk_item_factory_from_path ("<main>");
  tool_menu = gtk_item_factory_get_widget (factory, "<main>/Tools");

  /* Add a separator */
  CreateMenuItem (tool_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);

  gtk_menu_append (GTK_MENU (tool_menu), entry);

  return(true);
}


void
show_ggvis_window(PluginInstance *inst, GtkWidget *widget)
{
  if (g_slist_length(inst->gg->d) < 1) {
    fprintf(stderr, "No datasets to show\n");fflush(stderr);
    return;
  }

  if (inst->data == NULL) {
    GtkWidget *window;
    ggvisd *ggv = (ggvisd *) g_malloc (sizeof (ggvisd));
    extern void ggvis_init (ggvisd *);

    ggvis_init (ggv);

    window = create_ggvis_window (inst->gg, inst);
    gtk_object_set_data (GTK_OBJECT (window), "ggvisd", ggv);
    inst->data = window;  /*-- or this could be the ggvis structure --*/

  } else {
    gtk_widget_show_now ((GtkWidget*) inst->data);
  }
}

static void test_cb (GtkButton *button, ggobid* gg)
{
g_printerr ("perform a graph layout!\n");
}

GtkWidget *
create_ggvis_window(ggobid *gg, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *btn;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title(GTK_WINDOW(window), "ggvis");
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (close_ggvis_window), inst);

  main_vbox = gtk_vbox_new (FALSE,1);
  gtk_container_set_border_width (GTK_CONTAINER(main_vbox),0); 
  gtk_container_add (GTK_CONTAINER(window), main_vbox);

  btn = gtk_button_new_with_label ("test");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (test_cb), gg);
  gtk_box_pack_start (GTK_BOX (main_vbox), btn, false, false, 3);

  gtk_widget_show_all (window);

  return(window);
}


void close_ggvis_window(GtkWidget *w, PluginInstance *inst)
{
  inst->data = NULL;
}

void closeWindow(ggobid *gg, PluginInstance *inst)
{
  if(inst->data) {
    gtk_signal_disconnect_by_func(GTK_OBJECT(inst->data),
      GTK_SIGNAL_FUNC (close_ggvis_window), inst);
    gtk_widget_destroy((GtkWidget*) inst->data);
  }
}
