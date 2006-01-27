/* transform_ui.c */
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

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*-- called when closed from the close button --*/
static void close_btn_cb (GtkWidget *w, ggobid *gg) {
  gtk_widget_hide (gg->tform_ui.window);
}
/*-- called when closed from the window manager --*/
static void close_wmgr_cb (GtkWidget *w, GdkEvent *event, ggobid *gg) {
  gtk_widget_hide (gg->tform_ui.window);
}

static gchar *stage0_lbl[] = {"No transformation",
                              "Raise minimum to 0",
                              "Raise minimum to 1",
                              "Negative"};
static void stage0_cb (GtkWidget *w, ggobid *gg)
{
  gint indx = gtk_combo_box_get_active (GTK_COMBO_BOX(w));
  GtkWidget *tree_view = get_tree_view_from_object (G_OBJECT(gg->tform_ui.window));
  datad *d = (datad *) g_object_get_data(G_OBJECT (tree_view), "datad");
  gint *vars;// = (gint *) g_malloc (d->ncols * sizeof(gint));
  gint nvars;
  
  vars = get_selections_from_tree_view (tree_view, &nvars);

  if (nvars) {
    transform (0, indx, -99., vars, nvars, d, gg);
    g_free (vars);
  }
}

static gchar *stage1_lbl[] = {"No transformation",
                              "Box-Cox",
                              "Log base 10",
                              "Inverse",
                              "Absolute value",
                              "Scale to [a,b]",
                              };
static void
stage1_cb (GtkWidget *w, ggobid *gg)
{
  gint indx = gtk_combo_box_get_active (GTK_COMBO_BOX(w));
  GtkWidget *tree_view = get_tree_view_from_object (G_OBJECT(gg->tform_ui.window));
  datad *d = (datad *) g_object_get_data(G_OBJECT (tree_view), "datad");
  gint *vars;// = (gint *) g_malloc (d->ncols * sizeof(gint));
  gint nvars;
  
  vars = get_selections_from_tree_view (tree_view, &nvars);

  if (nvars) {
    transform (1, indx, gg->tform_ui.boxcox_adj->value, vars, nvars, d, gg);
    g_free (vars);
  }
}

/*
 * Set the spin widget's adjustment->step_increment to adj->value
*/
void boxcox_cb (GtkAdjustment *adj, ggobid *gg)
{
  GtkWidget *tree_view = get_tree_view_from_object (G_OBJECT(gg->tform_ui.window));
  datad *d = (datad *) g_object_get_data(G_OBJECT (tree_view), "datad");
  gint *vars; // = (gint *) g_malloc (d->ncols * sizeof(gint));
  gint nvars;
  
  vars = get_selections_from_tree_view (tree_view, &nvars);
  
  if (nvars) {
    transform (1, BOXCOX, adj->value, vars, nvars, d, gg);
    g_free (vars);
  }
}

gfloat
scale_get_a (ggobid *gg) {
  gchar *val_str;
  gfloat val = 0;  /*-- default value --*/
  GtkWidget *entry_a;
  entry_a = widget_find_by_name (gg->tform_ui.window, "TRANSFORM:entry_a");

  if (entry_a) {
    val_str = gtk_editable_get_chars (GTK_EDITABLE (entry_a), 0, -1);
    if (val_str != NULL && strlen (val_str) > 0) {
      val = (gfloat) atof (val_str);
      g_free (val_str);
    }
  } else {
    g_printerr ("Failed to locate the entry widget\n");
  }

  return val;
}
gfloat
scale_get_b (ggobid *gg) {
  gchar *val_str;
  gfloat val = 1;  /*-- default value --*/
  GtkWidget *entry_b;
  entry_b = widget_find_by_name (gg->tform_ui.window, "TRANSFORM:entry_b");

  if (entry_b) {
    val_str = gtk_editable_get_chars (GTK_EDITABLE (entry_b), 0, -1);
    if (val_str != NULL && strlen (val_str) > 0) {
      val = (gfloat) atof (val_str);
      g_free (val_str);
    }
  } else {
    g_printerr ("Failed to locate the entry widget\n");
  }

  return val;
}

static gchar *stage2_lbl[] = {"No transformation",
                              "Standardize",
                              "Sort",
                              "Rank",
                              "Normal score",
                              "Z-score",
                              "Discretize: 2 levels"
                              };
