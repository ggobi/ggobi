/* tourcorr_ui.c */
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

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include "vars.h"
#include "externs.h"
#include "tour.h"

/* This function initializes the tour variables - it should only be
   called more than once, when a new tour is started since a new
   subset of variable might be used, or when there is new data. */

void 
cpanel_tcorr_init (cpaneld *cpanel, ggobid *gg) {
  cpanel->tcorr1.paused = false;
  cpanel->tcorr1.step = TOURSTEP0;
  cpanel->tcorr2.paused = false;
  cpanel->tcorr2.step = TOURSTEP0;
  cpanel->tcorr.slidepos = sessionOptions->defaultTourSpeed;
  cpanel->tcorr.manip_mode = CMANIP_COMB;
}

/*-- scatterplot only; need a different routine for parcoords --*/
void
cpanel_tourcorr_set (displayd *display, cpaneld *cpanel, ggobid* gg)
/*
 * To handle the case where there are multiple scatterplots
 * which may have different tour options and parameters selected
*/
{
  GtkWidget *pnl, *w, *btn;
  GtkAdjustment *adj;

  pnl = (GtkWidget *) mode_panel_get_by_name(GGOBI(getPModeName)(COTOUR), gg);

  /*-- speed --*/
  w = widget_find_by_name (pnl, "COTOUR:speed_bar");
  adj = gtk_range_get_adjustment (GTK_RANGE (w));
  gtk_adjustment_set_value (GTK_ADJUSTMENT (adj),
    cpanel->tcorr.slidepos);

  /*-- paused --*/
  btn = widget_find_by_name (pnl, "COTOUR:pause_button");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (btn), cpanel->tcorr1.paused);

  /*-- manual manip --*/
  w = widget_find_by_name (pnl, "COTOUR:manip");
  if (w)
    gtk_combo_box_set_active (GTK_COMBO_BOX (w), cpanel->tcorr.manip_mode);

  /*-- PC axes --*/
  /*-- backtracking --*/
  /*-- local scan --*/
  /*-- path len... --*/
}

#ifdef TOUR_PP_IMPLEMENTED
static void ctouradv_window_open (void);
#endif

static void speedcorr_set_cb (GtkAdjustment *adj, ggobid *gg) {

  tourcorr_speed_set(adj->value, gg);
}

static void tourcorr_pause_cb (GtkToggleButton *button, ggobid *gg)
{

  tourcorr_pause (&gg->current_display->cpanel, button->active, gg);
}

static void tourcorr_reinit_cb (GtkWidget *w, ggobid *gg) {

  tourcorr_reinit(gg);
}

static void tourcorr_scramble_cb (GtkWidget *w, ggobid *gg) {

  tourcorr_scramble(gg);
}

static void tourcorr_snap_cb (GtkWidget *w, ggobid *gg) {

  tourcorr_snap(gg);
}

static void tourcorr_video_cb (GtkToggleButton *button, ggobid *gg)
{
  tourcorr_video(gg);
}

/*
static void syncaxes_cb (GtkToggleButton *button)
{
  g_printerr ("syncaxes: %d\n", button->active);
}
*/
/*
static void
hide_cb (GtkWidget *w ) {
  gtk_widget_hide (w);
}
*/

#ifdef TOUR_PP_IMPLEMENTED
static void ctourpp_cb (GtkWidget *w, ggobid *gg) {
  ctourpp_window_open (gg);
}
#endif
#ifdef TOUR_ADV_IMPLEMENTED
static void ctouradv_cb (GtkWidget *w, gpointer dummy) {
  ctouradv_window_open ();
}
#endif

static gchar *manip_lbl[] = {"Off", "Comb", "Vertical", "Horizontal", 
                             "EqualComb"};
static void manip_cb (GtkWidget *w, ggobid *gg)
{
  displayd *dsp = gg->current_display;
  cpaneld *cpanel = &dsp->cpanel;
  splotd *sp = gg->current_splot;

  cpanel->tcorr.manip_mode = gtk_combo_box_get_active(GTK_COMBO_BOX(w));

  if (cpanel->tcorr.manip_mode == CMANIP_OFF)
    splot_cursor_unset (sp);
  else
    splot_cursor_set (GDK_HAND2, sp);
}
/*
static gchar *pathlen_lbl[] = {"1/10", "1/5", "1/4", "1/3", "1/2", "1",
                               "2", "10", "Infinite"};
static void pathlen_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", pathlen_lbl[indx]);
}
*/

void
cpanel_ctour_make (ggobid *gg) {
  modepaneld *panel;
  GtkWidget *box, *btn, *sbar, *vb, *lbl;
  GtkObject *adj;
  GtkWidget *manip_opt;
  /* GtkWidget *pathlen_opt, *tgl; */
  
  panel = (modepaneld *) g_malloc(sizeof(modepaneld));
  gg->control_panels = g_list_append(gg->control_panels, (gpointer) panel);
  panel->name = g_strdup(GGOBI(getPModeName)(COTOUR));
  panel->w = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (panel->w), 5);

