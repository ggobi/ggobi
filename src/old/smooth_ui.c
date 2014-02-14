/* smooth_ui.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Eclipse Public License, which is distributed
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

static GtkWidget *window = NULL;
static gchar *smoother_lbl[] =
  { "Mean", "Median", "Nadaraya-Watson", "Spline" };

static void
smoother_cb (GtkWidget * w, ggobid * gg)
{
  gint indx = gtk_combo_box_get_active (GTK_COMBO_BOX (w));
  g_printerr ("cbd: %s\n", smoother_lbl[indx]);
}

static void
smooth_cb (GtkToggleButton * button)
{
  g_printerr ("active: %d\n", button->active);
}

static void
groups_cb (GtkToggleButton * button)
{
  g_printerr ("use groups: %d\n", button->active);
}

static void
window_cb (GtkToggleButton * button)
{
  g_printerr ("show window: %d\n", button->active);
}

void
width_cb (GtkAdjustment * adj, ggobid * gg)
{
  g_printerr ("width %f\n", adj->value);
}

/*
static void
hide_cb (GtkWidget *w ) {
  gtk_widget_hide (w);
}
*/

void
smooth_window_open (ggobid * gg)
{

  GtkWidget *tgl, *lbl;
  GtkWidget *vbox, *vb;
  GtkWidget *sbar, *opt;
  GtkObject *adj;
  if (window == NULL) {

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "smooth data");

    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (window), vbox);

/*
 * Smooth toggle
*/
    tgl = gtk_check_button_new_with_mnemonic ("_Smooth");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
                          "Add one or more smoothed lines to the current plot",
                          NULL);
    g_signal_connect (G_OBJECT (tgl), "toggled", G_CALLBACK (smooth_cb),
                      (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vbox), tgl, false, false, 3);

/*
 * smoothers option menu 
*/
    vb = gtk_vbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 1);

    lbl = gtk_label_new_with_mnemonic ("Smoothing f_unctions:"),
      gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vbox), lbl, false, false, 0);

    opt = gtk_combo_box_new_text ();
    gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), opt);
    gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
                          "Set the smoothing function", NULL);
    gtk_box_pack_start (GTK_BOX (vbox), opt, false, false, 0);
    populate_combo_box (opt, smoother_lbl, G_N_ELEMENTS (smoother_lbl),
                        G_CALLBACK (smoother_cb), gg);

/*
 * vbox for label and rangewidget
*/
    vb = gtk_vbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 1);

    lbl = gtk_label_new_with_mnemonic ("_Width:");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

    adj = gtk_adjustment_new (1.0, 0.0, 1.0, 0.01, .01, 0.0);
    g_signal_connect (G_OBJECT (adj), "value_changed",
                      G_CALLBACK (width_cb), gg);

    sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), sbar);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
                          "Set the width of the smoothing window", NULL);
    gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
    gtk_scale_set_digits (GTK_SCALE (sbar), 2);
    gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
    gtk_box_pack_start (GTK_BOX (vb), sbar, false, false, 0);

/*
 * Use color groups toggle
*/
    tgl = gtk_check_button_new_with_mnemonic ("Use _groups");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
                          "Add one smoothed line for each point color", NULL);
    g_signal_connect (G_OBJECT (tgl), "toggled",
                      G_CALLBACK (groups_cb), (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vbox), tgl, false, false, 3);

/*
 * Show smoothing window
*/
    tgl = gtk_check_button_new_with_mnemonic ("Sh_ow window");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
                          "Show the smoothing window on the scatterplot display",
                          NULL);
    g_signal_connect (G_OBJECT (tgl), "toggled", G_CALLBACK (window_cb),
                      (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vbox), tgl, false, false, 3);

    gtk_widget_show_all (window);
  }

  gdk_window_raise (window->window);
}
