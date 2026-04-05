/* tour1d_pp_ui.c */
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

#define WIDTH   200
#define HEIGHT  100

/*-- projection pursuit indices --*/
#define PCA            0
#define LDA            1
#define CGINI          2
#define CENTROPY       3
#define CART_VAR       4
#define SUBD           5

/* terms in expansion, bandwidth */
/*
static GtkWidget *param_vb, *param_lbl, *param_scale;
static GtkAdjustment *param_adj;
*/

/*-- called when closed from the close menu item --*/
static void action_close_cb (GtkWidget *w, displayd *dsp) {
  gtk_widget_hide (dsp->t1d_window);
  t1d_optimz(0, &dsp->t1d.get_new_target, 
    &dsp->t1d.target_selection_method, dsp);

  /*  free_optimize0_p(&dsp->t1d_pp_op); * should this go here? *
  free_pp(&dsp->t1d_pp_param); seems not, causes a crash because window
                               just gets hidden, so shouldn't
                               free the arrays. */
}
/*-- called when destroyed from the window manager --*/
static void
close_wmgr_cb (GtkWidget *w, GdkEventButton *event, displayd *dsp) {
  gtk_widget_hide (dsp->t1d_window);
  t1d_optimz(0, &dsp->t1d.get_new_target, 
    &dsp->t1d.target_selection_method, dsp);

  free_optimize0_p(&dsp->t1d_pp_op);
  free_pp(&dsp->t1d_pp_param);
  if (dsp->t1d_pp_surface != NULL) {
    cairo_surface_destroy (dsp->t1d_pp_surface);
    dsp->t1d_pp_surface = NULL;
  }
  gtk_widget_destroy (dsp->t1d_window);
  dsp->t1d_window = NULL;
}

static void
action_show_controls_cb(GtkCheckMenuItem *item, displayd *dsp) {
      if (gtk_check_menu_item_get_active(item))
        gtk_widget_show (dsp->t1d_control_frame);
      else
        gtk_widget_hide (dsp->t1d_control_frame);
}

/*static void
line_options_cb(gpointer data, guint action, GtkCheckMenuItem *w) {
  g_printerr ("action = %d\n", action);

  switch (action) {
    case 0:
    case 1:
    case 2:
    default:
      fprintf(stderr, "Unhandled switch-case in line_options_cb\n");
  }
  }*/

/*static void
bitmap_size_cb(gpointer data, guint action, GtkCheckMenuItem *w) {
  g_printerr ("action = %d\n", action);

  switch (action) {
    case 0:
    case 1:
    case 2:
    default:
      fprintf(stderr, "Unhandled switch-case in bitmap_size_cb\n");
  }
  }*/

/*static void
replot_freq_cb(gpointer data, guint action, GtkCheckMenuItem *w) {
  g_printerr ("action = %d\n", action);

  switch (action) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
      break;
    default:
      fprintf(stderr, "Unhandled switch-case in replot_freq_cb\n");
  }
  }*/

static void
t1d_optimz_cb (GtkToggleButton  *w, displayd *dsp) {
  if (dsp == NULL) {
    g_printerr ("No display corresponds to these controls\n");
    return;
  }

  t1d_optimz(gtk_toggle_button_get_active (w), &dsp->t1d.get_new_target, 
    &dsp->t1d.target_selection_method, dsp);
}

static void t1d_pptemp_set_cb (GtkAdjustment *adj, displayd *dsp) {

  t1d_pptemp_set(gtk_adjustment_get_value (adj), dsp, dsp->d->gg);
}

static void t1d_ppcool_set_cb (GtkAdjustment *adj, displayd *dsp) {

  t1d_ppcool_set(gtk_adjustment_get_value (adj), dsp, dsp->d->gg);
}

gchar *t1d_pp_func_lbl[] = {"Holes","Central Mass","PCA","LDA","Gini-C","Entropy-C"};
/*,"LDA","CART Gini","CART Entropy", 
                            "CART Variance","SUB-D"
                            };*/
