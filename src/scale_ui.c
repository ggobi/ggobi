/* scale_ui.c */
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
#include <string.h>
#include "vars.h"
#include "externs.h"
#include <math.h>

void
scale_update_set (gboolean update, displayd * dsp, ggobid * gg)
{
  cpaneld *cpanel = &dsp->cpanel;
  GtkWidget *panel =
    mode_panel_get_by_name (GGOBI (getIModeName) (SCALE), gg);
  GtkWidget *w;
  GtkUpdateType policy;

  cpanel->scale.updateAlways_p = update;

  if (cpanel->scale.updateAlways_p)
    policy = GTK_UPDATE_CONTINUOUS;
  else
    policy = GTK_UPDATE_DISCONTINUOUS;

  /* When the update policy changes, change the update policy of
     the range widgets as well */
  w = widget_find_by_name (panel, "SCALE:x_zoom");
  gtk_range_set_update_policy (GTK_RANGE (w), policy);
  w = widget_find_by_name (panel, "SCALE:y_zoom");
  gtk_range_set_update_policy (GTK_RANGE (w), policy);
  w = widget_find_by_name (panel, "SCALE:x_pan");
  gtk_range_set_update_policy (GTK_RANGE (w), policy);
  w = widget_find_by_name (panel, "SCALE:y_pan");
  gtk_range_set_update_policy (GTK_RANGE (w), policy);

}

/* Use the hscale widget name to find the corresponding adjustment */
static GtkAdjustment *
scale_adjustment_find_by_name (gchar * name, ggobid * gg)
{
  GtkWidget *panel, *w;

  panel = mode_panel_get_by_name ("Scale", gg);
  w = widget_find_by_name (panel, name);
  if (GTK_IS_HSCALE (w))
    return (gtk_range_get_adjustment (GTK_RANGE (w)));
  else return NULL;
}

static void
increment_adjustment (GtkAdjustment * adj, gdouble step, gdouble eps)
{
  gdouble value = adj->value + step;
  value = MAX (value, adj->lower);
  value = MIN (value, adj->upper);
  if (fabs (value - adj->value) > eps)
    gtk_adjustment_set_value (adj, value);
}

static void
zoom_cb (GtkAdjustment * adj, ggobid * gg)
{
  displayd *display = gg->current_display;
  splotd *sp = gg->current_splot;
  cpaneld *cpanel = &display->cpanel;
  gchar *name = (gchar *) g_object_get_data (G_OBJECT (adj), "name");
  gdouble oscalex = sp->scale.x, oscaley = sp->scale.y;
  GtkAdjustment *adj_other;
  // step and eps are in the space of the adjustment values;
  // exp_eps is in the space of the scaling values.
  gdouble expvalue = pow (10., adj->value), step = 0.0; // exp10
  gdouble eps = .0001, exp_eps = .001;

  /* this unappealing case arises when cpanel_scale_set is resetting
     adjustment values, which calls the associated callbacks. */
  if (display != sp->displayptr)
    return;

  if (strcmp (name, "SCALE:x_zoom_adj") == 0) {
    cpanel->scale.zoomval.x = adj->value;
    step = adj->value - log10 (sp->scale.x);
    sp->scale.x = expvalue;
    if (cpanel->scale.fixAspect_p && fabs (step) > eps) {
      adj_other = scale_adjustment_find_by_name ("SCALE:y_zoom", gg);
      sp->scale.y = pow (10., adj_other->value + step);
      increment_adjustment (adj_other, step, eps);
    }
  }
  else {
    cpanel->scale.zoomval.y = adj->value;
    step = adj->value - log10 (sp->scale.y);
    sp->scale.y = expvalue;
    if (cpanel->scale.fixAspect_p && fabs (step) > eps) {
      adj_other = scale_adjustment_find_by_name ("SCALE:x_zoom", gg);
      sp->scale.x = pow (10.0, adj_other->value + step);
      increment_adjustment (adj_other, step, eps);
    }
  }

  if (fabs (oscalex - sp->scale.x) > exp_eps ||
      fabs (oscaley - sp->scale.y) > exp_eps) {
    splot_plane_to_screen (display, cpanel, sp, gg);
    ruler_ranges_set (false, display, sp, gg);
    splot_redraw (sp, FULL, gg);
  }
}

static void
pan_cb (GtkAdjustment * adj, ggobid * gg)
{
  displayd *display = gg->current_display;
  splotd *sp = gg->current_splot;
  cpaneld *cpanel = &display->cpanel;
  gchar *name = (gchar *) g_object_get_data (G_OBJECT (adj), "name");

  /* this unappealing case arises when cpanel_scale_set is resetting
     adjustment values, which calls the associated callbacks. */
  if (display != sp->displayptr)
    return;

  if (strcmp (name, "SCALE:x_pan_adj") == 0) {
    cpanel->scale.panval.x = adj->value;
    sp->pmid.x = -1 * adj->value;
  }
  else {
    cpanel->scale.panval.y = adj->value;
    sp->pmid.y = -1 * adj->value;
  }

  splot_plane_to_screen (display, cpanel, sp, gg);
  ruler_ranges_set (false, display, sp, gg);
  splot_redraw (sp, FULL, gg);
}

