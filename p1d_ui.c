/*-- p1d_ui.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static gchar *type_lbl[] = {"Texturing", "ASH"};

static void type_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->p1d_type = GPOINTER_TO_INT (cbd);

  display_tailpipe (gg->current_display, gg);
}

static void ash_smoothness_cb (GtkAdjustment *adj, ggobid *gg) 
{
  cpaneld *cpanel = &gg->current_display->cpanel;

  /*-- adj->value ranges from .01 to .5; min value for nASHes = 1 --*/
  cpanel->nASHes = (gint) ((gfloat) cpanel->nbins * (adj->value / 2.0));

  display_tailpipe (gg->current_display, gg);
}

static void cycle_cb (GtkToggleButton *button, ggobid* gg)
{
  gg->p1d.cycle_p = button->active;
}
static void cycle_speed_cb (GtkAdjustment *adj, gpointer cbd) {
  g_printerr ("%d\n", ((gint) adj->value));
}

static gint direction = FORWARD;
static void chdir_cb (GtkButton *button)
{
  direction = -1 * direction;
}

static void scale_set_default_values (GtkScale *scale)
{
  gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_draw_value (scale, false);
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
  if (scatterplot_event_handled (w, event, cpanel, sp, gg))
    return true;

  /*-- insert mode-specific key presses (if any) here --*/

  return true;
}

void
p1d_event_handlers_toggle (splotd *sp, gboolean state) {
  displayd *display = (displayd *) sp->displayptr;

  if (state == on) {
    sp->key_press_id = gtk_signal_connect (GTK_OBJECT (display->window),
                                           "key_press_event",
                                           (GtkSignalFunc) key_press_cb,
                                           (gpointer) sp);
  } else {
    if (sp->key_press_id)
      gtk_signal_disconnect (GTK_OBJECT (display->window), sp->key_press_id);
  }
}

/*--------------------------------------------------------------------*/

void
cpanel_p1dplot_make (ggobid *gg) {
  GtkWidget *tgl, *btn, *vb;
  GtkWidget *sbar;
  
  gg->control_panel[P1PLOT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[P1PLOT]), 5);

/*
 * option menu
*/
  gg->ash.type_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->ash.type_opt,
    "Display either textured dot plots or average shifted histograms", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[P1PLOT]),
                      gg->ash.type_opt, false, false, 0);
  populate_option_menu (gg->ash.type_opt, type_lbl,
                        sizeof (type_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) type_cb, gg);
/*
 * ASH smoothness
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[P1PLOT]), vb,
    false, false, 0);

  gtk_box_pack_start (GTK_BOX (vb), gtk_label_new ("ASH smoothness:"),
    false, false, 0);

  /*-- value, lower, upper, step --*/
  gg->ash.smoothness_adj = gtk_adjustment_new (0.19, 0.02, 0.5, 0.01, .01, 0.0);
  gtk_signal_connect (GTK_OBJECT (gg->ash.smoothness_adj), "value_changed",
                      GTK_SIGNAL_FUNC (ash_smoothness_cb), gg);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (gg->ash.smoothness_adj));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
    "Adjust ASH smoothness", NULL);
  gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
  gtk_scale_set_digits (GTK_SCALE (sbar), 2);

  gtk_box_pack_start (GTK_BOX (vb), sbar,
    false, false, 1);
/*
 * Cycling controls
*/

  tgl = gtk_check_button_new_with_label ("Cycle");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
                        "Cycle through 1D plots", NULL);
  gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                      GTK_SIGNAL_FUNC (cycle_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[P1PLOT]),
                      tgl, false, false, 1);
  gtk_widget_set_sensitive (tgl, false);

  /* value, lower, upper, step_increment, page_increment, page_size */
  /* Note that the page_size value only makes a difference for
   * scrollbar widgets, and the highest value you'll get is actually
   * (upper - page_size). */
  gg->p1d.cycle_speed_adj = gtk_adjustment_new (1.0, 0.0, 100.0, 1.0, 1.0, 0.0);
  gtk_signal_connect (GTK_OBJECT (gg->p1d.cycle_speed_adj), "value_changed",
                      GTK_SIGNAL_FUNC (cycle_speed_cb), gg);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (gg->p1d.cycle_speed_adj));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
    "Adjust cycling speed", NULL);
  scale_set_default_values (GTK_SCALE (sbar));
  gtk_box_pack_start (GTK_BOX (gg->control_panel[P1PLOT]), sbar,
    false, false, 1);
  gtk_widget_set_sensitive (sbar, false);

  btn = gtk_button_new_with_label ("Change direction");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Change cycling direction", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[P1PLOT]),
                      btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (chdir_cb), gg);
  gtk_widget_set_sensitive (btn, false);

  gtk_widget_show_all (gg->control_panel[P1PLOT]);
}


/*-- for all plot modes, for now: it excludes the changing variable --*/
void
cpanel_p1d_init (cpaneld *cpanel, ggobid *gg) {
  cpanel->nASHes = 20;
  cpanel->nbins = 200;
}

/*-- scatterplot only; need a different routine for parcoords --*/
void
cpanel_p1d_set (cpaneld *cpanel, ggobid* gg)
/*
 * To handle the case where there are multiple scatterplots
 * which may have different p1d options and parameters selected
*/
{
  /*-- Texturing or ASH --*/
  gtk_option_menu_set_history (GTK_OPTION_MENU (gg->ash.type_opt),
                               cpanel->p1d_type);

  /*-- ASH smoothness parameter --*/
  gtk_adjustment_set_value (GTK_ADJUSTMENT (gg->ash.smoothness_adj),
    2 * (gfloat) cpanel->nASHes / (gfloat) cpanel->nbins);

  /*-- Cycling on or off --*/
  /*-- Cycling speed --*/
  /*-- Cycling direction --*/
}