void t1d_pp_func_cb (GtkWidget *w, displayd *dsp)
{
  cpaneld *cpanel = NULL;
  gint indx = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
  gchar *label = g_strdup("PP index: (0.000) 0.0000 (0.000)");
  ggobid *gg;

  if (dsp == NULL) {
    g_printerr ("No display corresponds to these controls\n");
    return;
  }
  gg = GGobiFromDisplay (dsp);

  cpanel = &dsp->cpanel;
  cpanel->t1d.pp_indx = indx;
  dsp->t1d.get_new_target = true;

  dsp->t1d.ppval = 0.0;
  dsp->t1d.oppval = -1.0;
  dsp->t1d_pp_op.index_best = 0.0;
  sprintf(label, "PP index: (%3.1f) %5.3f (%3.1f) ",0.0,dsp->t1d.ppval,0.0);
  gtk_label_set_text(GTK_LABEL(dsp->t1d_pplabel),label);

  t1d_clear_ppda(dsp, gg);

  /*  if (indx == SUBD || LDA || CART_GINI || CART_ENTROPY || CART_VAR || PCA)
    gtk_widget_hide (param_vb);
  else {
  gtk_widget_show (param_vb);
  }*/
}

/*
static void bitmap_cb (GtkButton *button)
{
  g_printerr ("drop a new bitmp\n");
}
static void
return_to_bitmap_cb (GtkToggleButton  *w) {
  g_printerr ("return to bitmap?  %d\n", w->active);
}
static void
record_bitmap_cb (GtkToggleButton  *w) {
  g_printerr ("record bitmap?  %d\n", w->active);
}
*/

static gint
ppda_configure_cb (GtkWidget *w, GdkEventConfigure *event, displayd *dsp)
{
  gint wid = gtk_widget_get_allocated_width (w);
  gint hgt = gtk_widget_get_allocated_height (w);

  if (dsp->t1d_pp_surface != NULL)
    cairo_surface_destroy (dsp->t1d_pp_surface);

  dsp->t1d_pp_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
    wid, hgt);

  return false;
}

static gboolean
ppda_present (GtkWidget *w, cairo_t *cr, displayd *dsp)
{
  if (dsp->t1d_pp_surface == NULL)
    return FALSE;

  cairo_set_source_surface (cr, dsp->t1d_pp_surface, 0, 0);
  cairo_paint (cr);

  return FALSE;
}

static gboolean
ppda_draw_cb (GtkWidget *w, cairo_t *cr, displayd *dsp)
{
  return ppda_present (w, cr, dsp);
}

void
tour1dpp_window_open (ggobid *gg) {
  GtkWidget *hbox, *vbox, *vbc, *vb, *frame, *tgl, *hb, *opt, *sbar, *lbl;
  GtkWidget *file_menu, *options_menu;
  GtkAdjustment *adj;
  GtkAccelGroup *accel_group = NULL;
  /*GtkWidget *da, *label, *entry;*/
  displayd *dsp = gg->current_display;  /* ok as long as we only use the gui */
  GGobiData *d = dsp->d;
  /*-- to initialize the checkboxes in the menu --*/

  if (dsp->t1d_window == NULL) {
    dsp->t1d_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (dsp->t1d_window), 
      "Projection Pursuit - 1D");
    g_signal_connect (G_OBJECT (dsp->t1d_window), "delete_event",
                        G_CALLBACK (close_wmgr_cb), (gpointer) dsp);
    /*gtk_window_set_policy (GTK_WINDOW (dsp->t1d_window), true, true, false);*/
    g_signal_connect (G_OBJECT(d), "rows_in_plot_changed",
      G_CALLBACK(reset_pp), gg);

    gtk_container_set_border_width (GTK_CONTAINER (dsp->t1d_window), 10);

/*
 * Add the main menu bar
*/
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 1);
    gtk_container_add (GTK_CONTAINER (dsp->t1d_window), vbox);
    accel_group = ggobi_window_add_accel_group (dsp->t1d_window);
    dsp->t1d_mbar = gtk_menu_bar_new ();
    file_menu = ggobi_menu_add_submenu (dsp->t1d_mbar, "_File");
    ggobi_menu_append_item (file_menu, "_Close", "<control>C",
                            accel_group, G_CALLBACK (action_close_cb), dsp);
    options_menu = ggobi_menu_add_submenu (dsp->t1d_mbar, "_Options");
    ggobi_menu_append_check_item (options_menu, "_Show controls",
                                  "<control>S", accel_group, true,
                                  G_CALLBACK (action_show_controls_cb), dsp);
    gtk_box_pack_start (GTK_BOX (vbox), dsp->t1d_mbar, false, true, 0);