void
scale_pan_reset (displayd * display)
{
  ggobid *gg = display->ggobi;
  splotd *sp = gg->current_splot;
  GtkAdjustment *adj;
  gdouble value = 0.0;

  adj = scale_adjustment_find_by_name ("SCALE:y_pan", gg);
  gtk_adjustment_set_value (adj, value);
  adj = scale_adjustment_find_by_name ("SCALE:x_pan", gg);
  gtk_adjustment_set_value (adj, value);

  /* If we've been scaling using the scale widgets, the following is
     redundant, but we need it if we've been scaling by direct
     manipulation */

  sp->pmid.x = sp->pmid.y = 0;

  splot_plane_to_screen (display, &display->cpanel, sp, gg);
  ruler_ranges_set (false, gg->current_display, sp, gg);
  splot_redraw (sp, FULL, gg);
}

void
scale_zoom_reset (displayd * dsp)
{
  ggobid *gg = dsp->ggobi;
  splotd *sp = gg->current_splot;
  GtkAdjustment *adj;
  gdouble value = log10 (SCALE_DEFAULT);

  adj = scale_adjustment_find_by_name ("SCALE:y_zoom", gg);
  gtk_adjustment_set_value (adj, value);
  adj = scale_adjustment_find_by_name ("SCALE:x_zoom", gg);
  gtk_adjustment_set_value (adj, value);

  /* If we've been scaling using the scale widgets, the following is
     redundant, but we need it if we've been scaling by direct
     manipulation */

  sp->scale.x = sp->scale.y = SCALE_DEFAULT;

  splot_plane_to_screen (dsp, &dsp->cpanel, sp, gg);
  ruler_ranges_set (false, dsp, sp, gg);
  splot_redraw (sp, FULL, gg);
}

/*--------------------------------------------------------------------*/
/*           Resetting various state variables                        */
/*--------------------------------------------------------------------*/

static void
aspect_ratio_cb (GtkToggleButton * button, ggobid * gg)
{
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;

  cpanel->scale.fixAspect_p = button->active;
}

/*--------------------------------------------------------------------*/
/*      Handling keyboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

/*
If ctrl-alt-p is pressed, toggle on and off click-style panning.
If ctrl-alt-z is pressed, toggle on and off click-style zooming.
*/

static gint
key_press_cb (GtkWidget * w, GdkEventKey * event, splotd * sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  cpaneld *cpanel = &gg->current_display->cpanel;

/*-- add a key_press_cb in each mode, and let it begin with these lines --*/
  if (splot_event_handled (w, event, cpanel, sp, gg))
    return true;

  return false;
}

static gint
motion_notify_cb (GtkWidget * w, GdkEventMotion * event, splotd * sp)
{
  gboolean button1_p, button2_p;
  ggobid *gg = GGobiFromSPlot (sp);
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;

  /*-- get the mouse position and find out which buttons are pressed --*/
  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);

  /*-- if neither button is pressed, we shouldn't have gotten the event --*/
  if (!button1_p && !button2_p)
    return false;

  /*-- I'm not sure this could ever happen --*/
  if (sp->mousepos.x == sp->mousepos_o.x
      && sp->mousepos.y == sp->mousepos_o.y)
    return false;

  if (button1_p) {
    pan_by_drag (sp, gg);
  }
  else if (button2_p) {
    zoom_by_drag (sp, gg);
  }

  if (cpanel->scale.updateAlways_p) {
    /*-- redisplay this plot --*/
    splot_plane_to_screen (display, &display->cpanel, sp, gg);
    ruler_ranges_set (false, gg->current_display, sp, gg);
    splot_redraw (sp, FULL, gg);
  }
  else {
    splot_redraw (sp, QUICK, gg);
  }

  sp->mousepos_o.x = sp->mousepos.x;
  sp->mousepos_o.y = sp->mousepos.y;

  return true;
}

static gint
scroll_cb (GtkWidget *w, GdkEventScroll *event, splotd *sp)
{
  /* For now, make this a fixed-ratio zoom. Most people don't have horizontal
     mouse wheels. */
  gdouble factor = 1.0, xscale = sp->scale.x, yscale = sp->scale.y;
  if (event->direction == GDK_SCROLL_UP)
    factor += SCALE_SCROLL_INC;
  else if (event->direction == GDK_SCROLL_DOWN)
    factor -= SCALE_SCROLL_INC;
  xscale *= factor;
  yscale *= factor;
  splot_zoom(sp, xscale, yscale);
  return(true);
}

