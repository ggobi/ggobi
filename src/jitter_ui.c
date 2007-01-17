/* jitter_ui.c */
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

void
jitter_vars_init (GGobiStage * d)
{
  d->jitter.type = UNIFORM;
  d->jitter.convex = true;
}

static void
jitter_cb (GtkButton * w, GGobiSession * gg)
{
  GtkWidget *tree_view =
    get_tree_view_from_object (G_OBJECT (gg->jitter_ui.window));
  GGobiStage *d =
    (GGobiStage *) g_object_get_data (G_OBJECT (tree_view), "datad");
  gint *vars;                   // = (gint *) g_malloc (d->n_cols * sizeof(gint));
  gint nvars;

  vars = get_selections_from_tree_view (tree_view, &nvars);

  if (nvars) {
    rejitter (vars, nvars, d, gg);
    g_free (vars);
  }
}

/*
 * Set the degree of jittering
*/
static void
degree_cb (GtkAdjustment * adj, GGobiSession * gg)
{
  GtkWidget *tree_view =
    get_tree_view_from_object (G_OBJECT (gg->jitter_ui.window));
  GGobiStage *d =
    (GGobiStage *) g_object_get_data (G_OBJECT (tree_view), "datad");
  gint *vars;                   // = (gint *) g_malloc (d->n_cols * sizeof(gint));
  gint nvars;

  vars = get_selections_from_tree_view (tree_view, &nvars);

  jitter_value_set (adj->value, d, gg);
  if (nvars) {
    rejitter (vars, nvars, d, gg);
    g_free (vars);
  }
}

/*-- called when closed from the close button --*/
static void
close_btn_cb (GtkWidget * w, GGobiSession * gg)
{
  gtk_widget_hide (gg->jitter_ui.window);
}

/*-- called when closed from the window manager --*/
static void
close_wmgr_cb (GtkWidget * w, GdkEvent * event, GGobiSession * gg)
{
  gtk_widget_hide (gg->jitter_ui.window);
}

static gchar *type_lbl[] = { "Uniform", "Normal" };

static void
type_cb (GtkWidget * w, GGobiSession * gg)
{
  gint indx = gtk_combo_box_get_active (GTK_COMBO_BOX (w));
  GtkWidget *tree_view =
    get_tree_view_from_object (G_OBJECT (gg->jitter_ui.window));
  GGobiStage *d =
    (GGobiStage *) g_object_get_data (G_OBJECT (tree_view), "datad");
  gint *vars;                   // = (gint *) g_malloc (d->n_cols * sizeof(gint));
  gint nvars;

  vars = get_selections_from_tree_view (tree_view, &nvars);

  d->jitter.type = indx;

  if (nvars) {
    rejitter (vars, nvars, d, gg);
    g_free (vars);
  }
}

void
jitter_window_open (GGobiSession * gg)
{

  GtkWidget *btn, *lbl;
  GtkWidget *vbox, *vb, *hb;
  GtkWidget *sbar, *opt;
  GtkObject *adj;
  GtkWidget *notebook;

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length (gg->d) == 0)
    return;

  else {

    if (gg->jitter_ui.window == NULL) {

      gg->jitter_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

      /*-- suggested by Gordon Deane --*/
      gtk_window_set_default_size (GTK_WINDOW (gg->jitter_ui.window), 200,
                                   400);
      /*-- 400 looks too big on the laptop, trying other numbers   --*/
      gtk_window_set_default_size (GTK_WINDOW (gg->jitter_ui.window), 200,
                                   250);
      /*--                           --*/

      g_signal_connect (G_OBJECT (gg->jitter_ui.window), "delete_event",
                        G_CALLBACK (close_wmgr_cb), (gpointer) gg);
      gtk_window_set_title (GTK_WINDOW (gg->jitter_ui.window), "Jitter Data");

      gtk_container_set_border_width (GTK_CONTAINER (gg->jitter_ui.window),
                                      10);

      vbox = gtk_vbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (gg->jitter_ui.window), vbox);

      /* Create a notebook, set the position of the tabs */
      notebook = create_variable_notebook (vbox,
                                           GTK_SELECTION_MULTIPLE,
                                           GGOBI_VARIABLE_ALL_VARTYPES, all_datatypes,
                                           G_CALLBACK (NULL), NULL, gg);

      /*-- option menu --*/
      opt = gtk_combo_box_new_text ();
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
                            "The jittering is either distributed uniform or normal",
                            NULL);
      gtk_box_pack_start (GTK_BOX (vbox), opt, false, false, 0);
      populate_combo_box (opt, type_lbl, G_N_ELEMENTS (type_lbl),
                          G_CALLBACK (type_cb), gg);

      /*-- vbox for label and rangewidget --*/
      vb = gtk_vbox_new (true, 2);
      gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 1);

      lbl = gtk_label_new_with_mnemonic ("_Degree of jitter:");
      gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

      adj = gtk_adjustment_new (0.0, 0.0, 0.7, 0.01, .01, 0.0);
      g_signal_connect (G_OBJECT (adj), "value_changed",
                        G_CALLBACK (degree_cb), (gpointer) gg);

      sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
                            "Set the degree of jitter", NULL);
      gtk_scale_set_draw_value (GTK_SCALE (sbar), false);
      gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
      gtk_scale_set_digits (GTK_SCALE (sbar), 2);
      gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
      gtk_box_pack_start (GTK_BOX (vb), sbar, false, false, 0);

      gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), sbar);

      /*-- Rejitter button --*/
      btn = gtk_button_new_with_mnemonic ("_Jitter");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                            "Rejitter the data", NULL);
      g_signal_connect (G_OBJECT (btn), "clicked",
                        G_CALLBACK (jitter_cb), (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);

      /*-- Close button --*/
      gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new (), false, true,
                          2);
      hb = gtk_hbox_new (false, 2);
      gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 1);

      btn = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
      g_signal_connect (G_OBJECT (btn), "clicked",
                        G_CALLBACK (close_btn_cb), gg);
      gtk_box_pack_start (GTK_BOX (hb), btn, true, false, 0);

      g_object_set_data (G_OBJECT (gg->jitter_ui.window), "notebook", notebook);
      gtk_widget_show_all (gg->jitter_ui.window);
    }
  }

  gdk_window_raise (gg->jitter_ui.window->window);
}