/*
 * Divide the window:  controls on the left, plot on the right
*/
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 1);
    gtk_box_pack_start (GTK_BOX (vbox),
                        hbox, true, true, 1);

/*
 * Controls
*/
    dsp->t1d_control_frame = gtk_frame_new (NULL);
    //gtk_frame_set_shadow_type (GTK_FRAME (dsp->t1d_control_frame), GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (dsp->t1d_control_frame), 5);
    gtk_box_pack_start (GTK_BOX (hbox),
                        dsp->t1d_control_frame, false, false, 1);

    vbc = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width (GTK_CONTAINER (vbc), 5);
    gtk_container_add (GTK_CONTAINER (dsp->t1d_control_frame), vbc);

/*
 * Optimize toggle
*/
    tgl = gtk_check_button_new_with_mnemonic ("_Optimize");
    gtk_widget_set_tooltip_text ((tgl), gg->tips ? ("Guide the tour using projection pursuit optimization or tour passively") : NULL);
    g_signal_connect (G_OBJECT (tgl), "toggled",
                        G_CALLBACK (t1d_optimz_cb), (gpointer) dsp);
    gtk_box_pack_start (GTK_BOX (vbc),
                      tgl, false, false, 1);

/*
 * Box to hold temp start and cooling controls
*/
    hb = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);

    vb = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

	lbl = gtk_label_new_with_mnemonic ("_Temp start:");
    gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  /*-- value, lower, upper, step --*/
    adj = GTK_ADJUSTMENT (gtk_adjustment_new (1.0, 0.1, 3.0, 0.1, 0.1, 0.0));
    g_signal_connect (G_OBJECT (adj), "value_changed",
                      G_CALLBACK (t1d_pptemp_set_cb), dsp);

    sbar = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, adj);
	gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), sbar);
    gtk_widget_set_name (sbar, "TOUR1D:PP_TEMPST");
    gtk_widget_set_tooltip_text ((sbar), gg->tips ? ("Adjust starting temp of pp") : NULL);
    gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
    gtk_scale_set_digits (GTK_SCALE (sbar), 2);

    gtk_box_pack_start (GTK_BOX (vb), sbar,
      false, false, 0);
    gtk_box_pack_start (GTK_BOX (hb), vb,
      false, false, 0);

  /*-- value, lower, upper, step --*/
    vb = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

	lbl = gtk_label_new_with_mnemonic ("_Cooling:");
    gtk_box_pack_start (GTK_BOX (vb), lbl,
      false, false, 0);

    adj = GTK_ADJUSTMENT (gtk_adjustment_new (0.99, 0.5, 1.0, 0.05, 0.05, 0.0));
    g_signal_connect (G_OBJECT (adj), "value_changed",
                      G_CALLBACK (t1d_ppcool_set_cb), dsp);

    sbar = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, adj);
	gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), sbar);
    gtk_widget_set_name (sbar, "TOUR1D:PP_COOLING");
    gtk_widget_set_tooltip_text ((sbar), gg->tips ? ("Adjust cooling") : NULL);
    gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
    gtk_scale_set_digits (GTK_SCALE (sbar), 2);

    gtk_box_pack_start (GTK_BOX (vb), sbar,
      false, false, 0);
    gtk_box_pack_start (GTK_BOX (hb), vb,
      false, false, 0);

    gtk_box_pack_start (GTK_BOX (vbc), hb, false, false, 0);

/*
 * Index value with label
*/
    hb = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
    gtk_box_pack_start (GTK_BOX (vbc), hb, false, false, 2);

    dsp->t1d_pplabel = gtk_label_new ("PP index: (0.00) 0.0000 (0.00)");
    gtk_label_set_xalign (GTK_LABEL (dsp->t1d_pplabel), 0);
    gtk_label_set_yalign (GTK_LABEL (dsp->t1d_pplabel), 0.5);
    gtk_box_pack_start (GTK_BOX (hb), dsp->t1d_pplabel, false, false, 0);
    gtk_widget_set_tooltip_text ((dsp->t1d_pplabel), gg->tips ? ("The value of the projection pursuit index for the current projection") : NULL);

    /*    entry = gtk_entry_new_with_max_length (32);
    gtk_entry_set_editable (GTK_ENTRY (entry), false);
    gtk_entry_set_text (GTK_ENTRY (entry), "0");
    gtk_box_pack_start (GTK_BOX (hb), entry, false, false, 2);
    gtk_widget_set_tooltip_text ((entry), gg->tips ? ("The value of the projection pursuit index for the current projection") : NULL);
    g_signal_connect (G_OBJECT (entry), "value_changed",
    G_CALLBACK (t1d_writeindx_cb), gg);*/
    /*    g_signal_connect (G_OBJECT (dsp->t1d.ppval), "value_changed",
            G_CALLBACK (t1d_writeindx_cb), gg);*/

