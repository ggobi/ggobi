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

/*
 * ViewMode menu
*/
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
    "^g", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (TOUR1D) : gg, gg);

  CreateMenuItem (gg->app.scatterplot_mode_menu, "2D Tour",
    "^t", "", NULL, accel_group, func,
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
  CreateMenuItem (gg->app.scatterplot_mode_menu, "Edit Edges",
    "^l", "", NULL, accel_group, func,
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

void
scatterplot_display_menus_make (displayd *display,
                                GtkAccelGroup *accel_group,
                                GtkSignalFunc func,
                                GtkWidget *mbar, ggobid *gg)
{
  GtkWidget *topmenu, *edges_menu, *options_menu;
  GtkWidget *item;
  gint nd = g_slist_length (gg->d);
  datad *e;
  datad *d = display->d;  /*-- this dataset --*/
  gint k, ne = 0;

  /*-- If this datad has ids, find the number of other datad's with edges --*/
  if (d->rowid.idv.nels > 0) {
    for (k=0; k<nd; k++) { 
      e = (datad*) g_slist_nth_data (gg->d, k);
      if (e != d && e->edge.n > 0)
        ne++;
    }

    /*-- if ne > 0, add edge set:  a single choice or multiple choices --*/
    if (ne > 0) {

      /*-- Edges menu --*/
      topmenu = submenu_make ("_Edges", 'E', accel_group);
      edges_menu = gtk_menu_new ();

      if (ne > 1) {  /*-- add cascading menu --*/
        GtkWidget *submenu, *anchor;
        gchar *lbl;
        extern void edgeset_add_cb (GtkWidget *w, datad *e);

        submenu = gtk_menu_new ();
        anchor = CreateMenuItem (edges_menu, "Select edge set",
          NULL, NULL, gg->main_menubar, NULL, NULL, NULL, NULL);

        for (k=0; k<nd; k++) { 
          e = (datad *) g_slist_nth_data (gg->d, k);
          if (e == d)
            continue;
          lbl = datasetName (e, k);
          item = CreateMenuItem (submenu, lbl,
            NULL, NULL, NULL, gg->main_accel_group,
            GTK_SIGNAL_FUNC (edgeset_add_cb), e, gg);
          gtk_object_set_data (GTK_OBJECT (item),
            "display", GINT_TO_POINTER (display));
          g_free (lbl);
        }

        gtk_menu_item_set_submenu (GTK_MENU_ITEM (anchor), submenu);

        /* Add a separator */
        CreateMenuItem (edges_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);
      }

      item = CreateMenuCheck (edges_menu,
        "Show edges (undirected)",
        func, GINT_TO_POINTER (DOPT_EDGES_U), off, gg);
      gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

      item = CreateMenuCheck (edges_menu,
        "Show 'arrowheads' (for directed edges)",
        func, GINT_TO_POINTER (DOPT_EDGES_A), off, gg);
      gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

      /* Add a separator */
      CreateMenuItem (edges_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

      item = CreateMenuCheck (edges_menu,
        "Show directed edges (both edges and 'arrowheads')",
        func, GINT_TO_POINTER (DOPT_EDGES_D), off, gg);
      gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

      gtk_menu_item_set_submenu (GTK_MENU_ITEM (topmenu), edges_menu);
      submenu_append (topmenu, mbar);
      gtk_widget_show (topmenu);
    }
  }


/*
 * Options menu
*/
  topmenu = submenu_make ("_Options", 'O', accel_group);
  options_menu = gtk_menu_new ();

  item = CreateMenuCheck (options_menu, "Show points",
    func, GINT_TO_POINTER (DOPT_POINTS), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);
/*
  item = CreateMenuCheck (options_menu, "Show missings",
    func, GINT_TO_POINTER (DOPT_MISSINGS), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);
*/

  /* Add a separator */
  CreateMenuItem (options_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  item = CreateMenuCheck (options_menu, "Show axes",
    func, GINT_TO_POINTER (DOPT_AXES), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);
  item = CreateMenuCheck (options_menu, "Center axes (3D+ modes)",
    func, GINT_TO_POINTER (DOPT_AXES_C), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (topmenu), options_menu);
  submenu_append (topmenu, mbar);
  gtk_widget_show (topmenu);
}