/*
 * speed scrollbar
*/
  /* value, lower, upper, step_increment, page_increment, page_size */
  /* Note that the page_size value only makes a difference for
   * scrollbar widgets, and the highest value you'll get is actually
   * (upper - page_size). */
  adj = gtk_adjustment_new (sessionOptions->defaultTourSpeed, 0.0, MAX_TOUR_SPEED, 1.0, 1.0, 0.0);
  g_signal_connect (G_OBJECT (adj), "value_changed",
                      G_CALLBACK (speedcorr_set_cb), (gpointer) gg);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_name (sbar, "COTOUR:speed_bar");
  gtk_widget_set_tooltip_text (sbar,
    "Adjust speed of tour motion");
  scale_set_default_values (GTK_SCALE (sbar));

  gtk_box_pack_start (GTK_BOX (panel->w), sbar,
    false, false, 1);

/*
 * Box to hold 'pause' toggle button
*/
  box = gtk_hbox_new (true, 1);

  btn = gtk_check_button_new_with_mnemonic ("_Pause");
  gtk_widget_set_name (btn, "COTOUR:pause_button");
  gtk_widget_set_tooltip_text (btn,
    "Stop tour motion temporarily (keyboard shortcut: w)");
  g_signal_connect (G_OBJECT (btn), "toggled",
                     G_CALLBACK (tourcorr_pause_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  gtk_box_pack_start (GTK_BOX (panel->w),
    box, false, false, 1);
/*
 * Box to hold 'Reinit' toggle and 'Scramble' button
*/
  box = gtk_hbox_new (true, 2);

  btn = gtk_button_new_with_mnemonic("_Reinit");
  gtk_widget_set_tooltip_text (btn,
    "Reset projection");
  g_signal_connect (G_OBJECT (btn), "clicked",
                     G_CALLBACK (tourcorr_reinit_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  btn = gtk_button_new_with_mnemonic ("Scr_amble");
  gtk_widget_set_tooltip_text (btn,
    "Reset projection to random value");
  g_signal_connect (G_OBJECT (btn), "clicked",
                     G_CALLBACK (tourcorr_scramble_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  gtk_box_pack_start (GTK_BOX (panel->w),
    box, false, false, 1);

/*
 * snapshot and video stream controls
  box = gtk_hbox_new (true, 2);

   btn = gtk_button_new_with_mnemonic ("_Snap");
  gtk_widget_set_tooltip_text (btn,
    "Take a snapshot of this frame to re-generate plot outside ggobi");
  g_signal_connect (G_OBJECT (btn), "clicked",
                     G_CALLBACK (tourcorr_snap_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  btn = gtk_check_button_new_with_mnemonic ("Vid_eo");
  gtk_widget_set_tooltip_text (btn,
    "Save sequence of projection frames out to file");
  g_signal_connect (G_OBJECT (btn), "toggled",
                     G_CALLBACK (tourcorr_video_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  gtk_box_pack_start (GTK_BOX (panel->w), box, false, false, 1);
 */

/*
 * manipulation option menu with label
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (panel->w), vb, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("_Manual manipulation:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  manip_opt = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), manip_opt);
  gtk_widget_set_name (manip_opt, "COTOUR:manip");
  //gtk_container_set_border_width (GTK_CONTAINER (manip_opt), 4);
  gtk_widget_set_tooltip_text (manip_opt,
    "Set the manual manipulation method", NULL);
  gtk_container_add (GTK_CONTAINER (vb), manip_opt);
  populate_combo_box (manip_opt, manip_lbl, G_N_ELEMENTS(manip_lbl),
    G_CALLBACK(manip_cb), (gpointer) gg);

/*
 * path length option menu 
*/
  /*  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (panel->w), vb, false, false, 0);

  lbl = gtk_label_new ("Path length:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  pathlen_opt = gtk_option_menu_new ();
  gtk_container_set_border_width (GTK_CONTAINER (pathlen_opt), 4);
  gtk_widget_set_tooltip_text (pathlen_opt,
    "Set the path length");
  gtk_container_add (GTK_CONTAINER (vb), pathlen_opt);
  populate_option_menu (pathlen_opt, pathlen_lbl,
    sizeof (pathlen_lbl) / sizeof (gchar *),
    G_CALLBACK(pathlen_cb), "GGobi", gg);
  */
/*
 * Sync Axes toggle
*/
  /*  tgl = gtk_check_button_new_with_label ("Sync axes");
  gtk_widget_set_tooltip_text (tgl,
    "Synchronize the horizontal and vertical axes");
  g_signal_connect (G_OBJECT (tgl), "toggled",
                      G_CALLBACK (syncaxes_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (panel->w),
                      tgl, false, false, 1);
  */
/*
 * projection pursuit button
*/
  /*  btn = gtk_button_new_with_label ("Projection pursuit ...");
  gtk_widget_set_tooltip_text (btn,
    "Open panel for correlation tour projection pursuit");
  gtk_box_pack_start (GTK_BOX (panel->w),
                      btn, false, false, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (ctourpp_cb), gg);
  */
/*
 * advanced features button
*/
  /*  btn = gtk_button_new_with_label ("Advanced features ...");
  gtk_widget_set_tooltip_text (btn,
    "Open panel for additional correlation tour features");
  gtk_box_pack_start (GTK_BOX (panel->w),
                      btn, false, false, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (ctouradv_cb), NULL);
  */
  gtk_widget_show_all (panel->w); 
  
}

/*----------------------------------------------------------------------*/
/*               Advanced features panel and callbacks                  */
/*----------------------------------------------------------------------*/

/*
The following are considered advanced features for now:
  history
*/

#ifdef TOUR_PP_IMPLEMENTED
static GtkWidget *window = NULL;
static void
ctouradv_window_open (void) {
  GtkWidget *vbox, *btn, *frame;

  if (window == NULL) {
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Advanced Correlation Tour");
    
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    
    frame = gtk_frame_new ("History");
    //gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame, false, false, 0);

    btn = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
    g_signal_connect_swapped (G_OBJECT (btn), "clicked",
                   G_CALLBACK (hide_cb), (GtkObject*) window);
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, true, 2);
  }

  gtk_widget_show_all (window);
}
#endif

/*----------------------------------------------------------------------*/
/*                              I/O events                              */
/*----------------------------------------------------------------------*/

/*-- called from the Options menu --*/
void tourcorr_io_cb (GtkWidget *w, gpointer *cbd) {
/*
  gchar *lbl = (gchar *) cbd;
  g_printerr ("cbd: %s\n", lbl);
*/
}

/*--------------------------------------------------------------------*/
/*      Handling keyboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

static gint
key_press_cb (GtkWidget *w, GdkEventKey *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot(sp);
  cpaneld *cpanel = &gg->current_display->cpanel;
  
/*-- add a key_press_cb in each mode, and let it begin with these lines --*/
  if (splot_event_handled (w, event, cpanel, sp, gg))
    return true;

  /*-- insert mode-specific key presses (if any) here --*/
  if (event->keyval == GDK_w || event->keyval == GDK_W) {
    /*-- turn pause on and off --*/
    GtkWidget *pnl;
    GtkWidget *pause_button = NULL;
    pnl = mode_panel_get_by_name(GGOBI(getPModeName)(COTOUR), gg);
    if (pnl) {
      pause_button = widget_find_by_name (pnl, "COTOUR:pause_button");
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pause_button),
        !cpanel->tcorr1.paused || !cpanel->tcorr2.paused);
	  return true;
    }
  }

  return false;
}

static gint
motion_notify_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot(sp);
  gboolean button1_p, button2_p;

  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);

  /*-- if neither button is pressed, we shouldn't have gotten the event --*/
  if (!button1_p && !button2_p)
    return false;

  tourcorr_manip(sp->mousepos.x, sp->mousepos.y, sp, gg);

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  ggobid *gg = GGobiFromWidget(w, true);
  displayd *dsp = gg->current_display;
  cpaneld *cpanel = &dsp->cpanel;
  gboolean button1_p, button2_p;
  mousepos_get_pressed (w, event, &button1_p, &button2_p, sp);

  if (cpanel->tcorr.manip_mode != CMANIP_OFF) 
  {
    sp->motion_id = g_signal_connect (G_OBJECT (sp->da),
                                      "motion_notify_event",
                                      G_CALLBACK(motion_notify_cb),
                                      (gpointer) sp);

    tourcorr_manip_init(sp->mousepos.x, sp->mousepos.y, sp);
  }

  return true;
}
static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;
  GdkModifierType state;

  gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y, &state);

  tourcorr_manip_end(sp);

  gdk_pointer_ungrab (event->time);

  return retval;
}

void
ctour_event_handlers_toggle (splotd *sp, gboolean state) {
  displayd *display = (displayd *) sp->displayptr;

  if (state == on) {
    if(GGOBI_IS_WINDOW_DISPLAY(display) && GGOBI_WINDOW_DISPLAY(display)->useWindow)
      sp->key_press_id = g_signal_connect (G_OBJECT (GGOBI_WINDOW_DISPLAY(display)->window),
        "key_press_event",
        G_CALLBACK(key_press_cb),
        (gpointer) sp);
    sp->press_id = g_signal_connect (G_OBJECT (sp->da),
                                       "button_press_event",
                                       G_CALLBACK(button_press_cb),
                                       (gpointer) sp);
    sp->release_id = g_signal_connect (G_OBJECT (sp->da),
                                         "button_release_event",
                                         G_CALLBACK(button_release_cb),
                                         (gpointer) sp);
  } else {
    disconnect_key_press_signal (sp);
    disconnect_button_press_signal (sp);
    disconnect_button_release_signal (sp);
  }
}


