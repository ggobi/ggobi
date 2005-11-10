/*-- rotate_ui.c --*/
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
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include "vars.h"
#include "externs.h"

#ifdef ROTATION_IMPLEMENTED

static GtkWidget *ro_paused_btn, *ro_type_opt, *ro_axis_opt;

static void rotation_speed_cb (GtkAdjustment *adj, gpointer cbd) {
  g_printerr ("%d\n", ((gint) adj->value));
}
static void scale_set_default_values (GtkScale *scale )
{
  gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_draw_value (scale, false);
}

static void chdir_cb (GtkButton *button, ggobid *gg)
{
  if (gg->current_display != NULL) {
    cpaneld *cpanel = &gg->current_display->cpanel;

    cpanel->ro_direction = -1 * cpanel->ro_direction;
  }
}

static void rotate_pause_cb (GtkToggleButton *button)
{
  g_printerr ("rotate pause: %d\n", button->active);
}
static void reinit_cb (GtkWidget *w) {
  g_printerr ("reinit\n");
}

static gchar *type_lbl[] = {"Rotate", "Rock", "Interpolate"};
static void type_cb (GtkWidget *w, ggobid *gg)
{

  if (gg->current_display != NULL) {
    cpaneld *cpanel = &gg->current_display->cpanel;
    gint indx = gtk_combo_box_get_active(GTK_COMBO_BOX(w));

    cpanel->ro_type = indx;

    g_printerr ("cbd: %s\n", type_lbl[indx]);
  }
}

static gchar *axis_lbl[] = {"Y Axis", "X Axis", "Oblique Axis"};
static void axis_cb (GtkWidget *w, ggobid *gg)
{
  if (gg->current_display != NULL) {
    cpaneld *cpanel = &gg->current_display->cpanel;
    gint indx = gtk_combo_box_get_active(GTK_COMBO_BOX(w));

    cpanel->ro_axis = indx;

    g_printerr ("cbd: %s\n", axis_lbl[indx]);
  }
}

/*--------------------------------------------------------------------*/
/*                        I/O events                                  */
/*--------------------------------------------------------------------*/

/*
static void rotation_io_cb (GtkWidget *w, gpointer *cbd) {
  gchar *lbl = (gchar *) cbd;
  g_printerr ("cbd: %s\n", lbl);
}
*/

/*--------------------------------------------------------------------*/
/*          Handling mouse events in the plot window                  */
/*--------------------------------------------------------------------*/

static gint
motion_notify_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  g_printerr ("(ro_motion_notify_cb)\n");

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{

  g_printerr ("rotate button_press: %d\n", event->button);

  sp->mousepos.x = (gint) event->x;
  sp->mousepos.y = (gint) event->y;

  sp->motion_id = g_signal_connect (G_OBJECT (sp->da),
                                      "motion_notify_event",
                                      G_CALLBACK(motion_notify_cb),
                                      (gpointer) sp);
  return true;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;

  sp->mousepos.x = (gint) event->x;
  sp->mousepos.y = (gint) event->y;

  disconnect_motion_signal (sp);

  return retval;
}

void
rotation_event_handlers_toggle (splotd *sp, gboolean state)
{
  if (state == on) {
    sp->press_id = g_signal_connect (G_OBJECT (sp->da),
                                       "button_press_event",
                                       G_CALLBACK(button_press_cb),
                                       (gpointer) sp);
    sp->release_id = g_signal_connect (G_OBJECT (sp->da),
                                         "button_release_event",
                                         G_CALLBACK(button_release_cb),
                                         (gpointer) sp);
  } else {
    disconnect_button_press_signal (sp);
    disconnect_button_release_signal (sp);
  }
}

/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/

void
rotation_menus_make (ggobid *gg) {
}

/*--------------------------------------------------------------------*/
/*                   End of main menubar section                      */
/*--------------------------------------------------------------------*/

