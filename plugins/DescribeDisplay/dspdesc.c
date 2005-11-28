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
void       show_dspdesc_window(GtkAction *action, PluginInstance *inst);

static void window_close (PluginInstance *inst);

extern void desc_write (PluginInstance *inst);

void
dspdesc_init (dspdescd *desc)
{
  desc->title = NULL;
  desc->filename = NULL;
}


/*
  Should it not (also?) go on the display's File menu?
  The action is for the display.
*/
gboolean
addToToolsMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  static GtkActionEntry entry = {
	"DescribeDisplay", NULL, "_Save display description", NULL, "Save an S-language description of this display", 
		G_CALLBACK (show_dspdesc_window)
  };
  
  inst->data = NULL;
  inst->info = plugin;
  inst->gg = gg;

  GGOBI(addToolAction)(&entry, (gpointer)inst, gg);
  
  return(true);
}


void
show_dspdesc_window (GtkAction *action, PluginInstance *inst)
{
  dspdescd *desc;

  if (inst->data == NULL) {
    desc = (dspdescd *) g_malloc (sizeof (dspdescd));

    dspdesc_init (desc);
    inst->data = desc;

    create_dspdesc_window (inst->gg, inst);
    g_object_set_data(G_OBJECT (desc->window), "dspdescd", desc);

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
  GtkWidget *window, *hb, *label, *entry;
  GtkTooltips *tips = gtk_tooltips_new ();
  dspdescd *desc = dspdescFromInst (inst); 

  window = gtk_file_chooser_dialog_new("Save display description", NULL, 
  	GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, 
	GTK_STOCK_CLOSE, GTK_RESPONSE_REJECT, NULL);
	
  desc->window = window;

  /* label and entry widget for main title */
  hb = gtk_hbox_new (false, 1);
  
  label = gtk_label_new_with_mnemonic ("Figure _title: ");
  gtk_box_pack_start (GTK_BOX (hb), label, false, false, 2);

  entry = gtk_entry_new ();
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), entry);
  g_object_set_data(G_OBJECT(window), "TITLE", entry);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), entry,
    "Type in the figure title", NULL);
  gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);
  gtk_widget_show_all(hb);
  
  gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(window), hb);
	
  if (gtk_dialog_run(GTK_DIALOG(window)) == GTK_RESPONSE_ACCEPT)
	desc_write(inst);
	
  window_close(inst);
	
  return(window);
}

void
window_close(PluginInstance *inst)
{
	if(inst->data) {
	    dspdescd *desc = (dspdescd *) inst->data;
     	    gtk_widget_hide(desc->window);
	}
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
    g_signal_handlers_disconnect_by_func(G_OBJECT(inst->data),
      G_CALLBACK (close_dspdesc_window), inst);
    */
    gtk_widget_destroy (desc->window);
  }
}

