                                    /* impute_ui.c *//*-- should be called missing_ui.c --*/
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

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*-- called when closed from the close button --*/
static void
close_btn_cb (GtkWidget * w, ggobid * gg)
{
  gtk_widget_hide (gg->impute.window);
}

/*-- called when closed from the window manager --*/
static void
close_wmgr_cb (GtkWidget * w, GdkEvent * event, ggobid * gg)
{
  gtk_widget_hide (gg->impute.window);
}

/* Random */
static void
set_random_cb (GtkToggleButton * w, ggobid * gg)
{
  gg->impute.type = IMP_RANDOM;
}

/* Fixed */
static void
set_fixed_cb (GtkToggleButton * w, ggobid * gg)
{
  gg->impute.type = IMP_FIXED;
}

/* Below */
static void
set_fixed_below_cb (GtkToggleButton * w, ggobid * gg)
{
  gg->impute.type = IMP_BELOW;
}

/* Above */
static void
set_fixed_above_cb (GtkToggleButton * w, ggobid * gg)
{
  gg->impute.type = IMP_ABOVE;
}

/* Mean */
static void
set_mean_cb (GtkToggleButton * w, ggobid * gg)
{
  gg->impute.type = IMP_MEAN;
}

/* Median */
static void
set_median_cb (GtkToggleButton * w, ggobid * gg)
{
  gg->impute.type = IMP_MEDIAN;
}

static void
rescale_cb (GtkButton * button, ggobid * gg)
{
  GtkWidget *tv = get_tree_view_from_object (G_OBJECT (gg->impute.window));
  GGobiData *d = (GGobiData *) g_object_get_data (G_OBJECT (tv), "datad");

  limits_set (d, true, true, gg->lims_use_visible);
  vartable_limits_set (d);
  vartable_stats_set (d);

  tform_to_world (d, gg);
  displays_tailpipe (FULL, gg);
}
static void
group_cb (GtkToggleButton * button, ggobid * gg)
{
  gg->impute.bgroup_p = button->active;
}
static void
show_missings_cb (GtkToggleButton * button, ggobid * gg)
{
  GtkWidget *tv = get_tree_view_from_object (G_OBJECT (gg->impute.window));
  GGobiData *d = (GGobiData *) g_object_get_data (G_OBJECT (tv), "datad");

  d->missings_show_p = button->active;
  displays_tailpipe (FULL, gg);
}

static gboolean
impute_fixed_cb (ImputeType impute_type, gfloat * val, ggobid * gg)
{
  GtkWidget *w;
  gchar *val_str;
  gboolean ok = true;

  if (impute_type == IMP_ABOVE || impute_type == IMP_BELOW) {

    if (impute_type == IMP_ABOVE) {
      w = widget_find_by_name (gg->impute.window, "IMPUTE:entry_above");
      val_str = gtk_editable_get_chars (GTK_EDITABLE (w), 0, -1);
    }
    else { // if (impute_type == IMP_BELOW) {
      w = widget_find_by_name (gg->impute.window, "IMPUTE:entry_below");
      val_str = gtk_editable_get_chars (GTK_EDITABLE (w), 0, -1);
    }

    if (strlen (val_str) == 0) {
      gchar *message =
        g_strdup_printf
        ("You selected '%% over or under' but didn't specify a percentage.\n");
      quick_message (message, false);
      g_free (message);
      ok = false;
      return ok;
    }

    *val = (gfloat) atof (val_str);
    g_free (val_str);
    if (*val < 0 || *val > 100) {
      gchar *message =
        g_strdup_printf
        ("You specified %f%%; please specify a percentage between 0 and 100.\n",
         *val);
      quick_message (message, false);
      g_free (message);
      ok = false;
      return ok;
    }
  }
  else if (impute_type == IMP_FIXED) {
    w = widget_find_by_name (gg->impute.window, "IMPUTE:entry_val");
    val_str = gtk_editable_get_chars (GTK_EDITABLE (w), 0, -1);
    if (strlen (val_str) == 0) {
      quick_message
        ("You've selected 'Specify' but haven't specified a value.\n", false);
      ok = false;
      return ok;
    }
    else {
      *val = (gfloat) atof (val_str);
      g_free (val_str);
    }
  }

  return ok;
}

