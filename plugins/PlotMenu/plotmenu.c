#include "session.h"
#include "plugin.h"

#include "GGStructSizes.c"


#include "parcoordsClass.h"
#include "scatterplotClass.h"
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
gint createXYPlotMenu(GtkWidget *menu, splotd *sp, ggobid *gg);

int addVariableElements(GtkWidget *menu, GGobiData *d, splotd *sp, GtkSignalFunc f);
void destroyMenu(splotd *sp, GtkWidget *menu);

/*
 Callback that is invoked when a new splotd is created within the ggobi session.
 */
void
newSplotEvent(ggobid *gg, splotd *sp, gpointer udata)
{
    GtkWidget *menu;
    GtkWidget *item;


    menu = gtk_menu_new();

    if(GGOBI_IS_PARCOORDS_SPLOT(sp)) {
       createParcoordsMenu(menu, sp, gg);
    } else if(GGOBI_IS_SCATTER_SPLOT(sp) && GGOBI_IS_SCATTERPLOT_DISPLAY(GGOBI_SPLOT(sp)->displayptr)) {
       createXYPlotMenu(menu, sp, gg);
    } else {

        item = gtk_menu_item_new_with_label("Add");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_menu_item_new_with_label("Delete");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    }


    gtk_widget_show_all(menu);

    g_signal_connect(G_OBJECT(sp), "event", G_CALLBACK(showMenu), menu);
    g_signal_connect(G_OBJECT(sp), "destroy", G_CALLBACK(destroyMenu), menu);
}

void
destroyMenu(splotd *sp, GtkWidget *menu)
{
  
}

/*
  Popup the menu on the splotd.
*/
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
   g_signal_connect(G_OBJECT(gg), "splot_new", newSplotEvent, udata);
}




/* Register interest in hearing about new ggobid's */
gboolean
onLoadPlotMenu(gboolean init, GGobiPluginInfo *plugin)
{
  g_signal_connect(G_OBJECT(getGGobiApp()), "new_ggobi", newGGobiEvent, NULL);

  return(true);
}



/* A data structure that is used to inform the callback 
   of the splotd and the identity of the variable that is 
   being switched in the plot.
*/
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



/* 
  Handler
*/
void
switchXYPlotVariable(GtkWidget *src, ParCoordsPanelID *p, gboolean x)
{
  displayd *display = p->sp->displayptr;
  ggobid *gg = display->ggobi;
  if(x)
    p->sp->xyvars.x = p->id;
  else
    p->sp->xyvars.y = p->id;

  display_tailpipe (display, FULL, gg);

  varpanel_refresh (display, gg);  
}


/* Simple version of setting the variables for a scatterplot
  that knows it is setting the X variable.  This means it doesn't
  need to be passed which variable.
*/
void
switchXPlotVariable(GtkWidget *src, ParCoordsPanelID *p)
{
  switchXYPlotVariable(src, p, true);
}

/*
  Change the Y variable in the current scatterplot.
*/
void
switchYPlotVariable(GtkWidget *src, ParCoordsPanelID *p)
{
  switchXYPlotVariable(src, p, false);
}

/*
  Construct the menu for a parallel coordinates splotd.
*/

void
removeParcoordsSPlot(GtkWidget *menuItem, splotd *sp)
{
  gint prev = -1;
  displayd *dpy = sp->displayptr;
  ggobid *gg = dpy->ggobi;

  parcoords_add_delete_splot(&(dpy->cpanel), sp, sp->p1dvar, &prev, gg, dpy, VAR_DELETE);

  display_tailpipe (dpy, FULL, gg);
}


/* See varsel. */

void
insertParcoordsSPlot(GtkWidget *menuItem, ParCoordsPanelID *p)
{
  gint prev = -1;
  displayd *dpy = p->sp->displayptr;
  ggobid *gg = dpy->ggobi;

  parcoords_add_delete_splot(&(dpy->cpanel), p->sp, p->id, &prev, gg, dpy, VAR_INSERT);

  display_tailpipe (dpy, FULL, gg);

/*XXX Need to set the relevant button on the cpanel to say this is now on. 

? varpanel_refresh (dpy, gg);
*/
}

gint
createParcoordsMenu(GtkWidget *menu, splotd *sp, ggobid *gg)
{
   GGobiData *d = sp->displayptr->d;
   GtkWidget *item, *submenu;

   addVariableElements(menu, d, sp, switchParCoordsVariable);


   item = gtk_menu_item_new();
   gtk_menu_append(GTK_MENU(menu), item); 

   item = gtk_menu_item_new_with_label("Delete");
   gtk_menu_append(GTK_MENU(menu), item); 
   g_signal_connect(G_OBJECT(item), "activate", removeParcoordsSPlot, sp);

   item = gtk_menu_item_new_with_label("Insert");
   submenu = gtk_menu_new();
   addVariableElements(submenu, d, sp, insertParcoordsSPlot);
   gtk_menu_append(GTK_MENU(menu), item); 
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);


   return(d->ncols);
}


/*
  Construct the menu for a scatterplot splotd.
*/
gint
createXYPlotMenu(GtkWidget *menu, splotd *sp, ggobid *gg)
{
   GGobiData *d = sp->displayptr->d;
   GtkWidget *el, *submenu;

   el = gtk_menu_item_new_with_label("X");
   submenu = gtk_menu_new();
   addVariableElements(submenu, d, sp, switchXPlotVariable);
   gtk_menu_append(GTK_MENU(menu), el); 
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(el), submenu);

   el = gtk_menu_item_new_with_label("Y");
   submenu = gtk_menu_new();
   addVariableElements(submenu, d, sp, switchYPlotVariable);
   gtk_menu_append(GTK_MENU(menu), el); 
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(el), submenu);
   

   return(d->ncols);
}


/*
 Utility function for many menu constructions that loops over the
 variables in the dataset and constructs a menu item for each one
 and associates the callback function with each.
*/
int
addVariableElements(GtkWidget *menu, GGobiData *d, splotd *sp, GtkSignalFunc f)
{
   vartabled *el;
   GtkWidget *item;
   int i;
   for(i = 0; i < d->ncols; i++) {
      ParCoordsPanelID *id;

      el = vartable_element_get(i, d);
      item = gtk_menu_item_new_with_label(el->collab);
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);


      id = (ParCoordsPanelID *) g_malloc(sizeof(ParCoordsPanelID));
      id->sp =  sp;  id->id = i;

      g_signal_connect(G_OBJECT(item), "activate", f, id);
   }
   return(d->ncols);
}
