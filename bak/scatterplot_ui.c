/* scatterplot_ui.c */

#include <gtk/gtk.h>

#include "vars.h"

/* external functions */
extern GtkWidget *CreateMenuItem (GtkWidget *, gchar *, gchar *, gchar *,
                                  GtkWidget *, GtkAccelGroup *,
                                  GtkSignalFunc, gpointer);
/* */

/**********************************************************************/
/******************* Resetting the main menubar ***********************/
/**********************************************************************/

GtkWidget *scatterplot_mode_menu;

void
make_scatterplot_menus (GtkAccelGroup *accel_group, GtkSignalFunc func) {

/*
 * I/O menu
*/
  scatterplot_mode_menu = gtk_menu_new ();

  CreateMenuItem (scatterplot_mode_menu, "1DPlot",
    "^d", "", NULL, accel_group, func, GINT_TO_POINTER (P1PLOT));
  CreateMenuItem (scatterplot_mode_menu, "XYPlot",
    "^x", "", NULL, accel_group, func, GINT_TO_POINTER (XYPLOT));
  CreateMenuItem (scatterplot_mode_menu, "Rotation",
    "^r", "", NULL, accel_group, func, GINT_TO_POINTER (ROTATE));
  CreateMenuItem (scatterplot_mode_menu, "Grand Tour",
    "^g", "", NULL, accel_group, func, GINT_TO_POINTER (GRTOUR));
  CreateMenuItem (scatterplot_mode_menu, "Correlation Tour",
    "^c", "", NULL, accel_group, func, GINT_TO_POINTER (COTOUR));

  /* Add a separator */
  CreateMenuItem (scatterplot_mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL);

  CreateMenuItem (scatterplot_mode_menu, "Scale",
    "^s", "", NULL, accel_group, func, GINT_TO_POINTER (SCALE));
  CreateMenuItem (scatterplot_mode_menu, "Brush",
    "^b", "", NULL, accel_group, func, GINT_TO_POINTER (BRUSH));
  CreateMenuItem (scatterplot_mode_menu, "Identify",
    "^i", "", NULL, accel_group, func, GINT_TO_POINTER (IDENT));
  CreateMenuItem (scatterplot_mode_menu, "Edit Lines",
    "^l", "", NULL, accel_group, func, GINT_TO_POINTER (LINEED));
  CreateMenuItem (scatterplot_mode_menu, "Move Points",
    "^m", "", NULL, accel_group, func, GINT_TO_POINTER (MOVEPTS));

  gtk_widget_show (scatterplot_mode_menu);
}

/**********************************************************************/
/******************* End of main menubar section **********************/
/**********************************************************************/
