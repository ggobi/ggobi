/* ctour_ui.c */
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
#include <strings.h>

#include "vars.h"
#include "externs.h"

static void ctouradv_window_open (void);

static void speed_cb (GtkAdjustment *adj, gpointer cbd) {
  g_printerr ("%d\n", ((gint) adj->value));
}
static void scale_set_default_values (GtkScale *scale )
{
  gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_draw_value (scale, false);
}
static void pause_cb (GtkToggleButton *button)
{
  g_printerr ("ctour pause: %d\n", button->active);
}
static void reinit_cb (GtkWidget *w) {
  g_printerr ("reinit\n");
}
static void syncaxes_cb (GtkToggleButton *button)
{
  g_printerr ("syncaxes: %d\n", button->active);
}
static void
hide_cb (GtkWidget *w ) {
  gtk_widget_hide (w);
}

static void ctourpp_cb (GtkWidget *w, ggobid *gg) {
  g_printerr ("open projection pursuit panel\n");
  ctourpp_window_open (gg);
}
static void ctouradv_cb (GtkWidget *w, gpointer dummy) {
  g_printerr ("open advanced correlation tour features panel\n");
  ctouradv_window_open ();
}

static gchar *manip_lbl[] = {"Vertical", "Horizontal", "Comb",
                             "EqualComb"};
static void manip_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", manip_lbl[indx]);
}
static gchar *pathlen_lbl[] = {"1/10", "1/5", "1/4", "1/3", "1/2", "1",
                               "2", "10", "Infinite"};
static void pathlen_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", pathlen_lbl[indx]);
}

void
cpanel_ctour_make (ggobid *gg) {
  GtkWidget *box, *tgl, *btn, *sbar, *vb, *lbl;
  GtkObject *adj;
  GtkWidget *manip_opt, *pathlen_opt;
  
  gg->control_panel[COTOUR] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[COTOUR]), 5);

/*
 * speed scrollbar
*/
  /* value, lower, upper, step_increment, page_increment, page_size */
  /* Note that the page_size value only makes a difference for
   * scrollbar widgets, and the highest value you'll get is actually
   * (upper - page_size). */
  adj = gtk_adjustment_new (1.0, 0.0, 100.0, 1.0, 1.0, 0.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                      GTK_SIGNAL_FUNC (speed_cb), NULL);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
    "Adjust speed of tour motion", NULL);
  scale_set_default_values (GTK_SCALE (sbar));

  gtk_box_pack_start (GTK_BOX (gg->control_panel[COTOUR]), sbar,
    false, false, 1);

/*
 * Box to hold 'pause' toggle and 'reinit' button
*/
  box = gtk_hbox_new (true, 2);

  btn = gtk_check_button_new_with_label ("Pause");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Stop tour motion temporarily", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "toggled",
                     GTK_SIGNAL_FUNC (pause_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  btn = gtk_button_new_with_label ("Reinit");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Reset projection", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                     GTK_SIGNAL_FUNC (reinit_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  gtk_box_pack_start (GTK_BOX (gg->control_panel[COTOUR]), box, false, false, 1);


/*
 * manipulation option menu with label
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[COTOUR]), vb, false, false, 0);

  lbl = gtk_label_new ("Manual manipulation:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  manip_opt = gtk_option_menu_new ();
  gtk_container_set_border_width (GTK_CONTAINER (manip_opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), manip_opt,
    "Set the manual manipulation method", NULL);
  gtk_container_add (GTK_CONTAINER (vb), manip_opt);
  populate_option_menu (manip_opt, manip_lbl,
                        sizeof (manip_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) manip_cb, gg);

/*
 * path length option menu 
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[COTOUR]), vb, false, false, 0);

  lbl = gtk_label_new ("Path length:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  pathlen_opt = gtk_option_menu_new ();
  gtk_container_set_border_width (GTK_CONTAINER (pathlen_opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), pathlen_opt,
    "Set the path length", NULL);
  gtk_container_add (GTK_CONTAINER (vb), pathlen_opt);
  populate_option_menu (pathlen_opt, pathlen_lbl,
                        sizeof (pathlen_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) pathlen_cb, gg);

/*
 * Sync Axes toggle
*/
  tgl = gtk_check_button_new_with_label ("Sync axes");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
    "Synchronize the horizontal and vertical axes", NULL);
  gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                      GTK_SIGNAL_FUNC (syncaxes_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[COTOUR]),
                      tgl, false, false, 1);

/*
 * projection pursuit button
*/
  btn = gtk_button_new_with_label ("Projection pursuit ...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Open panel for correlation tour projection pursuit", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[COTOUR]),
                      btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (ctourpp_cb), gg);

/*
 * advanced features button
*/
  btn = gtk_button_new_with_label ("Advanced features ...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Open panel for additional correlation tour features", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[COTOUR]),
                      btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (ctouradv_cb), NULL);

  gtk_widget_show_all (gg->control_panel[COTOUR]);
}

/*----------------------------------------------------------------------*/
/*               Advanced features panel and callbacks                  */
/*----------------------------------------------------------------------*/

/*

The following are considered advanced features for now:
  history
*/

static GtkWidget *window = NULL;

static void
ctouradv_window_open (void) {
  GtkWidget *vbox, *btn, *frame;

  if (window == NULL) {
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "advanced ctour");
    
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    

/*
 * tour history functions
*/
    frame = gtk_frame_new ("History");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame, false, false, 0);

/*
 * Close button
*/
    btn = gtk_button_new_with_label ("Close");
    gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
                   GTK_SIGNAL_FUNC (hide_cb), (GtkObject*) window);
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, true, 2);
  }

  gtk_widget_show_all (window);
}

/*----------------------------------------------------------------------*/
/*               Handling mouse events in the plot window               */
/*----------------------------------------------------------------------*/

static gint
motion_notify_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  g_printerr ("(ct_motion_notify_cb)\n");

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  gg->current_splot = sp;
  gg->current_display = (displayd *) sp->displayptr;

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

  gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->motion_id);

  return retval;
}

void
ctour_event_handlers_toggle (splotd *sp, gboolean state) {

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
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->press_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->release_id);
  }
}

