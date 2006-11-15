/* tour2d_pp_ui.c */
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

#define WIDTH   200
#define HEIGHT  100


/* terms in expansion, bandwidth */
/*
static GtkWidget *param_vb, *param_lbl, *param_scale;
static GtkAdjustment *param_adj;
*/

/*-- called when closed from the close menu item --*/
static void action_close_cb (GtkAction *action, displayd *dsp) {
  gtk_widget_hide (dsp->t2d_window);
  t1d_optimz(0, &dsp->t2d.get_new_target, 
    &dsp->t2d.target_selection_method, dsp);

  /*  free_optimize0_p(&dsp->t2d_pp_op); * should this go here? *
  free_pp(&dsp->t2d_pp_param); seems not, causes a crash because window
                               just gets hidden, so shouldn't
                               free the arrays. */
}


/*-- called when destroyed from the window manager --*/
static void
close_wmgr_cb (GtkWidget *w, GdkEventButton *event, displayd *dsp) 
{
  gtk_widget_hide (dsp->t2d_window);
  t2d_optimz(0, &dsp->t2d.get_new_target, 
    &dsp->t2d.target_selection_method, dsp);

  free_optimize0_p(&dsp->t2d_pp_op);
  free_pp(&dsp->t2d_pp_param);
  gtk_widget_destroy (dsp->t2d_window);
  dsp->t2d_window = NULL;
}

/*
static void
hide_cb (GtkWidget *w) {
  gtk_widget_hide (w);
}
*/

static void
action_show_controls_cb(GtkToggleAction *action, displayd *dsp) {
      if (gtk_toggle_action_get_active(action))
        gtk_widget_show (dsp->t1d_control_frame);
      else
        gtk_widget_hide (dsp->t1d_control_frame);
}

/*static void
line_options_cb(gpointer data, guint action, GtkCheckMenuItem *w) {

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
t2d_optimz_cb (GtkToggleButton  *w, displayd *dsp) 
{
  if (dsp == NULL) {
    g_printerr ("No display corresponds to these projection pursuit controls; close this panel.\n");
    return;
  }

  t2d_optimz(w->active, &dsp->t2d.get_new_target, &dsp->t2d.target_selection_method, dsp);
}

static void t2d_pptemp_set_cb (GtkAdjustment *adj, displayd *dsp) {

  t2d_pptemp_set(adj->value, dsp, dsp->d->gg);
}

static void t2d_ppcool_set_cb (GtkAdjustment *adj, displayd *dsp) {

  t2d_ppcool_set(adj->value, dsp, dsp->d->gg);
}

/*
static void
sphere_cb (GtkWidget  *w, ggobid *gg) {

  sphere_panel_open(gg);
}
*/

#include "tour_pp.h"

static gchar *t2d_pp_func_lbl[] = {"Holes","Central Mass","LDA","Gini-C","Entropy-C"};

TourPPIndex StandardPPIndices[] = {
    {"Holes", &holes_raw, false, NULL},
    {"Central Mass", &central_mass_raw, false, NULL},
    {"LDA", &discriminant, true, NULL},
    {"Gini-C", &cartgini, true, NULL},
    {"Entropy-C", &cartentropy, true, NULL},
};


void t2d_pp_func_cb (GtkWidget *w, displayd *dsp)
{
  ggobid *gg;
  cpaneld *cpanel = NULL;
  gint indx = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
  gchar *label = g_strdup("PP index: (0.000) 0.0000 (0.000)");

  if (dsp == NULL) {
    g_printerr ("No display corresponds to these controls\n");
    return;
  }
  gg = GGobiFromDisplay (dsp);

  cpanel = &dsp->cpanel;
  cpanel->t2d.pp_indx = indx;
  cpanel->t2d.ppindex = StandardPPIndices[indx];
  dsp->t2d.get_new_target = true;

  dsp->t2d.ppval = 0.0;
  dsp->t2d.oppval = -1.0;
  dsp->t2d_pp_op.index_best = 0.0;
  sprintf(label,"PP index: (%3.1f) %5.3f (%3.1f) ", 0.0, dsp->t2d.ppval, 0.0);
  gtk_label_set_text(GTK_LABEL(dsp->t2d_pplabel),label);

  t2d_clear_ppda(dsp, gg);

  /*
  if (indx == SUBD || LDA || CART_GINI || CART_ENTROPY || CART_VAR || PCA)
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
t2d_ppda_configure_cb (GtkWidget *w, GdkEventConfigure *event, displayd *dsp)
{
  gint wid = w->allocation.width, hgt = w->allocation.height;

  if (dsp->t2d_pp_pixmap != NULL)
    gdk_pixmap_unref (dsp->t2d_pp_pixmap);

  dsp->t2d_pp_pixmap = gdk_pixmap_new (dsp->t2d_ppda->window,
    wid, hgt, -1);

  return false;
}

static gint
t2d_ppda_expose_cb (GtkWidget *w, GdkEventConfigure *event, displayd *dsp)
{
  ggobid *gg = dsp->d->gg;
/*
  gint margin=10;
  gint j;
  gint xpos, ypos, xstrt, ystrt;
  gchar *tickmk;
  GtkStyle *style = gtk_widget_get_style (dsp->t2d_ppda);
  GGobiStage *d = dsp->d;
*/
  gint wid = w->allocation.width, hgt = w->allocation.height;
  /*  static gboolean init = true;*/

  /*  if (init) {
    t2d_clear_ppda(dsp, gg);
    init=false;
    }*/

  gdk_draw_pixmap (dsp->t2d_ppda->window, gg->plot_GC, dsp->t2d_pp_pixmap,
                   0, 0, 0, 0,
                   wid, hgt);

  return false;
}

