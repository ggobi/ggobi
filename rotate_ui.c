/*-- rotate_ui.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
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
static void type_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);

  if (gg->current_display != NULL) {
    cpaneld *cpanel = &gg->current_display->cpanel;
    gint indx = GPOINTER_TO_INT (cbd);

    cpanel->ro_type = indx;

    g_printerr ("cbd: %s\n", type_lbl[indx]);
  }
}

static gchar *axis_lbl[] = {"Y Axis", "X Axis", "Oblique Axis"};
static void axis_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  if (gg->current_display != NULL) {
    cpaneld *cpanel = &gg->current_display->cpanel;
    gint indx = GPOINTER_TO_INT (cbd);

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

  sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                      "motion_notify_event",
                                      (GtkSignalFunc) motion_notify_cb,
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
    sp->press_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                       "button_press_event",
                                       (GtkSignalFunc) button_press_cb,
                                       (gpointer) sp);
    sp->release_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                         "button_release_event",
                                         (GtkSignalFunc) button_release_cb,
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
  GtkWidget *btn, *sbar, *box;
  GtkObject *adj;
  
  gg->control_panel[ROTATE] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[ROTATE]), 5);

/*
 * speed scrollbar
*/
  /* value, lower, upper, step_increment, page_increment, page_size */
  /* Note that the page_size value only makes a difference for
   * scrollbar widgets, and the highest value you'll get is actually
   * (upper - page_size). */
  adj = gtk_adjustment_new (1.0, 0.0, 100.0, 1.0, 1.0, 0.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                      GTK_SIGNAL_FUNC (rotation_speed_cb), NULL);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
    "Adjust speed of rotation", NULL);
  scale_set_default_values (GTK_SCALE (sbar));

  gtk_box_pack_start (GTK_BOX (gg->control_panel[ROTATE]), sbar,
    false, false, 1);

/*
 * Box to hold 'pause' toggle and 'reinit' button
*/
  box = gtk_hbox_new (true, 2);

  ro_paused_btn = gtk_check_button_new_with_label ("Pause");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), ro_paused_btn,
    "Stop rotation temporarily", NULL);
  gtk_signal_connect (GTK_OBJECT (ro_paused_btn), "toggled",
                     GTK_SIGNAL_FUNC (rotate_pause_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (box), ro_paused_btn, true, true, 1);

  btn = gtk_button_new_with_label ("Reinit");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Reset projection", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                     GTK_SIGNAL_FUNC (reinit_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  gtk_box_pack_start (GTK_BOX (gg->control_panel[ROTATE]), box, false, false, 1);

/*
 * Button to change direction
*/
  btn = gtk_button_new_with_label ("Change direction");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Change direction of rotation", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[ROTATE]), btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (chdir_cb), gg);

/*
 * option menu: rotate/rock/interpolate
*/
  ro_type_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), ro_type_opt,
    "Rotate freely, rock locally, or interpolate between two orthogonal projections",
    NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[ROTATE]),
                      ro_type_opt, false, false, 0);
  populate_option_menu (ro_type_opt, type_lbl,
                        sizeof (type_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) type_cb, gg);

/*
 * option menu: y/x/oblique axis
*/
  ro_axis_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), ro_axis_opt,
    "Choose axis of rotation",   NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[ROTATE]),
                      ro_axis_opt, false, false, 0);
  populate_option_menu (ro_axis_opt, axis_lbl,
                        sizeof (axis_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) axis_cb, gg);
  gtk_widget_show_all (gg->control_panel[ROTATE]);
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

  gtk_option_menu_set_history (GTK_OPTION_MENU (ro_type_opt),
                               cpanel->ro_type);
  gtk_option_menu_set_history (GTK_OPTION_MENU (ro_axis_opt),
                               cpanel->ro_axis);

  GTK_TOGGLE_BUTTON (ro_paused_btn)->active = cpanel->ro_paused_p;
}
#endif
