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

/**
 Callback for when the user selects the "Barplot" menu item on the
 "Display" menu.
  This is called with the plugin instance as the first argument and 
  from that, we can get the ggobid instance. We have also arranged
  that the datad of interest associated with the "Barplot" menu item
  is available from the menu item widget. Given this, we can generate
  the barplot.  (For now, I use the first two variables in the dataset
  and create a parallel coordinates plot!)

 */
void
show_barplot_display(PluginInstance *inst, GtkWidget *widget)
{
    datad *data;
    displayd *display;

    if(ValidateGGobiRef(inst->gg, false) == NULL) {
	return;
    }

    data = (datad*) gtk_object_get_data(GTK_OBJECT(widget), "data");
    if(!data)
	return;

    {
     int nselected_vars = 2;
     gint *selected_vars;
      selected_vars = (gint *)g_malloc(2 * sizeof(gint));
      selected_vars[0] = 0;
      selected_vars[1] = 1;

      GGobi_newParCoords(selected_vars, nselected_vars, data, inst->gg);
      varpanel_refresh (inst->gg);
    }

    return;
}

/**
 Add an entry to the Display menu with the label `Barplot'
 and arrange to have show_barplot_display() called when the
 user selects this entry.
 */
gboolean
addBarplotMenuItems(ggobid *gg, PluginInstance *inst)
{


  GtkWidget *entry;

  entry = GGobi_addDisplayMenuItem(gg, "Barplot");    
  gtk_signal_connect_object (GTK_OBJECT(entry), "activate",
                             GTK_SIGNAL_FUNC (show_barplot_display),
                             (gpointer) inst);

  return(true);
}