/*
 * pp index menu and scale inside frame
*/
    /*    frame = gtk_frame_new ("PP index function");
    //gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbc), frame, false, false, 0);
    */

    vb = gtk_box_new (GTK_ORIENTATION_VERTICAL, 3);
    gtk_box_pack_start (GTK_BOX (vbc), vb, false, false, 2);
    /*    gtk_container_add (GTK_CONTAINER (frame), vb);*/

    opt = gtk_combo_box_text_new ();
    //gtk_container_set_border_width (GTK_CONTAINER (opt), 4);

    gtk_widget_set_tooltip_text ((opt), gg->tips ? ("Set the projection pursuit index") : NULL);
    gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
    /*  gtk_box_pack_start (GTK_BOX (hb), opt, false, false, 0);*/
    populate_combo_box (opt, t1d_pp_func_lbl, G_N_ELEMENTS(t1d_pp_func_lbl),
      G_CALLBACK(t1d_pp_func_cb), (gpointer) dsp);

    /*    param_vb = gtk_box_new (GTK_ORIENTATION_VERTICAL, 3);
    gtk_container_set_border_width (GTK_CONTAINER (param_vb), 4);
    gtk_box_pack_start (GTK_BOX (vb), param_vb, false, false, 2);

    param_lbl = gtk_label_new ("Terms in expansion:");
    gtk_label_set_xalign (GTK_LABEL (param_lbl), 0);
    gtk_label_set_yalign (GTK_LABEL (param_lbl), 0.5);
    gtk_box_pack_start (GTK_BOX (param_vb), param_lbl, false, false, 0);

    param_adj = (GtkAdjustment *) gtk_adjustment_new (1.0,
                                                      1.0, 30.0,
                                                      1.0, 1.0, 0.0);
    param_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT (param_adj));
    gtk_widget_set_tooltip_text ((param_scale), gg->tips ? ("Set number of terms in the expansion for some indices; bandwidth for others") : NULL);
    gtk_scale_set_digits (GTK_SCALE (param_scale), 0);
    gtk_scale_set_value_pos (GTK_SCALE (param_scale), GTK_POS_BOTTOM);

    gtk_box_pack_start (GTK_BOX (param_vb), param_scale, true, true, 0);
    */

/*
 * Drawing area in a frame
*/
    frame = gtk_frame_new (NULL);
    //gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_box_pack_start (GTK_BOX (hbox),
                        frame, true, true, 1);

    dsp->t1d_ppda = gtk_drawing_area_new ();
    gtk_widget_set_size_request (GTK_WIDGET (dsp->t1d_ppda), WIDTH, HEIGHT);
    g_signal_connect (G_OBJECT (dsp->t1d_ppda),
                        "configure_event",
                        G_CALLBACK(ppda_configure_cb),
                        (gpointer) dsp);

    g_signal_connect (G_OBJECT (dsp->t1d_ppda),
                        "draw",
                        G_CALLBACK(ppda_draw_cb),
                        (gpointer) dsp);

    gtk_container_add (GTK_CONTAINER (frame), dsp->t1d_ppda);

    gtk_widget_show_all (dsp->t1d_window);


  }

  alloc_optimize0_p(&dsp->t1d_pp_op, d->nrows_in_plot, dsp->t1d.nactive, 1);
  alloc_pp(&dsp->t1d_pp_param, d->nrows_in_plot, dsp->t1d.nactive, 1);

  gtk_widget_show_all (dsp->t1d_window);
}

#undef SUBD           
#undef LDA            
#undef CGINI      
#undef CENTROPY   
#undef CART_VAR       
#undef PCA            
