/* tour2d3_ui.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include "vars.h"
#include "externs.h"

/* This function initializes the rotation variables - it should only be
   called once, when a new tour is started since a new
   subset of variable might be used, or when there is new data. */


void 
cpanel_t2d3_init (cpaneld *cpanel, ggobid *gg) {
  cpanel->t2d3.step = TOURSTEP0;
  cpanel->t2d3.paused = false;
  cpanel->t2d3.slidepos = 50.;/* If this is changed, it needs to be 
     changed in th cpanel_tour2d3_make routine also. */
  cpanel->t2d3.manip_mode = MANIP_OBLIQUE;
}

void
cpanel_tour2d3_set (cpaneld *cpanel, ggobid* gg)
/*
 * To handle the case where there are multiple scatterplots
 * which may have different tour options and parameters selected
*/
{
  GtkWidget *w, *btn;
  GtkWidget *pnl = gg->control_panel[TOUR2D3];
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
    gtk_option_menu_set_history (GTK_OPTION_MENU (w), cpanel->t2d3.manip_mode);
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
manip_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  displayd *dsp = gg->current_display;
  cpaneld *cpanel = &dsp->cpanel;
  splotd *sp = gg->current_splot;

  cpanel->t2d3.manip_mode = GPOINTER_TO_INT (cbd);

  if (cpanel->t2d3.manip_mode == MANIP_OFF)
    splot_cursor_set ((gint) NULL, sp);
  else
    splot_cursor_set (GDK_HAND2, sp);
}

void
cpanel_tour2d3_make (ggobid *gg) {
  GtkWidget *box, *btn, *sbar, *lbl, *vb;
  GtkObject *adj;
  GtkWidget *manip_opt;
  
  gg->control_panel[TOUR2D3] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[TOUR2D3]),
    5);

/*
 * speed scrollbar
*/
  /* value, lower, upper, step_increment, page_increment, page_size */
  /* Note that the page_size value only makes a difference for
   * scrollbar widgets, and the highest value you'll get is actually
   * (upper - page_size). */
  adj = gtk_adjustment_new (50.0, 0.0, 100.0, 1.0, 1.0, 0.0);

  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                      GTK_SIGNAL_FUNC (speed2d3_set_cb), (gpointer) gg);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_name (sbar, "TOUR2D3:speed_bar");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
    "Adjust speed of tour motion", NULL);
  scale_set_default_values (GTK_SCALE (sbar));

  gtk_box_pack_start (GTK_BOX (gg->control_panel[TOUR2D3]), sbar,
    false, false, 1);

/*
 * Box to hold 'pause' toggle button
*/
  box = gtk_hbox_new (true, 1);

  btn = gtk_check_button_new_with_label ("Pause");
  gtk_widget_set_name (btn, "TOUR2D3:pause_button");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Stop tour motion temporarily (keyboard shortcut: w)", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "toggled",
                     GTK_SIGNAL_FUNC (tour2d3_pause_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  gtk_box_pack_start (GTK_BOX (gg->control_panel[TOUR2D3]), box,
    false, false, 1);

/*
 * Box to hold 'Reinit' toggle and 'Scramble' button
*/
  box = gtk_hbox_new (true, 2);

  btn = gtk_button_new_with_label ("Reinit");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Reset projection to first two active variables", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                     GTK_SIGNAL_FUNC (reinit_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  btn = gtk_button_new_with_label ("Scramble");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Reset projection to random value", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                     GTK_SIGNAL_FUNC (scramble_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  gtk_box_pack_start (GTK_BOX (gg->control_panel[TOUR2D3]), box,
    false, false, 1);

/*
 * manipulation option menu and label inside vbox
*/

  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[TOUR2D3]), vb,
    false, false, 0);

  lbl = gtk_label_new ("Manual manipulation:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  manip_opt = gtk_option_menu_new ();
  gtk_widget_set_name (manip_opt, "TOUR2D3:manip");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), manip_opt,
    "Set the manual manipulation method", NULL);
  gtk_box_pack_end (GTK_BOX (vb), manip_opt, false, false, 0);
  populate_option_menu (manip_opt, manip_lbl,
    sizeof (manip_lbl) / sizeof (gchar *),
    (GtkSignalFunc) manip_cb, "GGobi", gg);

  gtk_widget_show_all (gg->control_panel[TOUR2D3]);
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
    GtkWidget *pause_button = NULL;
    pause_button = widget_find_by_name (gg->control_panel[TOUR2D3],
      "TOUR2D3:pause_button");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pause_button),
      !cpanel->t2d3.paused);
  }

  return true;
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
    sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                      "motion_notify_event",
                                      (GtkSignalFunc) motion_notify_cb,
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
    if(GTK_IS_GGOBI_WINDOW_DISPLAY(display))
      sp->key_press_id = gtk_signal_connect (GTK_OBJECT (GTK_GGOBI_WINDOW_DISPLAY(display)->window),
        "key_press_event",
        (GtkSignalFunc) key_press_cb,
        (gpointer) sp);
    sp->press_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                       "button_press_event",
                                       (GtkSignalFunc) button_press_cb,
                                       (gpointer) sp);
    sp->release_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                         "button_release_event",
                                         (GtkSignalFunc) button_release_cb,
                                         (gpointer) sp);
  } else {
    disconnect_key_press_signal (sp);
    disconnect_button_press_signal (sp);
    disconnect_button_release_signal (sp);
  }
}