static gint
button_press_cb (GtkWidget * w, GdkEventButton * event, splotd * sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot (sp);
  gboolean button1_p, button2_p;
  mousepos_get_pressed (w, event, &button1_p, &button2_p, sp);

  gg->current_splot = sp->displayptr->current_splot = sp;
  gg->current_display = (displayd *) sp->displayptr;

  sp->mousepos_o.x = sp->mousepos.x;
  sp->mousepos_o.y = sp->mousepos.y;

  sp->mousedownpos.x = sp->mousepos.x;
  sp->mousedownpos.y = sp->mousepos.y;

  sp->motion_id = g_signal_connect (G_OBJECT (sp->da),
                                    "motion_notify_event",
                                    G_CALLBACK (motion_notify_cb),
                                    (gpointer) sp);
  return retval;
}

static gint
button_release_cb (GtkWidget * w, GdkEventButton * event, splotd * sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot (sp);
  GdkModifierType state;
  displayd *dsp = sp->displayptr;
  cpaneld *cpanel = &dsp->cpanel;

  gg->buttondown = 0;

  gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y,
                          &state);

  gdk_pointer_ungrab (event->time);
  disconnect_motion_signal (sp);

  if (!cpanel->scale.updateAlways_p) {
    displayd *display = sp->displayptr;
    /*-- redisplay this plot --*/
    splot_plane_to_screen (display, &display->cpanel, sp, gg);
    ruler_ranges_set (false, gg->current_display, sp, gg);
    splot_redraw (sp, FULL, gg);
  }
  else {
    splot_redraw (sp, QUICK, gg);
  }

  return retval;
}

void
scale_event_handlers_toggle (splotd * sp, gboolean state)
{
  displayd *display = (displayd *) sp->displayptr;

  if (state == on) {
    if (GGOBI_IS_WINDOW_DISPLAY (display)
        && GGOBI_WINDOW_DISPLAY (display)->useWindow)
      sp->key_press_id =
        g_signal_connect (G_OBJECT (GGOBI_WINDOW_DISPLAY (display)->window),
                          "key_press_event", G_CALLBACK (key_press_cb),
                          (gpointer) sp);


    sp->press_id = g_signal_connect (G_OBJECT (sp->da),
                                     "button_press_event",
                                     G_CALLBACK (button_press_cb),
                                     (gpointer) sp);
    sp->release_id = g_signal_connect (G_OBJECT (sp->da),
                                       "button_release_event",
                                       G_CALLBACK (button_release_cb),
                                       (gpointer) sp);
    sp->scroll_id = g_signal_connect (G_OBJECT (sp->da),
                                       "scroll_event",
                                       G_CALLBACK (scroll_cb),
                                       (gpointer) sp);
  }
  else {
    disconnect_key_press_signal (sp);
    disconnect_button_press_signal (sp);
    disconnect_button_release_signal (sp);
    disconnect_scroll_signal (sp);
  }
}

