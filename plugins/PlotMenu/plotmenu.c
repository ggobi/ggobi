#include "ggobi.h"
#include "plugin.h"

#include "GGStructSizes.c"


/*
 The strategy is quite simple.
 When the plugin is loaded, we register an event handler for the creation of new ggobid's.
 When we receive notification of a new ggobid, then we register for the creation of any 
 new splotd's.  And when we receive these, we attach a menu to these.

 We can try to be clever about sharing a single menu or a single menu per splotd type.
 But that comes later.
*/


void
newSplotEvent(ggobid *gg, splotd *sp, gpointer udata)
{
	g_printerr("Adding menu to the new splot\n");
}

void
newGGobiEvent(GtkObject *obj, ggobid *gg, gpointer udata)
{
   g_printerr("Adding listener to new ggobid\n");
   gtk_signal_connect(GTK_OBJECT(gg), "splot_new", newSplotEvent, udata);
}



gboolean
onLoadPlotMenu(gboolean init, GGobiPluginInfo *plugin)
{
  g_printerr("<onLoadPlotMenu/>\n");
  gtk_signal_connect(GTK_OBJECT(getGGobiApp()), "new_ggobi", newGGobiEvent, NULL);

  return(true);
}