static void stage2_cb (GtkWidget *w, ggobid *gg)
{
  GtkWidget *tree_view = get_tree_view_from_object (G_OBJECT(gg->tform_ui.window));
  datad *d = (datad *) g_object_get_data(G_OBJECT (tree_view), "datad");
  gint *vars;// = (gint *) g_malloc (d->ncols * sizeof(gint));
  gint nvars;
  gint indx = gtk_combo_box_get_active (GTK_COMBO_BOX(w));

  vars = get_selections_from_tree_view (tree_view, &nvars);
  
  if (nvars) {
    transform (2, indx, -99, vars, nvars, d, gg);
    g_free (vars);
  }
}

static void tform_reset_cb (GtkWidget *w, ggobid *gg)
{
  gint j;
  GtkWidget *tree_view = get_tree_view_from_object (G_OBJECT(gg->tform_ui.window));
  datad *d = (datad *) g_object_get_data(G_OBJECT (tree_view), "datad");

  for (j=0; j<d->ncols; j++) {
    transform0_values_set (NO_TFORM0, j, d, gg);
    transform1_values_set (NO_TFORM1, 1.0, j, d, gg);
    transform2_values_set (NO_TFORM2, j, d, gg);

    transform1_apply (j, d, gg);
    transform2_apply (j, d, gg);

    tform_label_update (j, d, gg);
  }

  limits_set (true, true, d, gg);  
  vartable_limits_set (d);
  vartable_stats_set (d);

  tform_to_world (d, gg);
  displays_tailpipe (FULL, gg);
}

void
transform_window_open (ggobid *gg) 
{
  GtkWidget *vbox, *frame, *notebook, *hb, *vb, *btn;
  GtkWidget *stage0_option_menu, *stage1_option_menu, *stage2_option_menu;
  GtkWidget *lbl, *entry_a, *entry_b;
  GtkWidget *spinner;

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length (gg->d) == 0) 
    return;

  if (gg->tform_ui.window == NULL) {
    gg->tform_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->tform_ui.window),
                          "Transform Variables");
    gtk_container_set_border_width (GTK_CONTAINER (gg->tform_ui.window), 10);

    g_signal_connect (G_OBJECT (gg->tform_ui.window),
                        "delete_event",
                        G_CALLBACK (close_wmgr_cb),
                        (gpointer) gg);

