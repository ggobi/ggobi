/*-- xyplot_ui.c --*/
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

static const gchar *const fix_axis_lbl[] = {"No fixed axes", "Fix X", "Fix Y"};
static void fix_axis_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  gg->xyplot.cycle_axis = GPOINTER_TO_INT (cbd);
}

static void cycle_cb (GtkToggleButton *button, ggobid *gg)
{
  extern GtkFunction xycycle_func (ggobid *gg);
  gg->xyplot.cycle_p = button->active;

  if (gg->xyplot.cycle_p) {
    gg->xyplot.cycle_id = gtk_timeout_add (gg->xyplot.cycle_delay,
      (GtkFunction) xycycle_func, (gpointer) gg);
    gg->xyplot.cycle_p = true;
  } else {
    gtk_timeout_remove (gg->xyplot.cycle_id);
    gg->xyplot.cycle_id = 0;
    gg->xyplot.cycle_p = false;
  }
}
static void scale_set_default_values (GtkScale *scale)
{
  gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_draw_value (scale, false);
}

static void cycle_speed_cb (GtkAdjustment *adj, ggobid *gg) {
  extern GtkFunction xycycle_func (ggobid *gg);

  gg->xyplot.cycle_delay = -1 * (guint32) adj->value;
  if (gg->xyplot.cycle_p) {
    gtk_timeout_remove (gg->xyplot.cycle_id);
    gg->xyplot.cycle_id = gtk_timeout_add (gg->xyplot.cycle_delay,
    (GtkFunction) xycycle_func, (gpointer) gg);
  }
}

static void chdir_cb (GtkButton *button, ggobid* gg)
{
  gg->xyplot.cycle_dir = -1 * gg->xyplot.cycle_dir;
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

  return true;
}

void
xyplot_event_handlers_toggle (splotd *sp, gboolean state) {
  displayd *display = (displayd *) sp->displayptr;

  if (state == on) {
    sp->key_press_id = gtk_signal_connect (GTK_OBJECT (display->window),
                                           "key_press_event",
                                           (GtkSignalFunc) key_press_cb,
                                           (gpointer) sp);
  } else {
    if (sp->key_press_id) {
      gtk_signal_disconnect (GTK_OBJECT (display->window), sp->key_press_id);
      sp->key_press_id = 0;
    }
  }
}

/*--------------------------------------------------------------------*/

void
cpanel_xyplot_make (ggobid *gg) {
  GtkWidget *cycle_tgl, *chdir_btn, *cycle_sbar, *opt;
  GtkObject *adj;
  
  gg->control_panel[XYPLOT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[XYPLOT]), 5);

  cycle_tgl = gtk_check_button_new_with_label ("Cycle");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), cycle_tgl,
    "Cycle through pairwise plots", NULL);
  gtk_signal_connect (GTK_OBJECT (cycle_tgl), "toggled",
                     GTK_SIGNAL_FUNC (cycle_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[XYPLOT]), cycle_tgl,
    false, false, 3);

/*
 * make an option menu
*/
  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Fix one of the axes during plot cycling or let them both float", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[XYPLOT]), opt,
    false, false, 0);
  populate_option_menu (opt, (gchar**) fix_axis_lbl,
                        sizeof (fix_axis_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) fix_axis_cb, gg);
  
  /* value, lower, upper, step_increment, page_increment, page_size */
  /* Note that the page_size value only makes a difference for
   * scrollbar widgets, and the highest value you'll get is actually
   * (upper - page_size). */
  gg->xyplot.cycle_delay = 1000;
  adj = gtk_adjustment_new (-1.0 * gg->xyplot.cycle_delay,
    -5000.0, -250.0, 100.0, 1000.0, 0.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                      GTK_SIGNAL_FUNC (cycle_speed_cb), gg);

  cycle_sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  scale_set_default_values (GTK_SCALE (cycle_sbar));

  gtk_box_pack_start (GTK_BOX (gg->control_panel[XYPLOT]),
                      cycle_sbar, false, false, 1);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), cycle_sbar,
    "Adjust cycling speed", NULL);

  chdir_btn = gtk_button_new_with_label ("Change direction");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), chdir_btn,
    "Change cycling direction", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[XYPLOT]),
                      chdir_btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (chdir_btn), "clicked",
                      GTK_SIGNAL_FUNC (chdir_cb), gg);

  gtk_widget_show_all (gg->control_panel[XYPLOT]);
}
