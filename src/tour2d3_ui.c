/* tour2d3_ui.c */
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

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include "vars.h"
#include "externs.h"
#include "tour.h"

/* This function initializes the rotation variables - it should only be
   called once, when a new tour is started since a new
   subset of variable might be used, or when there is new data. */


void 
cpanel_t2d3_init (cpaneld *cpanel, ggobid *gg) {
  cpanel->t2d3.step = TOURSTEP0;
  cpanel->t2d3.paused = false;
  cpanel->t2d3.slidepos = sessionOptions->defaultTourSpeed;
  cpanel->t2d3.manip_mode = MANIP_OBLIQUE;
}

void
cpanel_tour2d3_set (displayd *display, cpaneld *cpanel, ggobid* gg)
/*
 * To handle the case where there are multiple scatterplots
 * which may have different tour options and parameters selected
*/
{
  GtkWidget *w, *btn;
  GtkWidget *pnl = mode_panel_get_by_name(GGOBI(getPModeName)(TOUR2D3), gg);
  GtkAdjustment *adj;

  /*-- speed --*/
  w = widget_find_by_name (pnl, "TOUR2D3:speed_bar");
  adj = gtk_range_get_adjustment (GTK_RANGE (w));
  gtk_adjustment_set_value (GTK_ADJUSTMENT (adj),
    cpanel->t2d3.slidepos);

  /*-- paused --*/
  btn = widget_find_by_name (pnl, "TOUR2D3:pause_button");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (btn), cpanel->t2d3.paused);

  /*-- manual manip --*/
  w = widget_find_by_name (pnl, "TOUR2D3:manip");
  if (w)
    gtk_combo_box_set_active (GTK_COMBO_BOX (w), cpanel->t2d3.manip_mode);
}


static void
speed2d3_set_cb (GtkAdjustment *adj, ggobid *gg)
{
  tour2d3_speed_set(adj->value, gg);
}

static void
tour2d3_pause_cb (GtkToggleButton *button, ggobid *gg)
{
  tour2d3_pause (&gg->current_display->cpanel, button->active, gg);
}

static void reinit_cb (GtkWidget *w, ggobid *gg)
{
  tour2d3_reinit(gg);
}

static void scramble_cb (GtkWidget *w, ggobid *gg)
{
  tour2d3_scramble(gg);
}

static gchar *manip_lbl[] = {"Off", "Oblique", "Vert", "Horiz", "Radial",
                             "Angular"};
static void
manip_cb (GtkWidget *w, ggobid *gg)
{
  displayd *dsp = gg->current_display;
  cpaneld *cpanel = &dsp->cpanel;
  splotd *sp = gg->current_splot;

  cpanel->t2d3.manip_mode = gtk_combo_box_get_active(GTK_COMBO_BOX(w));

  if (cpanel->t2d3.manip_mode == MANIP_OFF)
    splot_cursor_set ((gint) NULL, sp);
  else
    splot_cursor_set (GDK_HAND2, sp);
}

void
cpanel_tour2d3_make (ggobid *gg) {
  modepaneld *panel;
  GtkWidget *box, *btn, *sbar, *lbl, *vb;
  GtkObject *adj;
  GtkWidget *manip_opt;
  
  panel = (modepaneld *) g_malloc(sizeof(modepaneld));
  gg->control_panels = g_list_append(gg->control_panels, (gpointer) panel);
  panel->name = g_strdup(GGOBI(getPModeName)(TOUR2D3));
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
                      G_CALLBACK (speed2d3_set_cb), (gpointer) gg);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_name (sbar, "TOUR2D3:speed_bar");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
    "Adjust speed of tour motion", NULL);
  scale_set_default_values (GTK_SCALE (sbar));

  gtk_box_pack_start (GTK_BOX (panel->w), sbar,
    false, false, 1);

/*
 * Box to hold 'pause' toggle button
*/
  box = gtk_hbox_new (true, 1);

  btn = gtk_check_button_new_with_mnemonic ("_Pause");
  gtk_widget_set_name (btn, "TOUR2D3:pause_button");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Stop tour motion temporarily (keyboard shortcut: w)", NULL);
  g_signal_connect (G_OBJECT (btn), "toggled",
                     G_CALLBACK (tour2d3_pause_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  gtk_box_pack_start (GTK_BOX (panel->w), box,
    false, false, 1);

/*
 * Box to hold 'Reinit' toggle and 'Scramble' button
*/
  box = gtk_hbox_new (true, 2);

  btn = gtk_button_new_with_mnemonic ("_Reinit");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Reset projection to first two active variables", NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
                     G_CALLBACK (reinit_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  btn = gtk_button_new_with_mnemonic ("Scr_amble");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Reset projection to random value", NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
                     G_CALLBACK (scramble_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  gtk_box_pack_start (GTK_BOX (panel->w), box,
    false, false, 1);

/*
 * manipulation option menu and label inside vbox
*/

  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (panel->w), vb,
    false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("_Manual manipulation:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  manip_opt = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), manip_opt);
  gtk_widget_set_name (manip_opt, "TOUR2D3:manip");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), manip_opt,
    "Set the manual manipulation method", NULL);
  gtk_box_pack_end (GTK_BOX (vb), manip_opt, false, false, 0);
  populate_combo_box (manip_opt, manip_lbl, G_N_ELEMENTS(manip_lbl),
    G_CALLBACK(manip_cb), gg);

  gtk_widget_show_all (panel->w);
}

/*----------------------------------------------------------------------*/
/*                              I/O events                              */
/*----------------------------------------------------------------------*/

/*-- called from the Options menu --*/
void tour2d3_io_cb (GtkWidget *w, gpointer *cbd) {
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
    GtkWidget *pnl = mode_panel_get_by_name(GGOBI(getPModeName)(TOUR2D3), gg);
    GtkWidget *pause_button = NULL;
    pause_button = widget_find_by_name (pnl, "TOUR2D3:pause_button");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pause_button),
      !cpanel->t2d3.paused);
	return true;
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

  tour2d3_manip(sp->mousepos.x, sp->mousepos.y, sp, gg);
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
  if (cpanel->t2d3.manip_mode != MANIP_OFF) 
  {
    sp->motion_id = g_signal_connect (G_OBJECT (sp->da),
                                      "motion_notify_event",
                                      G_CALLBACK(motion_notify_cb),
                                      (gpointer) sp);
    tour2d3_manip_init(sp->mousepos.x, sp->mousepos.y, sp);
  }

  return true;
}
static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;
  GdkModifierType state;

  gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y, &state);

  tour2d3_manip_end(sp);

  gdk_pointer_ungrab (event->time);

  return retval;
}

void
tour2d3_event_handlers_toggle (splotd *sp, gboolean state) {
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
