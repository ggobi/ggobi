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
  GtkWidget *options_menu, *submenu, *item;

/*
 * Options menu
*/
  submenu = submenu_make ("_Options", 'O', accel_group);
  options_menu = gtk_menu_new ();

  item = CreateMenuCheck (options_menu, "Show points",
    func, GINT_TO_POINTER (DOPT_POINTS), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);
  item = CreateMenuCheck (options_menu, "Show lines (undirected)",
    func, GINT_TO_POINTER (DOPT_SEGS_U), off, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);
  item = CreateMenuCheck (options_menu,
    "Show arrowheads (for directed lines)",
    func, GINT_TO_POINTER (DOPT_SEGS_D), off, gg);
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

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (submenu), options_menu);
  submenu_append (submenu, mbar);

  gtk_widget_show (submenu);
}

