/*-- parcoords_ui.c --*/
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"
#include "utils_ui.h"

/*----------------------------------------------------------------------*/
/*                       Callbacks                                      */
/*----------------------------------------------------------------------*/

static void
ash_smoothness_cb (GtkAdjustment * adj, GGobiSession * gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;

  cpanel->p1d.nASHes = (gint)
    ((gdouble) cpanel->p1d.nbins * (adj->value / 2.0));

  display_tailpipe (gg->current_display, FULL, gg);
}

static gchar *arrangement_lbl[] = { "Row", "Column" };
static void
arrangement_cb (GtkWidget * w, GGobiSession * gg)
{
  gint indx = gtk_combo_box_get_active (GTK_COMBO_BOX (w));

  if (indx != gg->current_display->cpanel.parcoords_arrangement)
    parcoords_reset_arrangement (gg->current_display, indx, gg);

  gg->current_display->cpanel.parcoords_arrangement = indx;
}

static gchar *type_lbl[] = { "Texturing", "ASH", "Dotplot" };
static void
type_cb (GtkWidget * w, GGobiSession * gg)
{
  cpaneld *cpanel;
  cpanel = &gg->current_display->cpanel;
  cpanel->p1d.type = gtk_combo_box_get_active (GTK_COMBO_BOX (w));

  display_tailpipe (gg->current_display, FULL, gg);
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

GtkWidget *
cpanel_parcoords_make (GGobiSession * gg)
{
  modepaneld *panel;
  GtkWidget *vbox, *vb, *lbl, *sbar, *opt;
  GtkObject *adj;

  panel = (modepaneld *) g_malloc (sizeof (modepaneld));
  gg->control_panels = g_list_append (gg->control_panels, (gpointer) panel);
  panel->name = g_strdup ("PCPLOT");

  panel->w = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (panel->w), 5);

/*
 * arrangement of plots, row or column
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (panel->w), vb, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("Plot _arrangement:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), opt);
  gtk_widget_set_name (opt, "PCPLOT:sel_mode_option_menu");
  //gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
                        "When opening a new parallel coordinates display, arrange the 1d plots in a row or a column",
                        NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_combo_box (opt, arrangement_lbl, G_N_ELEMENTS (arrangement_lbl),
                      G_CALLBACK (arrangement_cb), gg);

/*
 * option menu
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (panel->w), vb, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("Sp_reading method:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_combo_box_new_text ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
                        "Display either textured dot plots or average shifted histograms",
                        NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_combo_box (opt, type_lbl, G_N_ELEMENTS (type_lbl),
                      G_CALLBACK (type_cb), gg);
  /*-- this should be set to the value of cpanel->p1d_type --*/
  gtk_combo_box_set_active (GTK_COMBO_BOX (opt), DOTPLOT);

/*
 * ASH smoothness
*/
  vbox = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (panel->w), vbox, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("ASH s_moothness:"),
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), lbl, false, false, 0);

  adj = gtk_adjustment_new (0.19, 0.02, 0.5, 0.01, .01, 0.0);
  g_signal_connect (G_OBJECT (adj), "value_changed",
                    G_CALLBACK (ash_smoothness_cb), gg);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), sbar);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
                        "Adjust ASH smoothness", NULL);
  gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
  gtk_scale_set_digits (GTK_SCALE (sbar), 2);

  gtk_box_pack_start (GTK_BOX (vbox), sbar, false, false, 1);

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
    G_CALLBACK(showcases_cb), "GGobi", gg);
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
    G_CALLBACK(varscale_cb), "GGobi", gg);
*/

  gtk_widget_show_all (panel->w);

  return (panel->w);
}


/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/

static const gchar *mode_ui_str =
  "<ui>"
  "	<menubar>"
  "		<menu action='IMode'>"
  "			<menuitem action='DefaultIMode'/>"
  "			<separator/>"
  "			<menuitem action='Brush'/>"
  "			<menuitem action='Identify'/>" "		</menu>" "	</menubar>" "</ui>";

const gchar *
parcoords_mode_ui_get (displayd * dsp)
{
  return (mode_ui_str);
}

/*
  The useIds indicates whether the callback data should be integers
  identifying the menu item or the global gg.
  At present, this is always false.
  See scatmat_mode_menu_make and scatterplot_mode_menu_make.
 */

/*--------------------------------------------------------------------*/
/*                   End of main menubar section                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                      Control panel updating                        */
/*--------------------------------------------------------------------*/

/*-- there already exists parcoords_cpanel_init --*/

void
cpanel_parcoords_set (displayd * display, cpaneld * cpanel, GtkWidget * panel,
                      GGobiSession * gg)
{
}
