/*-- parcoords_ui.c --*/
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

/*----------------------------------------------------------------------*/
/*                       Callbacks                                      */
/*----------------------------------------------------------------------*/

static void
ash_smoothness_cb (GtkAdjustment *adj, ggobid *gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;

  cpanel->p1d.nASHes = (gint)
    ((gfloat) cpanel->p1d.nbins * (adj->value / 2.0));

  display_tailpipe (gg->current_display, FULL, gg);
}

static gchar *arrangement_lbl[] = {"Row", "Column"};
static void arrangement_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  ggobid *gg = GGobiFromWidget(w, true);

  if (indx != gg->current_display->cpanel.parcoords_arrangement)
    parcoords_reset_arrangement (gg->current_display, indx, gg);

  gg->current_display->cpanel.parcoords_arrangement = indx;
}

static gchar *type_lbl[] = {"Texturing", "ASH", "Dotplot"};
static void type_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel;
    cpanel = &gg->current_display->cpanel;
  cpanel->p1d.type = GPOINTER_TO_INT (cbd);

  display_tailpipe (gg->current_display, FULL, gg);
}

static gchar *selection_mode_lbl[] = {"Replace", "Insert", "Append", "Delete"};
static void selection_mode_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->parcoords_selection_mode = GPOINTER_TO_INT (cbd);
}

/*
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
*/

/*--------------------------------------------------------------------*/
/*                   Control panel section                            */
/*--------------------------------------------------------------------*/

void
cpanel_parcoords_make (ggobid *gg) {
  GtkWidget *vbox, *vb, *lbl, *sbar, *opt;
  GtkObject *adj;
  
  gg->control_panel[PCPLOT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[PCPLOT]), 5);

/*
 * arrangement of plots, row or column
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[PCPLOT]), vb, false, false, 0);

  lbl = gtk_label_new ("Plot arrangement:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_widget_set_name (opt, "PCPLOT:sel_mode_option_menu");
  gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "When opening a new parallel coordinates display, arrange the 1d plots in a row or a column",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, arrangement_lbl,
                        sizeof (arrangement_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) arrangement_cb, gg);
/*
 * option menu: selection mode
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[PCPLOT]), vb, false, false, 0);

  lbl = gtk_label_new ("Selection mode:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Selecting an unselected variable either replaces the variable in the current plot, inserts a new plot before the current plot, or appends a new plot after the last plot",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, selection_mode_lbl,
                        sizeof (selection_mode_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) selection_mode_cb, gg);

/*
 * option menu
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[PCPLOT]), vb, false, false, 0);

  lbl = gtk_label_new ("Spreading method:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Display either textured dot plots or average shifted histograms", NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, type_lbl,
                        sizeof (type_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) type_cb, gg);
  /*-- this should be set to the value of cpanel->p1d_type --*/
  gtk_option_menu_set_history (GTK_OPTION_MENU (opt), DOTPLOT);

/*
 * ASH smoothness
*/
  vbox = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[PCPLOT]), vbox,
    false, false, 0);

  lbl = gtk_label_new ("ASH smoothness:"),
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), lbl, false, false, 0);

  adj = gtk_adjustment_new (0.19, 0.02, 0.5, 0.01, .01, 0.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                      GTK_SIGNAL_FUNC (ash_smoothness_cb), gg);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
    "Adjust ASH smoothness", NULL);
  gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
  gtk_scale_set_digits (GTK_SCALE (sbar), 2);

  gtk_box_pack_start (GTK_BOX (vbox), sbar,
    false, false, 1);

/*
 * show cases: label and option menu
*/
/*
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

  lbl = gtk_label_new ("Show cases:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Show all visible cases, or show only labelled cases", NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, showcases_lbl,
                        sizeof (showcases_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) showcases_cb, gg);
*/

/*
 * Variable scales
*/
/*
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

  lbl = gtk_label_new ("Scales:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Scale variables (and variable groups) on a common scale, or independently",
     NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, varscale_lbl,
                        sizeof (varscale_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) varscale_cb, gg);
*/

  gtk_widget_show_all (gg->control_panel[PCPLOT]);
}


/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/


/*
  The useIds indicates whether the callback data should be integers
  identifying the menu item or the global gg.
  At present, this is always false.
  See scatmat_mode_menu_make and scatterplot_mode_menu_make.
 */
void
parcoords_mode_menu_make (GtkAccelGroup *accel_group, GtkSignalFunc func,
  ggobid *gg, gboolean useIds)
{

  /*-- I/O menu --*/
  gg->parcoords.mode_menu = gtk_menu_new ();

  CreateMenuItem (gg->parcoords.mode_menu, "Parallel Coordinates",
    "^p", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (PCPLOT) : gg, gg);

  /* Add a separator */
  CreateMenuItem (gg->parcoords.mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);

  /*-- ViewMode menu --*/
  CreateMenuItem (gg->parcoords.mode_menu, "Brush",
    "^b", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (BRUSH) : gg, gg);
  CreateMenuItem (gg->parcoords.mode_menu, "Identify",
    "^i", "", NULL, accel_group,
    func, useIds ? GINT_TO_POINTER (IDENT) : gg, gg);
/*
  CreateMenuItem (gg->parcoords.mode_menu, "Move Points",
    "^m", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (MOVEPTS) : gg, gg);
*/

  gtk_widget_show (gg->parcoords.mode_menu);
}

/*--------------------------------------------------------------------*/
/*                   End of main menubar section                      */  
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                      Control panel updating                        */
/*--------------------------------------------------------------------*/

/*-- there already exists parcoords_cpanel_init --*/

void
cpanel_parcoords_set (cpaneld *cpanel, ggobid *gg)
{
  GtkWidget *w;

  w = widget_find_by_name (gg->control_panel[PCPLOT],
                           "PCPLOT:sel_mode_option_menu");

  gtk_option_menu_set_history (GTK_OPTION_MENU(w),
                               cpanel->parcoords_selection_mode);
}