void
cpanel_scale_make (ggobid * gg)
{
  modepaneld *panel;
  GtkWidget *f, *vb;
  GtkAdjustment *adjx, *adjy;
  GtkWidget *sbarx, *sbary;
  GtkWidget *tgl;

  panel = (modepaneld *) g_malloc (sizeof (modepaneld));
  gg->control_panels = g_list_append (gg->control_panels, (gpointer) panel);
  panel->name = g_strdup (GGOBI (getIModeName) (SCALE));
  panel->w = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (panel->w), 5);

  /*-- frame and vbox for zoom controls --*/
  f = gtk_frame_new ("Zoom");
  gtk_box_pack_start (GTK_BOX (panel->w), f, false, false, 0);

  vb = gtk_vbox_new (true, 0);
  gtk_container_add (GTK_CONTAINER (f), vb);

  /* value, lower, upper, step_increment, page_increment, page_size */
  adjx = (GtkAdjustment *)
    gtk_adjustment_new (log10 (SCALE_DEFAULT), -1.0, 1.0, 0.05, 0.05, 0.0);
  g_object_set_data (G_OBJECT (adjx), "name", "SCALE:x_zoom_adj");
  g_signal_connect (G_OBJECT (adjx), "value_changed",
                    G_CALLBACK (zoom_cb), gg);
  sbarx = gtk_hscale_new (GTK_ADJUSTMENT (adjx));
  gtk_widget_set_name (sbarx, "SCALE:x_zoom");
  scale_set_default_values (GTK_SCALE (sbarx));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbarx,
                        "Zoom horizontally", NULL);
  gtk_box_pack_start (GTK_BOX (vb), sbarx, true, true, 0);

  adjy = (GtkAdjustment *)
    gtk_adjustment_new (log10 (SCALE_DEFAULT), -1.0, 1.0, 0.05, 0.05, 0.0);
  g_object_set_data (G_OBJECT (adjy), "name", "SCALE:y_zoom_adj");
  g_signal_connect (G_OBJECT (adjy), "value_changed",
                    G_CALLBACK (zoom_cb), gg);
  sbary = gtk_hscale_new (GTK_ADJUSTMENT (adjy));
  gtk_widget_set_name (sbary, "SCALE:y_zoom");
  scale_set_default_values (GTK_SCALE (sbary));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbary,
                        "Zoom vertically", NULL);
  gtk_box_pack_start (GTK_BOX (vb), sbary, true, true, 0);

  tgl = gtk_check_button_new_with_mnemonic ("Fixed _aspect");
  gtk_widget_set_name (tgl, "SCALE:aspect_ratio_tgl");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
                        "Fix the aspect ratio while zooming.", NULL);
  g_signal_connect (G_OBJECT (tgl), "toggled",
                    G_CALLBACK (aspect_ratio_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (vb), tgl, false, false, 3);

  /*-- frame and vbox for pan controls --*/
  f = gtk_frame_new ("Pan");
  gtk_box_pack_start (GTK_BOX (panel->w), f, false, false, 0);

  vb = gtk_vbox_new (true, 0);
  gtk_container_add (GTK_CONTAINER (f), vb);

  /* value, lower, upper, step_increment, page_increment, page_size */
  adjx = (GtkAdjustment *)
    gtk_adjustment_new (0.0, -2 * PRECISION1, 2 * PRECISION1, 200, 400, 0.0);
  g_object_set_data (G_OBJECT (adjx), "name", "SCALE:x_pan_adj");
  g_signal_connect (G_OBJECT (adjx), "value_changed",
                    G_CALLBACK (pan_cb), gg);
  sbarx = gtk_hscale_new (GTK_ADJUSTMENT (adjx));
  gtk_widget_set_name (sbarx, "SCALE:x_pan");
  scale_set_default_values (GTK_SCALE (sbarx));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbarx,
                        "Pan horizontally", NULL);
  gtk_box_pack_start (GTK_BOX (vb), sbarx, true, true, 0);

  adjy = (GtkAdjustment *)
    gtk_adjustment_new (0.0, -2 * PRECISION1, 2 * PRECISION1, 200, 400, 0.0);
  g_object_set_data (G_OBJECT (adjy), "name", "SCALE:y_pan_adj");
  g_signal_connect (G_OBJECT (adjy), "value_changed",
                    G_CALLBACK (pan_cb), gg);
  sbary = gtk_hscale_new (GTK_ADJUSTMENT (adjy));
  gtk_widget_set_name (sbary, "SCALE:y_pan");
  scale_set_default_values (GTK_SCALE (sbary));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbary,
                        "Pan vertically", NULL);
  gtk_box_pack_start (GTK_BOX (vb), sbary, true, true, 0);


  gtk_widget_show_all (panel->w);
}

/*--------------------------------------------------------------------*/
/*                      Control panel updating                        */
/*--------------------------------------------------------------------*/

void
cpanel_scale_init (cpaneld * cpanel, ggobid * gg)
{

  cpanel->scale.updateAlways_p = true;
  cpanel->scale.fixAspect_p = false;

  cpanel->scale.zoomval.x = log10 (SCALE_DEFAULT);
  cpanel->scale.zoomval.y = log10 (SCALE_DEFAULT);
  cpanel->scale.panval.x = 0.0;
  cpanel->scale.panval.y = 0.0;
}

void
cpanel_scale_set (displayd * display, cpaneld * cpanel, ggobid * gg)
{
  GtkWidget *w;
  GtkWidget *panel =
    mode_panel_get_by_name (GGOBI (getIModeName) (SCALE), gg);
  GtkAdjustment *adj;

  if (panel == (GtkWidget *) NULL)
    return;

  w = widget_find_by_name (panel, "SCALE:aspect_ratio_tgl");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w),
                                cpanel->scale.fixAspect_p);

  /*  One by one, find the adjustments and reset them */
  adj = scale_adjustment_find_by_name ("SCALE:x_zoom", gg);
  gtk_adjustment_set_value (adj, cpanel->scale.zoomval.x);
  adj = scale_adjustment_find_by_name ("SCALE:y_zoom", gg);
  gtk_adjustment_set_value (adj, cpanel->scale.zoomval.y);
  adj = scale_adjustment_find_by_name ("SCALE:x_pan", gg);
  gtk_adjustment_set_value (adj, cpanel->scale.panval.x);
  adj = scale_adjustment_find_by_name ("SCALE:y_pan", gg);
  gtk_adjustment_set_value (adj, cpanel->scale.panval.y);
}