static void
impute_cb (GtkWidget * w, ggobid * gg)
{
  gboolean redraw = true;
  GtkWidget *tree_view =
    get_tree_view_from_object (G_OBJECT (gg->impute.window));
  GGobiData *d =
    (GGobiData *) g_object_get_data (G_OBJECT (tree_view), "datad");
  gint *vars; // = (gint *) g_malloc (d->ncols * sizeof(gint));
  gint nvars;
  gfloat val = 0.0;  // compiler pacification

  vars = get_selections_from_tree_view (tree_view, &nvars);

  switch (gg->impute.type) {
  case IMP_RANDOM:
    impute_random (d, nvars, vars, gg);
    break;
  case IMP_FIXED:
  case IMP_BELOW:
  case IMP_ABOVE:
    if (impute_fixed_cb (gg->impute.type, &val, gg))
      redraw = impute_fixed (gg->impute.type, val, nvars, vars, d, gg);
    break;
  case IMP_MEAN:
  case IMP_MEDIAN:
    redraw = impute_mean_or_median (gg->impute.type, nvars, vars, d, gg);
    break;
  }

  if (redraw) {
    tform_to_world (d, gg);
    displays_tailpipe (FULL, gg);
  }

  g_free (vars);
}

/*------------------------------------------------------------------*/