static const gchar* tour2dpp_ui =
"<ui>"
"	<menubar>"
"		<menu action='File'>"
"			<menuitem action='Close'/>"
"		</menu>"
"		<menu action='Options'>"
"			<menuitem action='ShowControls'/>"
"		</menu>"
"	</menubar>"
"</ui>";

static GtkActionEntry entries[] = {
	{ "File", NULL, "_File" },
	{ "Close", GTK_STOCK_CLOSE, "_Close", "<control>C", 
		"Hide the projection pursuit window", G_CALLBACK(action_close_cb)
	},
	{ "Options", NULL, "_Options" }
};
static GtkToggleActionEntry t_entries[] = {
	{ "ShowControls", NULL, "_Show controls", "<control>S",
		"Hide the controls on the left so that the graph consumes the entire window",
		G_CALLBACK(action_show_controls_cb), true
	}
};

static void
rows_added_cb(GGobiStage *stage, guint n, ggobid *gg)
{
  reset_pp(stage, gg);
}
static void
rows_deleted_cb(GGobiStage *stage, guint *rows, guint n, ggobid *gg)
{
  reset_pp(stage, gg);
}

void
tour2dpp_window_open (ggobid *gg) {
  /*GtkWidget **btn, *label, *da, *entry;*/
  GtkWidget *hbox, *vbox, *vbc, *vb, *frame, *tgl, *hb, *opt, *sbar, *lbl;
  GtkObject *adj;
  displayd *dsp = gg->current_display;  /* ok as long as we only use the gui */
  GGobiStage *d = dsp->d;
  gboolean vars_sphered = true;
  /*-- to initialize the checkboxes in the menu --*/
  GtkWidget *item;

  if (dsp == NULL)
    return;

  /* check if selected variables are sphered beforeing allowing window
     to popup */
  /*  if (dsp->t2d.nactive > d->sphere.pcvars.nels)
    vars_sphered = false;
  gint i, j;
  for (j=0; j<dsp->t2d.nactive; j++) 
  {
    for (i=0; i<d->sphere.pcvars.nels; i++) 
      if (dsp->t2d.active_vars.els[j] == d->sphere.pcvars.els[i])
        break;
    if ((i == d->sphere.pcvars.nels-1) && 
       (dsp->t2d.active_vars.els[j] != d->sphere.pcvars.els[i]))
    {
      vars_sphered = false;
      break;
    }
    }*/

  if (!vars_sphered) {

    quick_message ("The selected variables have to be sphered first. Use the Tools menu and select the sphering option.", false);

  } else {

    if (dsp->t2d_window == NULL) {
		GtkUIManager *manager = gtk_ui_manager_new();
		GtkActionGroup *actions = gtk_action_group_new("Tour2DPPActions");
		
      dsp->t2d_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_window_set_title (GTK_WINDOW (dsp->t2d_window), 
        "Projection Pursuit - 2D");
      g_signal_connect (G_OBJECT (dsp->t2d_window), "delete_event",
                          G_CALLBACK (close_wmgr_cb), (gpointer) dsp);
      /*gtk_window_set_policy (GTK_WINDOW (dsp->t2d_window), true, true, false);*/
      g_signal_connect (G_OBJECT(d), "rows-added",
        G_CALLBACK(rows_added_cb), gg);
      g_signal_connect (G_OBJECT(d), "rows-deleted",
        G_CALLBACK(rows_deleted_cb), gg);

      gtk_container_set_border_width (GTK_CONTAINER (dsp->t2d_window), 10);
      g_object_set_data(G_OBJECT (dsp->t2d_window), "displayd", dsp);

/*
 * Add the main menu bar
*/
      vbox = gtk_vbox_new (FALSE, 1);
      gtk_container_set_border_width (GTK_CONTAINER (vbox), 1);
      gtk_container_add (GTK_CONTAINER (dsp->t2d_window), vbox);

	  gtk_action_group_add_actions(actions, entries, G_N_ELEMENTS(entries), dsp);
	  gtk_action_group_add_toggle_actions(actions, t_entries, G_N_ELEMENTS(t_entries), dsp);
	  gtk_ui_manager_insert_action_group(manager, actions, 0);
	  g_object_unref(G_OBJECT(actions));
	  dsp->t2d_mbar = create_menu_bar(manager, tour2dpp_ui, dsp->t2d_window);
	
      /*dsp->t2d_pp_accel_group = gtk_accel_group_new ();
      factory = get_main_menu (menu_items,
        sizeof (menu_items) / sizeof (menu_items[0]),
        dsp->t2d_pp_accel_group, dsp->t2d_window, &dsp->t2d_mbar,
        (gpointer) dsp);*/
      gtk_box_pack_start (GTK_BOX (vbox), dsp->t2d_mbar, false, true, 0);


/*
 * Divide the window:  controls on the left, plot on the right
*/
      hbox = gtk_hbox_new (false, 1);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 1);
      gtk_box_pack_start (GTK_BOX (vbox),
                        hbox, true, true, 1);
/*
 * Controls
*/
      dsp->t2d_control_frame = gtk_frame_new (NULL);
      //gtk_frame_set_shadow_type (GTK_FRAME (dsp->t2d_control_frame), GTK_SHADOW_IN);
      gtk_container_set_border_width (GTK_CONTAINER (dsp->t2d_control_frame), 5);
      gtk_box_pack_start (GTK_BOX (hbox),
                        dsp->t2d_control_frame, false, false, 1);

      vbc = gtk_vbox_new (false, 5);
      gtk_container_set_border_width (GTK_CONTAINER (vbc), 5);
      gtk_container_add (GTK_CONTAINER (dsp->t2d_control_frame), vbc);

/*
 * Optimize toggle
*/
      tgl = gtk_check_button_new_with_mnemonic ("_Optimize");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
        "Guide the tour using projection pursuit optimization or tour passively",
        NULL);
      g_signal_connect (G_OBJECT (tgl), "toggled",
                          G_CALLBACK (t2d_optimz_cb), (gpointer) dsp);
      gtk_box_pack_start (GTK_BOX (vbc),
                        tgl, false, false, 1);

/*
 * Box to hold temp start and cooling controls
*/
    hb = gtk_hbox_new (true, 2);

    vb = gtk_vbox_new (false, 0);

    lbl = gtk_label_new_with_mnemonic ("_Temp start:");
    gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  /*-- value, lower, upper, step --*/
    adj = gtk_adjustment_new (1.0, 0.1, 3.0, 0.1, 0.1, 0.0);
    g_signal_connect (G_OBJECT (adj), "value_changed",
                      G_CALLBACK (t2d_pptemp_set_cb), dsp);

    sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
	gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), sbar);
    gtk_widget_set_name (sbar, "TOUR2D:PP_TEMPST");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
    "Adjust starting temp of pp", NULL);
    gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
    gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
    gtk_scale_set_digits (GTK_SCALE (sbar), 2);

    gtk_box_pack_start (GTK_BOX (vb), sbar,
      false, false, 0);
    gtk_box_pack_start (GTK_BOX (hb), vb,
      false, false, 0);

  /*-- value, lower, upper, step --*/
    vb = gtk_vbox_new (false, 0);

    lbl = gtk_label_new_with_mnemonic ("_Cooling:");
    gtk_box_pack_start (GTK_BOX (vb), lbl,
      false, false, 0);

    adj = gtk_adjustment_new (0.99, 0.50, 1.0, 0.05, 0.05, 0.0);
    g_signal_connect (G_OBJECT (adj), "value_changed",
                      G_CALLBACK (t2d_ppcool_set_cb), dsp);

    sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
	gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), sbar);
    gtk_widget_set_name (sbar, "TOUR2D:PP_COOLING");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
    "Adjust cooling", NULL);
    gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
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
      hb = gtk_hbox_new (false, 3);
      gtk_box_pack_start (GTK_BOX (vbc), hb, false, false, 2);
  
      dsp->t2d_pplabel = gtk_label_new ("PP index: 0.0000");
      gtk_misc_set_alignment (GTK_MISC (dsp->t2d_pplabel), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (hb), dsp->t2d_pplabel, false, false, 0);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), dsp->t2d_pplabel,
        "The value of the projection pursuit index for the current projection",
        NULL);

    /*    entry = gtk_entry_new_with_max_length (32);
    gtk_entry_set_editable (GTK_ENTRY (entry), false);
    gtk_entry_set_text (GTK_ENTRY (entry), "0");
    gtk_box_pack_start (GTK_BOX (hb), entry, false, false, 2);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
      "The value of the projection pursuit index for the current projection",
      NULL);
    g_signal_connect (G_OBJECT (entry), "value_changed",
    G_CALLBACK (t2d_writeindx_cb), gg);*/
    /*    g_signal_connect (G_OBJECT (dsp->t2d.ppval), "value_changed",
          G_CALLBACK (t2d_writeindx_cb), gg);*/

