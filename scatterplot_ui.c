/* scatterplot_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/


void
scatterplot_main_menus_make (GtkAccelGroup *accel_group, GtkSignalFunc func, ggobid *gg, gboolean useIds) {

/*
 * I/O menu
*/
  gg->app.scatterplot_mode_menu = gtk_menu_new ();

  CreateMenuItem (gg->app.scatterplot_mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "PROJECTION MODES:",
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuItem (gg->app.scatterplot_mode_menu, "1D Plot",
    "^d", "", NULL, accel_group, func, useIds ? GINT_TO_POINTER  (P1PLOT) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "XYPlot",
    "^x", "", NULL, accel_group, func, useIds ? GINT_TO_POINTER  (XYPLOT) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "Rotation",
    "^r", "", NULL, accel_group, func, useIds ? GINT_TO_POINTER  (ROTATE) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "2D Tour",
    "^t", "", NULL, accel_group, func, useIds ? GINT_TO_POINTER  (TOUR2D) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "Correlation Tour",
    "^c", "", NULL, accel_group, func, useIds ? GINT_TO_POINTER  (COTOUR) : gg, gg);

  /* Add a separator */
  CreateMenuItem (gg->app.scatterplot_mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "INTERACTION MODES:",
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuItem (gg->app.scatterplot_mode_menu, "Scale",
    "^s", "", NULL, accel_group, func, useIds ? GINT_TO_POINTER (SCALE) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "Brush",
    "^b", "", NULL, accel_group, func, useIds ? GINT_TO_POINTER (BRUSH) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "Identify",
    "^i", "", NULL, accel_group, func, useIds ? GINT_TO_POINTER (IDENT) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "Edit Lines",
    "^l", "", NULL, accel_group, func, useIds ? GINT_TO_POINTER (LINEED) : gg, gg);
  CreateMenuItem (gg->app.scatterplot_mode_menu, "Move Points",
    "^m", "", NULL, accel_group, func, useIds ? GINT_TO_POINTER (MOVEPTS) : gg, gg);

  gtk_widget_show (gg->app.scatterplot_mode_menu);
}

