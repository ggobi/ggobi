#include "ggobi.h"
#include "plugin.h"

#include "GGStructSizes.c"


#include "parcoordsClass.h"
#include "externs.h"

/*
 The strategy is quite simple.
 When the plugin is loaded, we register an event handler for the creation of new ggobid's.
 When we receive notification of a new ggobid, then we register for the creation of any 
 new splotd's.  And when we receive these, we attach a menu to these.

 We can try to be clever about sharing a single menu or a single menu per splotd type.
 But that comes later.
*/

gint showMenu(GtkWidget *src, GdkEvent *event, GtkMenu *menu);

gint createParcoordsMenu(GtkWidget *menu, splotd *sp, ggobid *gg);

/*

 */
void
newSplotEvent(ggobid *gg, splotd *sp, gpointer udata)
{
    GtkWidget *menu;
    GtkWidget *item;


    menu = gtk_menu_new();

    if(GTK_IS_GGOBI_PARCOORDS_SPLOT(sp)) {
       createParcoordsMenu(menu, sp, gg);
    } else {

        item = gtk_menu_item_new_with_label("Add");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_menu_item_new_with_label("Delete");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    }


    gtk_widget_show_all(menu);

    gtk_signal_connect(GTK_OBJECT(sp), "event", (GtkSignalFunc) showMenu, menu);
}



gint
showMenu(GtkWidget *src, GdkEvent *event, GtkMenu *menu)
{
    if (event->type == GDK_BUTTON_PRESS) {
       GdkEventButton *bevent = (GdkEventButton *) event; 
       if(bevent->button == 3) {

           gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
                        bevent->button, bevent->time);

	   return(TRUE);
       }
    }
    return(FALSE);

}


/* Process a new GGobi being created by asking to hear about new splotd's */
void
newGGobiEvent(GtkObject *obj, ggobid *gg, gpointer udata)
{
   gtk_signal_connect(GTK_OBJECT(gg), "splot_new", newSplotEvent, udata);
}




/* Register interest in hearing about new ggobid's */
gboolean
onLoadPlotMenu(gboolean init, GGobiPluginInfo *plugin)
{
  gtk_signal_connect(GTK_OBJECT(getGGobiApp()), "new_ggobi", newGGobiEvent, NULL);

  return(true);
}




typedef struct {
   splotd *sp;
   guint id;
} ParCoordsPanelID;


void
switchParCoordsVariable(GtkWidget *src, ParCoordsPanelID *p)
{
  displayd *display = p->sp->displayptr;
  ggobid *gg = display->ggobi;
  p->sp->p1dvar = p->id;

  display_tailpipe (display, FULL, gg);

  varpanel_refresh (display, gg);  
}



gint
createParcoordsMenu(GtkWidget *menu, splotd *sp, ggobid *gg)
{
   datad *d = sp->displayptr->d;
   vartabled *el;
   GtkWidget *item;
   int i;

   for(i = 0; i < d->ncols; i++) {
      ParCoordsPanelID *id;

      el = vartable_element_get(i, d);
      item = gtk_menu_item_new_with_label(el->collab);
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);


      id = (ParCoordsPanelID *) g_malloc(sizeof(ParCoordsPanelID));
      id->sp =  sp;				 
      id->id = i;
      gtk_signal_connect(GTK_OBJECT(item), "activate", switchParCoordsVariable, id);
   }

   return(d->ncols);
}