void
impute_window_open (ggobid * gg)
{
  GtkWidget *frame0, *vb;
  GtkWidget *btn, *tgl, *notebook;
  GtkWidget *vbox, *hb;
  GtkWidget *table, *entry, *radio;
  gint row = 0;

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length (gg->d) == 0)
    /**/ return;

  if (gg->impute.window == NULL) {

    gg->impute.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->impute.window), "Missing Values");
    g_signal_connect (G_OBJECT (gg->impute.window),
                      "delete_event", G_CALLBACK (close_wmgr_cb), gg);

    gtk_container_set_border_width (GTK_CONTAINER (gg->impute.window), 5);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (gg->impute.window), vbox);

    /*-- Add a toggle button, show missings or not --*/
    tgl = gtk_check_button_new_with_mnemonic ("Sh_ow missing values");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tgl), on);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
                          "Draw the missing values when plotting displays; if there are multiple datasets, this applies only to the current dataset",
                          NULL);
    g_signal_connect (G_OBJECT (tgl), "toggled",
                      G_CALLBACK (show_missings_cb), (gpointer) gg);
    gtk_box_pack_start (GTK_BOX (vbox), tgl, false, false, 2);


    /*-- add a button to generate a new datad --*/
    btn = gtk_button_new_with_mnemonic ("_Add missings as new dataset");
    g_signal_connect (G_OBJECT (btn),
                      "clicked", G_CALLBACK (missings_datad_cb),
                      (gpointer) gg);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                          "Generate a new dataset from the 1's and 0's representing missingness",
                          NULL);
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 2);

    /*-- add a frame to contain the "imputation" widgets --*/
    frame0 = gtk_frame_new ("Assign or impute values");
    gtk_container_set_border_width (GTK_CONTAINER (frame0), 2);
    gtk_box_pack_start (GTK_BOX (vbox), frame0, true, true, 2);

    vb = gtk_vbox_new (false, 2);
    /*-- this has the effect of setting an internal border inside the frame --*/
    gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
    gtk_container_add (GTK_CONTAINER (frame0), vb);

    /* Create a notebook, set the position of the tabs */
    notebook = create_variable_notebook (vb,
                                         GTK_SELECTION_MULTIPLE, all_vartypes,
                                         all_datatypes, G_CALLBACK (NULL),
                                         NULL, gg);
    row = 0;
    table = gtk_table_new (6, 2, false);
    gtk_box_pack_start (GTK_BOX (vb), table, false, false, 2);

    /* Random */
    radio = gtk_radio_button_new_with_mnemonic (NULL, "_Random");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio,
                          "Assign to each missing value one of the existing variable values chosen at random",
                          NULL);
    g_signal_connect (G_OBJECT (radio), "toggled",
                      G_CALLBACK (set_random_cb), (gpointer) gg);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), true);
    gtk_table_attach (GTK_TABLE (table), radio, 0, 1, row, row + 1,
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      1, 1);
    tgl =
      gtk_check_button_new_with_mnemonic ("Condition on symbol and _color");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
                          "Condition the random imputation on the symbol and color; these groups can be seen in the case clusters window",
                          NULL);
    g_signal_connect (G_OBJECT (tgl), "toggled", G_CALLBACK (group_cb),
                      (gpointer) gg);
    gtk_table_attach (GTK_TABLE (table), tgl, 1, 2, row, row + 1,
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      1, 1);
    row++;



    /* Mean */
    radio =
      gtk_radio_button_new_with_mnemonic (gtk_radio_button_get_group
                                          (GTK_RADIO_BUTTON (radio)),
                                          "Variable _mean");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio,
                          "Assign the variable mean to each missing value",
                          NULL);
    g_signal_connect (G_OBJECT (radio), "toggled", G_CALLBACK (set_mean_cb),
                      (gpointer) gg);
    gtk_table_attach (GTK_TABLE (table), radio, 0, 1, row, row + 1,
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      1, 1);
    row++;

    /* Median */
    radio =
      gtk_radio_button_new_with_mnemonic (gtk_radio_button_get_group
                                          (GTK_RADIO_BUTTON (radio)),
                                          "Variable m_edian");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio,
                          "Assign the variable median to each missing value",
                          NULL);
    g_signal_connect (G_OBJECT (radio), "toggled", G_CALLBACK (set_median_cb),
                      (gpointer) gg);
    gtk_table_attach (GTK_TABLE (table), radio, 0, 1, row, row + 1,
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      1, 1);
    row++;


    /* Fixed */
    radio =
      gtk_radio_button_new_with_mnemonic (gtk_radio_button_get_group
                                          (GTK_RADIO_BUTTON (radio)),
                                          "_Fixed");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio,
                          "Assign a fixed value to each missing variable value",
                          NULL);
    g_signal_connect (G_OBJECT (radio), "toggled", G_CALLBACK (set_fixed_cb),
                      (gpointer) gg);
    gtk_table_attach (GTK_TABLE (table), radio, 0, 1, row, row + 1,
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      1, 1);
    entry = gtk_entry_new ();
    gtk_widget_set_name (entry, "IMPUTE:entry_val");
    gtk_entry_set_text (GTK_ENTRY (entry), "0");
    gtk_table_attach (GTK_TABLE (table), entry, 1, 2, row, row + 1,
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      1, 1);
    row++;

    /* Pctage below min */
    radio =
      gtk_radio_button_new_with_mnemonic (gtk_radio_button_get_group
                                          (GTK_RADIO_BUTTON (radio)),
                                          "Percent _below min");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio,
                          "Assign a fixed value which is some percentage below the minimum value for the variable",
                          NULL);
    g_signal_connect (G_OBJECT (radio), "toggled",
                      G_CALLBACK (set_fixed_below_cb), (gpointer) gg);
    gtk_table_attach (GTK_TABLE (table), radio, 0, 1, row, row + 1,
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      1, 1);
    entry = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (entry), "10");
    gtk_widget_set_name (entry, "IMPUTE:entry_below");
    gtk_table_attach (GTK_TABLE (table), entry, 1, 2, row, row + 1,
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      1, 1);
    row++;

    /* Pctage above min */
    radio =
      gtk_radio_button_new_with_mnemonic (gtk_radio_button_get_group
                                          (GTK_RADIO_BUTTON (radio)),
                                          "Percent ab_ove min");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio,
                          "Assign a fixed value which is some percentage above the minimum value for the variable",
                          NULL);
    g_signal_connect (G_OBJECT (radio), "toggled",
                      G_CALLBACK (set_fixed_above_cb), (gpointer) gg);
    gtk_table_attach (GTK_TABLE (table), radio, 0, 1, row, row + 1,
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      1, 1);
    entry = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (entry), "10");
    gtk_widget_set_name (entry, "IMPUTE:entry_above");
    gtk_table_attach (GTK_TABLE (table), entry, 1, 2, row, row + 1,
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL | GTK_EXPAND),
                      1, 1);
    row++;

   /*-- hbox to hold a few buttons --*/
    hb = gtk_hbox_new (true, 2);

    gtk_box_pack_start (GTK_BOX (vb), hb, false, false, 2);

    btn = gtk_button_new_with_mnemonic ("_Impute");
    g_signal_connect (G_OBJECT (btn),
                      "clicked", G_CALLBACK (impute_cb), (gpointer) gg);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                          "Impute or assign values to missings", NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, true, 2);

    btn = gtk_button_new_with_mnemonic ("Re_scale");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                          "Rescale the data after imputing", NULL);
    g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (rescale_cb), (gpointer) gg);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, true, 2);



    /*-- add a close button --*/
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 1);

    btn = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                          "Close the window", NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, false, 2);
    g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (close_btn_cb), gg);

    g_object_set_data (G_OBJECT (gg->impute.window), "notebook", notebook);
  }

  gtk_widget_show_all (gg->impute.window);
  gdk_window_raise (gg->impute.window->window);
}
