#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"

/**
 When we initialize a new ggobid instance, the plugin adds entries
 to the Display menu to offer the different categorical plot types.
 We have to take care of multiple datasets and adding entries
 for each dataset.

 Each menu entry has its own event handler and responds to the user
 selecting that item.  This involves creating the display,
 updating the control-panel appropriately, etc.
 

 The aim is to make the tasks involved here simpler for other plugins
 by providing high-level functions within the core GGobi API.
*/


GtkWidget *addDisplayMenuItem(ggobid *gg, const char *label);

void
show_barplot_display(PluginInstance *inst, GtkWidget *widget)
{
    datad *data;

    data = (datad*) gtk_object_get_data(GTK_OBJECT(widget), "data");

    if(!data)
	return;

    return;
}

gboolean
addBarplotMenuItems(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{

      /*  */
  GtkWidget *entry;

  entry = GGobi_addDisplayMenuItem(gg, "Barplot");    
  gtk_signal_connect_object (GTK_OBJECT(entry), "activate",
                             GTK_SIGNAL_FUNC (show_barplot_display),
                             (gpointer) inst);

  return(true);
}