/*
 * Transformations
*/
    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (gg->tform_ui.window), vbox);

    /* Create a notebook, set the position of the tabs */
    notebook = create_variable_notebook (vbox,
      GTK_SELECTION_MULTIPLE, all_vartypes, all_datatypes,
      G_CALLBACK(NULL), NULL, gg);

    /*
     * Stage 0: Domain adjustment
    */
    frame = gtk_frame_new ("Stage 0");
    //gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame, false, false, 1);

    stage0_option_menu = gtk_combo_box_new_text ();
    gtk_widget_set_name (stage0_option_menu, "TRANSFORM:stage0_option_menu");
    //gtk_container_set_border_width (GTK_CONTAINER (stage0_option_menu), 4);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), stage0_option_menu,
      "Stage 0: Adjust the domain of the variables",
      NULL);
    populate_combo_box (stage0_option_menu, stage0_lbl, G_N_ELEMENTS(stage0_lbl),
      G_CALLBACK(stage0_cb), gg);
    gtk_container_add (GTK_CONTAINER (frame), stage0_option_menu);

    /*
     * Stage 1: Power transformations et al
    */
    frame = gtk_frame_new ("Stage 1");
    //gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame, false, false, 1);

    vb = gtk_vbox_new (false, 5);
    gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
    gtk_container_add (GTK_CONTAINER (frame), vb);

    stage1_option_menu = gtk_combo_box_new_text ();
    gtk_widget_set_name (stage1_option_menu, "TRANSFORM:stage1_option_menu");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), stage1_option_menu,
      "Stage 1: Data-independent transformations, preserving user-defined limits",
      NULL);
    populate_combo_box (stage1_option_menu, stage1_lbl, G_N_ELEMENTS(stage1_lbl),
    	G_CALLBACK(stage1_cb), gg);
    gtk_box_pack_start (GTK_BOX (vb), stage1_option_menu, true, false, 1);

    /*-- label and spin button for Box-Cox parameter --*/
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vb), hb, false, false, 2);
    
    lbl = gtk_label_new_with_mnemonic ("Box-Cox _param:");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);
    gg->tform_ui.boxcox_adj = (GtkAdjustment *) gtk_adjustment_new (1.0,
                          -4, 5, 0.05, .5, 0.0);
    spinner = gtk_spin_button_new (gg->tform_ui.boxcox_adj, 0, 3);
	gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), spinner);

    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), false);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinner,
      "Set the Box-Cox power function parameter", NULL);
    gtk_box_pack_end (GTK_BOX (hb), spinner, true, true, 0);
    g_signal_connect (G_OBJECT (gg->tform_ui.boxcox_adj), "value_changed",
                        G_CALLBACK (boxcox_cb),
                        (gpointer) gg);

    /*-- labels and entries for scaling limits --*/
    /*style = gtk_widget_get_style (spinner);
    gdk_text_extents (
      gtk_style_get_font (style),
      "999999999", strlen ("999999999"),
      &lbearing, &rbearing, &width, &ascent, &descent);*/

    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vb), hb, false, false, 2);

    lbl = gtk_label_new_with_mnemonic ("_a:");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);

    entry_a = gtk_entry_new ();
	gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), entry_a);
    gtk_widget_set_name (entry_a, "TRANSFORM:entry_a");
    gtk_entry_set_text (GTK_ENTRY (entry_a), "0");
	gtk_entry_set_width_chars(GTK_ENTRY(entry_a), 9);
    //gtk_widget_set_usize (entry_a, width, -1);
    gtk_box_pack_start (GTK_BOX (hb), entry_a, false, false, 0);

    lbl = gtk_label_new_with_mnemonic ("_b:");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);

    entry_b = gtk_entry_new ();
	gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), entry_b);
    gtk_widget_set_name (entry_b, "TRANSFORM:entry_b");
    gtk_entry_set_text (GTK_ENTRY (entry_b), "1");
	gtk_entry_set_width_chars(GTK_ENTRY(entry_b), 9);
    //gtk_widget_set_usize (entry_b, width, -1);
    gtk_box_pack_start (GTK_BOX (hb), entry_b, false, false, 0);

    /*
     * Stage 2: Another standardization step
    */
    frame = gtk_frame_new ("Stage 2");
    //gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame, false, false, 1);

    stage2_option_menu = gtk_combo_box_new_text ();
    gtk_widget_set_name (stage2_option_menu, "TRANSFORM:stage2_option_menu");
    //gtk_container_set_border_width (GTK_CONTAINER (stage2_option_menu), 4);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), stage2_option_menu,
      "Stage 2: Data-dependent transformations, ignoring user-defined limits",
      NULL);
    populate_combo_box (stage2_option_menu, stage2_lbl, G_N_ELEMENTS(stage2_lbl),
      G_CALLBACK(stage2_cb), gg);
    gtk_container_add (GTK_CONTAINER (frame), stage2_option_menu);

    /*
     * A button or two
    */

    btn = gtk_button_new_with_mnemonic ("_Reset all");
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Set all transformation stages to 'no transformation' for the selected variables",
      NULL);
    g_signal_connect (G_OBJECT (btn), "clicked",
                        G_CALLBACK (tform_reset_cb), gg);

    /*-- add a close button --*/
    gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new(), false, true, 2);
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 1);

    btn = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Close the window", NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, false, 1);
    g_signal_connect (G_OBJECT (btn), "clicked",
                        G_CALLBACK (close_btn_cb), gg);

    g_object_set_data(G_OBJECT (gg->tform_ui.window),
      "notebook", notebook);
  } 

  gtk_widget_show_all (gg->tform_ui.window);
  gdk_window_raise (gg->tform_ui.window->window);
}

/*
 * These routines are used to set the values of the option menus.
 * They're used when the transformations are set from somewhere
 * other than those option menus, such as the reset button.
*/
void
transform0_opt_menu_set_value (gint j, datad *d, ggobid *gg)
{
  GtkWidget *stage0_option_menu;
  vartabled *vt = vartable_element_get (j, d);

  stage0_option_menu = widget_find_by_name (gg->tform_ui.window,
                                            "TRANSFORM:stage0_option_menu");
  gtk_combo_box_set_active (GTK_COMBO_BOX (stage0_option_menu),
    vt->tform0);
}
void
transform1_opt_menu_set_value (gint j, datad *d, ggobid *gg)
{
  GtkWidget *stage1_option_menu;
  vartabled *vt = vartable_element_get (j, d);

  stage1_option_menu = widget_find_by_name (gg->tform_ui.window,
                                            "TRANSFORM:stage1_option_menu");
  gtk_combo_box_set_active (GTK_COMBO_BOX (stage1_option_menu),
    vt->tform1);
}
void
transform2_opt_menu_set_value (gint j, datad *d, ggobid *gg)
{
  GtkWidget *stage2_option_menu;
  vartabled *vt = vartable_element_get (j, d);

  stage2_option_menu = widget_find_by_name (gg->tform_ui.window,
                                            "TRANSFORM:stage2_option_menu");
  gtk_combo_box_set_active (GTK_COMBO_BOX (stage2_option_menu),
    vt->tform2);
}