/*
 * pp index menu and scale inside frame
*/
    /*    frame = gtk_frame_new ("PP index function");
    //gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbc), frame, false, false, 0);
    */

    /*    vb = gtk_vbox_new (false, 3);*/
      vb = gtk_vbox_new (true, 2);
      gtk_box_pack_start (GTK_BOX (vbc), vb, false, false, 2);
    /*    gtk_container_add (GTK_CONTAINER (frame), vb);*/

      /*      btn = gtk_button_new_with_label ("Sphere Vars");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
        "Interface to sphering variables (principal components analysis)", NULL);
      g_signal_connect (G_OBJECT (btn), "clicked",
         G_CALLBACK (sphere_cb), (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (vb), btn, true, true, 1);
      */

      opt = gtk_combo_box_new_text ();
      //gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
/*
      gtk_misc_set_alignment (opt, 0, 0.5);
*/
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
        "Set the projection pursuit index", NULL);
      gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
      /*    gtk_box_pack_start (GTK_BOX (hb), opt, false, false, 0);*/
      populate_combo_box (opt, t2d_pp_func_lbl, G_N_ELEMENTS(t2d_pp_func_lbl),
        G_CALLBACK(t2d_pp_func_cb), (gpointer) dsp);

    /*    param_vb = gtk_vbox_new (false, 3);
    gtk_container_set_border_width (GTK_CONTAINER (param_vb), 4);
    gtk_box_pack_start (GTK_BOX (vb), param_vb, false, false, 2);

    param_lbl = gtk_label_new ("Terms in expansion:");
    gtk_misc_set_alignment (GTK_MISC (param_lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (param_vb), param_lbl, false, false, 0);

    param_adj = (GtkAdjustment *) gtk_adjustment_new (1.0,
                                                      1.0, 30.0,
                                                      1.0, 1.0, 0.0);
    param_scale = gtk_hscale_new (GTK_ADJUSTMENT (param_adj));
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), param_scale,
      "Set number of terms in the expansion for some indices; bandwidth for others", NULL);
    gtk_range_set_update_policy (GTK_RANGE (param_scale),
                                 GTK_UPDATE_CONTINUOUS);
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

      dsp->t2d_ppda = gtk_drawing_area_new ();
      gtk_widget_set_double_buffered(dsp->t2d_ppda, false);
      gtk_widget_set_size_request (GTK_WIDGET (dsp->t2d_ppda), WIDTH, HEIGHT);
      g_signal_connect (G_OBJECT (dsp->t2d_ppda),
                          "configure_event",
                          G_CALLBACK(t2d_ppda_configure_cb),
                          (gpointer) dsp);

      g_signal_connect (G_OBJECT (dsp->t2d_ppda),
                          "expose_event",
                          G_CALLBACK(t2d_ppda_expose_cb),
                          (gpointer) dsp);
  
      gtk_container_add (GTK_CONTAINER (frame), dsp->t2d_ppda);
      gtk_widget_show_all (dsp->t2d_window);

      /*-- Set the appropriate check menu items to true. -- dfs --*/
      item = gtk_ui_manager_get_widget (manager, "/menubar/Options/ShowControls");
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), true);

    }

    alloc_optimize0_p(&dsp->t2d_pp_op, d->n_rows, dsp->t2d.nactive, 2);
    alloc_pp(&dsp->t2d_pp_param, d->n_rows, dsp->t2d.nactive, 2);

    gtk_widget_show_all (dsp->t2d_window);
  }
}


