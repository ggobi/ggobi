/* scatterplot_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/


void
scatterplot_main_menus_make (GtkAccelGroup *accel_group, GtkSignalFunc func) {

/*
 * I/O menu
*/
  xg.app.scatterplot_mode_menu = gtk_menu_new ();

  CreateMenuItem (xg.app.scatterplot_mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL);
  CreateMenuItem (xg.app.scatterplot_mode_menu, "PROJECTION MODES:",
    "", "", NULL, NULL, NULL, NULL);

  CreateMenuItem (xg.app.scatterplot_mode_menu, "1D Plot",
    "^d", "", NULL, accel_group, func, GINT_TO_POINTER (P1PLOT));
  CreateMenuItem (xg.app.scatterplot_mode_menu, "XYPlot",
    "^x", "", NULL, accel_group, func, GINT_TO_POINTER (XYPLOT));
  CreateMenuItem (xg.app.scatterplot_mode_menu, "Rotation",
    "^r", "", NULL, accel_group, func, GINT_TO_POINTER (ROTATE));
  CreateMenuItem (xg.app.scatterplot_mode_menu, "2D Tour",
    "^t", "", NULL, accel_group, func, GINT_TO_POINTER (TOUR2D));
  CreateMenuItem (xg.app.scatterplot_mode_menu, "Correlation Tour",
    "^c", "", NULL, accel_group, func, GINT_TO_POINTER (COTOUR));

  /* Add a separator */
  CreateMenuItem (xg.app.scatterplot_mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL);
  CreateMenuItem (xg.app.scatterplot_mode_menu, "INTERACTION MODES:",
    "", "", NULL, NULL, NULL, NULL);

  CreateMenuItem (xg.app.scatterplot_mode_menu, "Scale",
    "^s", "", NULL, accel_group, func, GINT_TO_POINTER (SCALE));
  CreateMenuItem (xg.app.scatterplot_mode_menu, "Brush",
    "^b", "", NULL, accel_group, func, GINT_TO_POINTER (BRUSH));
  CreateMenuItem (xg.app.scatterplot_mode_menu, "Identify",
    "^i", "", NULL, accel_group, func, GINT_TO_POINTER (IDENT));
  CreateMenuItem (xg.app.scatterplot_mode_menu, "Edit Lines",
    "^l", "", NULL, accel_group, func, GINT_TO_POINTER (LINEED));
  CreateMenuItem (xg.app.scatterplot_mode_menu, "Move Points",
    "^m", "", NULL, accel_group, func, GINT_TO_POINTER (MOVEPTS));

  gtk_widget_show (xg.app.scatterplot_mode_menu);
}

