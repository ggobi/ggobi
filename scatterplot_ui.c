/* scatterplot_ui.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/

void
scatterplot_mode_menu_make (GtkAccelGroup *accel_group, GtkSignalFunc func,
			    ggobid *gg, gboolean useIds)
{

  /*-- ViewMode menu --*/
  gg->app.scatterplot_mode_menu = gtk_menu_new ();

  CreateMenuItem (gg->app.scatterplot_mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "PROJECTION MODES:",
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuItem (gg->app.scatterplot_mode_menu, "1D Plot",
    "^d", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (P1PLOT) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "XYPlot",
    "^x", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (XYPLOT) : gg, gg);

  CreateMenuItem (gg->app.scatterplot_mode_menu, "1D Tour",
    "^t", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (TOUR1D) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "Rotation",
    "^r", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (TOUR2D3) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "2D Tour",
    "^g", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (TOUR2D) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "2x1D Tour",
    "^c", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (COTOUR) : gg, gg);

  /* Add a separator */
  CreateMenuItem (gg->app.scatterplot_mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "INTERACTION MODES:",
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuItem (gg->app.scatterplot_mode_menu, "Scale",
    "^s", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (SCALE) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "Brush",
    "^b", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (BRUSH) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "Identify",
    "^i", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (IDENT) : gg, gg);
#ifdef EDIT_EDGES_IMPLEMENTED
  CreateMenuItem (gg->app.scatterplot_mode_menu, "Edit edges",
    "^e", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (EDGEED) : gg, gg);
#endif
  CreateMenuItem (gg->app.scatterplot_mode_menu, "Move Points",
    "^m", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (MOVEPTS) : gg, gg);

  gtk_widget_show (gg->app.scatterplot_mode_menu);
}

/*--------------------------------------------------------------------*/
/*                   Setting the display menubar                      */
/*--------------------------------------------------------------------*/

/*
 * This handles the initialization of the edge menu item and menu,
 * and it should also be called whenever the number of edge sets
 * might have changed:  when an edge set is added or removed, and
 * when a datad is added or removed.
*/
void
scatterplot_display_edge_menu_update (displayd *display,
                                      GtkAccelGroup *accel_group,
                                      GtkSignalFunc func, ggobid *gg)
{
  datad *d = display->d;  /*-- this dataset --*/
  gint nd = g_slist_length (gg->d);
  datad *e;
  gint k, ne = 0;
  GtkWidget *item;

  /*-- If this datad has ids, find the number of other datad's with edges --*/
  if (d->rowIds) {
    for (k=0; k<nd; k++) { 
      e = (datad*) g_slist_nth_data (gg->d, k);
      if (/* e != d && */ e->edge.n > 0)
        ne++;
    }
  }

  /*-- remove any existing submenu --*/
  if (display->edge_item != NULL && display->edge_menu != NULL) {
    gtk_menu_item_remove_submenu (GTK_MENU_ITEM (display->edge_item));
    display->edge_menu = NULL;
    if (ne < 1) {
      /*-- destroy menu item if there are no edge sets --*/
      gtk_widget_destroy (display->edge_item);
      display->edge_item = NULL;
    }
  } else {
    /*-- create the edge menu item if there is at least one edge set --*/
    if (ne > 0) {
      if (display->edge_item == NULL) {
        display->edge_item = submenu_make ("_Edges", 'E',
          gg->main_accel_group);
        submenu_insert (display->edge_item, display->menubar, 1);
      }
    }
  }

  /*-- then build the new menu if appropriate --*/
  if (ne) {
    /*-- build the menu --*/
    display->edge_menu = gtk_menu_new ();

    /*-- if there's only one edge set, there's no need for this menu --*/
    if (ne > 1) {  /*-- add cascading menu --*/
      GtkWidget *submenu, *anchor;
      gchar *lbl;

      submenu = gtk_menu_new ();
      anchor = CreateMenuItem (display->edge_menu, "Select edge set",
        NULL, NULL, gg->main_menubar, NULL, NULL, NULL, NULL);

      for (k=0; k<nd; k++) { 
        e = (datad *) g_slist_nth_data (gg->d, k);
        if (e == d)
          continue;
        else if (e->edge.n > 0) {
          lbl = datasetName (e, gg);
          item = CreateMenuItem (submenu, lbl,
            NULL, NULL, NULL, gg->main_accel_group,
            GTK_SIGNAL_FUNC (edgeset_add_cb), e, gg);
          gtk_object_set_data (GTK_OBJECT (item),
            "display", GINT_TO_POINTER (display));
          g_free (lbl);
        }
      }

      gtk_menu_item_set_submenu (GTK_MENU_ITEM (anchor), submenu);

      /* Add a separator */
      CreateMenuItem (display->edge_menu, NULL, "", "",
                      NULL, NULL, NULL, NULL, gg);
    } /*-- end of adding cascading menu --*/

    /*
     * The edge options are handled like other display options,
     * and their callback function is display_options_cb.
     * I may want to change that, but leave it for now.
    */
    item = CreateMenuCheck (display->edge_menu,
      "Show edges (undirected)",
      display_options_cb, GINT_TO_POINTER (DOPT_EDGES_U),
      display->options.edges_undirected_show_p, gg);
    gtk_widget_set_name (item, "DISPLAY MENU: show undirected edges");
    gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

    item = CreateMenuCheck (display->edge_menu,
      "Show 'arrowheads' (for directed edges)",
      display_options_cb, GINT_TO_POINTER (DOPT_EDGES_A),
      display->options.edges_arrowheads_show_p, gg);
    gtk_widget_set_name (item, "DISPLAY MENU: show arrowheads");
    gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

    /* Add a separator */
    CreateMenuItem (display->edge_menu, NULL, "", "",
                    NULL, NULL, NULL, NULL, gg);

    item = CreateMenuCheck (display->edge_menu,
      "Show directed edges (both edges and 'arrowheads')",
      display_options_cb, GINT_TO_POINTER (DOPT_EDGES_D),
      display->options.edges_directed_show_p, gg);
    gtk_widget_set_name (item, "DISPLAY MENU: show directed edges");
    gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

    gtk_menu_item_set_submenu (GTK_MENU_ITEM (display->edge_item),
      display->edge_menu);
  }
}

void
scatterplot_display_menus_make (displayd *display,
                                GtkAccelGroup *accel_group,
                                GtkSignalFunc func, ggobid *gg)
{
  GtkWidget *topmenu, *options_menu;
  GtkWidget *item;

  display->edge_item = NULL;
  display->edge_menu = NULL;
  scatterplot_display_edge_menu_update (display, accel_group, func, gg);

  /*-- Options menu --*/
  topmenu = submenu_make ("_Options", 'O', accel_group);
  /*-- add a tooltip --*/
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), topmenu,
    "Options menu for this display", NULL);

  options_menu = gtk_menu_new ();

  item = CreateMenuCheck (options_menu, "Show points",
    func, GINT_TO_POINTER (DOPT_POINTS), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

  /*-- Add a separator --*/
  CreateMenuItem (options_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  item = CreateMenuCheck (options_menu, "Show axes",
    func, GINT_TO_POINTER (DOPT_AXES), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

  item = CreateMenuCheck (options_menu, "Show 2D tour axes as text",
    func, GINT_TO_POINTER (DOPT_AXESLAB), off, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

  item = CreateMenuCheck (options_menu, "Show 2D tour proj vals",
    func, GINT_TO_POINTER (DOPT_AXESVALS), off, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (topmenu), options_menu);
  submenu_append (topmenu, display->menubar);
  gtk_widget_show (topmenu);
}