void
cpanel_rotation_make (ggobid *gg) {
  modepaneld *panel;
  GtkWidget *btn, *sbar, *box;
  GtkObject *adj;
  
  panel = (modepaneld *) g_malloc(sizeof(modepaneld));
  g_list_append(gg->control_panels, (gpointer) panel);
  panel->name = g_strdup("Rotation");
  panel->w = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (panel->w), 5);

/*
 * speed scrollbar
*/
  /* value, lower, upper, step_increment, page_increment, page_size */
  /* Note that the page_size value only makes a difference for
   * scrollbar widgets, and the highest value you'll get is actually
   * (upper - page_size). */
  adj = gtk_adjustment_new (1.0, 0.0, MAX_TOUR_SPEED, 1.0, 1.0, 0.0);
  g_signal_connect (G_OBJECT (adj), "value_changed",
                      G_CALLBACK (rotation_speed_cb), NULL);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
    "Adjust speed of rotation", NULL);
  scale_set_default_values (GTK_SCALE (sbar));

  gtk_box_pack_start (GTK_BOX (panel->w), sbar,
    false, false, 1);

/*
 * Box to hold 'pause' toggle and 'reinit' button
*/
  box = gtk_hbox_new (true, 2);

  ro_paused_btn = gtk_check_button_new_with_mnemonic ("_Pause");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), ro_paused_btn,
    "Stop rotation temporarily", NULL);
  g_signal_connect (G_OBJECT (ro_paused_btn), "toggled",
                     G_CALLBACK (rotate_pause_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (box), ro_paused_btn, true, true, 1);

  btn = gtk_button_new_with_mnemonic ("_Reinit");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Reset projection", NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
                     G_CALLBACK (reinit_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  gtk_box_pack_start (GTK_BOX (panel->w), box, false, false, 1);

/*
 * Button to change direction
*/
  btn = gtk_button_new_with_mnemonic ("Change di_rection");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Change direction of rotation", NULL);
  gtk_box_pack_start (GTK_BOX (panel->w), btn, false, false, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (chdir_cb), gg);

/*
 * option menu: rotate/rock/interpolate
*/
  ro_type_opt = gtk_combo_box_new_text ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), ro_type_opt,
    "Rotate freely, rock locally, or interpolate between two orthogonal projections",
    NULL);
  gtk_box_pack_start (GTK_BOX (panel->w),
                      ro_type_opt, false, false, 0);
  populate_combo_box (ro_type_opt, type_lbl, G_N_ELEMENTS(type_lbl),
                        G_CALLBACK(type_cb), gg);

/*
 * option menu: y/x/oblique axis
*/
  ro_axis_opt = gtk_combo_box_new_text ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), ro_axis_opt,
    "Choose axis of rotation",   NULL);
  gtk_box_pack_start (GTK_BOX (panel->w),
                      ro_axis_opt, false, false, 0);
  populate_combo_box (ro_axis_opt, axis_lbl, G_N_ELEMENTS(axis_lbl),
                        G_CALLBACK(axis_cb), gg);
  gtk_widget_show_all (panel->w);
}

/*--------------------------------------------------------------------*/
/*                      Control panel section                         */
/*--------------------------------------------------------------------*/

void
cpanel_rotation_init (cpaneld *cpanel, ggobid *gg) {
 cpanel->ro_paused_p = false;
 cpanel->ro_axis = RO_OBLIQUE;
 cpanel->ro_type = RO_ROTATE;
 cpanel->ro_direction = FORWARD;
}

void
cpanel_rotation_set (cpaneld *cpanel, ggobid *gg) {

  gtk_combo_box_set_active (GTK_COMBO_BOX (ro_type_opt),
                               cpanel->ro_type);
  gtk_combo_box_set_active (GTK_COMBO_BOX (ro_axis_opt),
                               cpanel->ro_axis);

  GTK_TOGGLE_BUTTON (ro_paused_btn)->active = cpanel->ro_paused_p;
}
#endif
