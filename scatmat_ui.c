/* scatmat_ui.c */

#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <stdlib.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*----------------------------------------------------------------------*/
/*                       Callbacks                                      */
/*----------------------------------------------------------------------*/

static gchar *selection_mode_lbl[] = {"Replace", "Insert", "Append", "Delete"};
static void selection_mode_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;

  switch (indx) {
    case 0:
      cpanel->scatmat_selection_mode = VAR_REPLACE;
    break;
    case 1:
      cpanel->scatmat_selection_mode = VAR_INSERT;
    break;
    case 2:
      cpanel->scatmat_selection_mode = VAR_APPEND;
    break;
    case 3:
      cpanel->scatmat_selection_mode = VAR_DELETE;
    break;
  }
}

/*------------------------------------------------------------------------*/
/*                         Control panel                                  */
/*------------------------------------------------------------------------*/

void
cpanel_scatmat_make (ggobid *gg) {
  GtkWidget *vb, *lbl, *opt;
  
  gg->control_panel[SCATMAT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[SCATMAT]),
                                  5);

 /*-- option menu: selection mode --*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[SCATMAT]),
                      vb, false, false, 0);

  lbl = gtk_label_new ("Selection mode:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_widget_set_name (opt, "SCATMAT:sel_mode_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Selecting an unselected variable either replaces the variable in the current plot, inserts a new plot before the current plot, or appends a new plot after the last plot",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, selection_mode_lbl,
    sizeof (selection_mode_lbl) / sizeof (gchar *),
    (GtkSignalFunc) selection_mode_cb, "GGobi", gg);

  gtk_widget_show_all (gg->control_panel[SCATMAT]);
}


/*------------------------------------------------------------------------*/
/*                       Resetting the main menubar                       */
/*------------------------------------------------------------------------*/


void
scatmat_mode_menu_make (GtkAccelGroup *accel_group, GtkSignalFunc func,
  ggobid *gg, gboolean useIds) 
{
  /*-- I/O menu --*/
  gg->app.scatmat_mode_menu = gtk_menu_new ();

  CreateMenuItem (gg->app.scatmat_mode_menu, "Scatterplot Matrix",
    "^a", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (SCATMAT) : gg, gg);

  /*-- Add a separator --*/
  CreateMenuItem (gg->app.scatmat_mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuItem (gg->app.scatmat_mode_menu, "Scale",
    "^s", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (SCALE) : gg, gg);
  CreateMenuItem (gg->app.scatmat_mode_menu, "Brush",
    "^b", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (BRUSH) : gg, gg);
  CreateMenuItem (gg->app.scatmat_mode_menu, "Identify",
    "^i", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (IDENT) : gg, gg);
  CreateMenuItem (gg->app.scatmat_mode_menu, "Move Points",
    "^m", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (MOVEPTS) : gg, gg);

  gtk_widget_show (gg->app.scatmat_mode_menu);
}

/*--------------------------------------------------------------------*/
/*                      Control panel updating                        */
/*--------------------------------------------------------------------*/

/*-- there already exists scatmat_cpanel_init --*/

void
cpanel_scatmat_set (displayd *display, cpaneld *cpanel, ggobid *gg)
{
  GtkWidget *w;

  w = widget_find_by_name (gg->control_panel[SCATMAT],
                           "SCATMAT:sel_mode_option_menu");

  gtk_option_menu_set_history (GTK_OPTION_MENU(w),
                               cpanel->scatmat_selection_mode);
}
