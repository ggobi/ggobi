#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "dspdesc.h"

#include "GGStructSizes.c"

void       close_dspdesc_window(GtkWidget *w, PluginInstance *inst);
GtkWidget *create_dspdesc_window(ggobid *gg, PluginInstance *inst);
void       show_dspdesc_window(GtkWidget *widget, PluginInstance *inst);

extern void desc_write_cb (GtkWidget *btn, PluginInstance *inst);

void
dspdesc_init (dspdescd *desc)
{
  desc->title = NULL;
  desc->filename = NULL;
}


gboolean
addToToolsMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  GtkWidget *entry;
  const gchar *lbl = "Save display description ...";

  inst->data = NULL;
  inst->info = plugin;
  inst->gg = gg;

  entry = GGobi_addToolsMenuItem ((gchar *)lbl, gg);
  gtk_signal_connect (GTK_OBJECT(entry), "activate",
                      GTK_SIGNAL_FUNC (show_dspdesc_window), inst);
  return(true);
}


void
show_dspdesc_window (GtkWidget *widget, PluginInstance *inst)
{
  dspdescd *desc;

  if (inst->data == NULL) {
    desc = (dspdescd *) g_malloc (sizeof (dspdescd));

    dspdesc_init (desc);
    inst->data = desc;

    create_dspdesc_window (inst->gg, inst);
    gtk_object_set_data (GTK_OBJECT (desc->window), "dspdescd", desc);

  } else {
    desc = (dspdescd *) inst->data;
    gtk_widget_show_now ((GtkWidget*) desc->window);
  }
}

dspdescd *
dspdescFromInst (PluginInstance *inst)
{
  dspdescd *desc = (dspdescd *) inst->data;
  return desc;
}

GtkWidget *
create_dspdesc_window(ggobid *gg, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *hb, *label, *entry, *btn;
  GtkTooltips *tips = gtk_tooltips_new ();
  dspdescd *desc = dspdescFromInst (inst); 

  /*-- I will probably have to get hold of this window, after which
       I can name all the other widgets --*/
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  desc->window = window;

  gtk_window_set_title(GTK_WINDOW(window), "Save display description");
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (close_dspdesc_window), inst);

  main_vbox = gtk_vbox_new (FALSE,1);
  gtk_container_set_border_width (GTK_CONTAINER(main_vbox), 5); 
  gtk_container_add (GTK_CONTAINER(window), main_vbox);

  /* label and entry widget for main title */
  hb = gtk_hbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (main_vbox), hb, true, true, 2);

  label = gtk_label_new ("Figure title");
  gtk_box_pack_start (GTK_BOX (hb), label, true, true, 2);

  entry = gtk_entry_new ();
  gtk_object_set_data (GTK_OBJECT(window), "TITLE", entry);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), entry,
    "Type in the figure title", NULL);
  gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

  /* label and entry widget for file name */
  hb = gtk_hbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (main_vbox), hb, true, true, 2);

  label = gtk_label_new ("File name");
  gtk_box_pack_start (GTK_BOX (hb), label, true, true, 2);

  entry = gtk_entry_new ();
  gtk_object_set_data (GTK_OBJECT(window), "FILENAME", entry);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), entry,
    "Type in the name of the file", NULL);
  gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);
  gtk_entry_set_text (GTK_ENTRY (entry), "ggdisplay.R");

  /* 'Do it' and 'Close' buttons */
  hb = gtk_hbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (main_vbox), hb, true, true, 2);

  btn = gtk_button_new_with_label ("Save");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "Create the file", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (desc_write_cb), inst);
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 2);


  btn = gtk_button_new_with_label ("Close");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "Close this window", NULL);
  /*
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (window_close_cb), inst);
  */
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 2);

  gtk_widget_show_all (window);

  return(window);
}


void close_dspdesc_window(GtkWidget *w, PluginInstance *inst)
{
  inst->data = NULL;
}

void closeWindow(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  if (inst->data) {
  dspdescd *desc = dspdescFromInst (inst); 
    /* I don't remember what this line is for -- dfs
    gtk_signal_disconnect_by_func(GTK_OBJECT(inst->data),
      GTK_SIGNAL_FUNC (close_dspdesc_window), inst);
    */
    gtk_widget_destroy (desc->window);
  }
}

