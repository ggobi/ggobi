/* parcoords_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*----------------------------------------------------------------------*/
/*                       Callbacks                                      */
/*----------------------------------------------------------------------*/

static void ash_smoothness_cb (GtkAdjustment *adj, gpointer cbd) {
  g_printerr ("%f\n", adj->value);
}

static gchar *arrangement_lbl[] = {"Row", "Column"};
static void arrangement_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", arrangement_lbl[indx]);

  if (indx != current_display->cpanel.parcoords_arrangement)
    parcoords_reset_arrangement (current_display, indx);

  current_display->cpanel.parcoords_arrangement = indx;
}

static gchar *type_lbl[] = {"Texturing", "ASH", "Dotplot"};
static void type_cb (GtkWidget *w, gpointer cbd)
{
  cpaneld *cpanel = &current_display->cpanel;
  cpanel->p1d_type = GPOINTER_TO_INT (cbd);

  display_reproject (current_display);
}

static gchar *selection_mode_lbl[] = {"Replace", "Insert", "Append"};
static void selection_mode_cb (GtkWidget *w, gpointer cbd)
{
  cpaneld *cpanel = &current_display->cpanel;
  cpanel->parcoords_selection_mode = GPOINTER_TO_INT (cbd);
}

static gchar *showcases_lbl[] = {"All", "Labelled"};
static void showcases_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", showcases_lbl[indx]);
}

static gchar *varscale_lbl[] = {"Common", "Independent"};
static void varscale_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", varscale_lbl[indx]);
}

/*--------------------------------------------------------------------*/
/*                   Control panel section                            */
/*--------------------------------------------------------------------*/

void
cpanel_parcoords_make () {
  GtkWidget *vbox, *vb, *lbl, *sbar, *opt;
  GtkObject *adj;
  
  control_panel[PCPLOT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (control_panel[PCPLOT]), 5);

/*
 * arrangement of plots, row or column
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (control_panel[PCPLOT]), vb, false, false, 0);

  lbl = gtk_label_new ("Plot arrangement:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
    "When opening a new parallel coordinates display, arrange the 1d plots in a row or a column",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, arrangement_lbl,
                        sizeof (arrangement_lbl) / sizeof (gchar *),
                        arrangement_cb);
/*
 * option menu: selection mode
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (control_panel[PCPLOT]), vb, false, false, 0);

  lbl = gtk_label_new ("Selection mode:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
    "Selecting a variable either replaces the variable in the current plot (swapping if appropriate), inserts a new plot before the current plot, or appends a new plot after it",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, selection_mode_lbl,
                        sizeof (selection_mode_lbl) / sizeof (gchar *),
                        selection_mode_cb);

/*
 * option menu
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (control_panel[PCPLOT]), vb, false, false, 0);

  lbl = gtk_label_new ("Spreading method:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
    "Display either textured dot plots or average shifted histograms", NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, type_lbl,
                        sizeof (type_lbl) / sizeof (gchar *),
                        type_cb);
  /*-- this should be set to the value of cpanel->p1d_type --*/
  gtk_option_menu_set_history (GTK_OPTION_MENU (opt), DOTPLOT);

/*
 * ASH smoothness
*/
  vbox = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (control_panel[PCPLOT]), vbox,
    false, false, 0);

  lbl = gtk_label_new ("ASH smoothness:"),
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), lbl, false, false, 0);

  adj = gtk_adjustment_new (0.1, 0.0, 0.5, 0.01, .01, 0.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                      GTK_SIGNAL_FUNC (ash_smoothness_cb), NULL);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), sbar,
    "Adjust ASH smoothness", NULL);
  gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
  gtk_scale_set_digits (GTK_SCALE (sbar), 2);

  gtk_box_pack_start (GTK_BOX (vbox), sbar,
    false, false, 1);

/*
 * show cases: label and option menu
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

  lbl = gtk_label_new ("Show cases:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
    "Show all visible cases, or show only labelled cases", NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, showcases_lbl,
                        sizeof (showcases_lbl) / sizeof (gchar *),
                        showcases_cb);

/*
 * Variable scales
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

  lbl = gtk_label_new ("Scales:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
    "Scale variables (and variable groups) on a common scale, or independently",
     NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, varscale_lbl,
                        sizeof (varscale_lbl) / sizeof (gchar *),
                        varscale_cb);

  gtk_widget_show_all (control_panel[PCPLOT]);
}


/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/

GtkWidget *parcoords_mode_menu;

void
parcoords_main_menus_make (GtkAccelGroup *accel_group, GtkSignalFunc func) {

/*
 * I/O menu
*/
  parcoords_mode_menu = gtk_menu_new ();

  CreateMenuItem (parcoords_mode_menu, "Parallel Coordinates",
    "^c", "", NULL, accel_group, func, GINT_TO_POINTER (PCPLOT));

  /* Add a separator */
  CreateMenuItem (parcoords_mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL);

  CreateMenuItem (parcoords_mode_menu, "Brush",
    "^b", "", NULL, accel_group, func, GINT_TO_POINTER (BRUSH));
  CreateMenuItem (parcoords_mode_menu, "Identify",
    "^i", "", NULL, accel_group, func, GINT_TO_POINTER (IDENT));
  CreateMenuItem (parcoords_mode_menu, "Move Points",
    "^m", "", NULL, accel_group, func, GINT_TO_POINTER (MOVEPTS));

  gtk_widget_show (parcoords_mode_menu);
}

/*--------------------------------------------------------------------*/
/*                   End of main menubar section                      */  
/*--------------------------------------------------------------------*/
