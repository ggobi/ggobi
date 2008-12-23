/*-- xyplot_ui.c --*/
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

#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

/*--------------------------------------------------------------------*/
/*                           Cycling                                  */
/*--------------------------------------------------------------------*/

static const gchar *const fix_axis_lbl[] = {"No fixed axes", "Fix X", "Fix Y"};
static void fix_axis_cb (GtkWidget *w, ggobid *gg)
{
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;
  
  cpanel->xyplot.cycle_axis = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
}

void
xyplot_cycle_activate (gboolean state, cpaneld *cpanel, ggobid *gg)
{
  if (state) {
    gg->xyplot.cycle_id = g_timeout_add (cpanel->xyplot.cycle_delay,
      (GSourceFunc) xycycle_func, (gpointer) gg);
    cpanel->xyplot.cycle_p = true;
  } else {
    if (gg->xyplot.cycle_id) {
      g_source_remove (gg->xyplot.cycle_id);
      gg->xyplot.cycle_id = 0;
      cpanel->xyplot.cycle_p = false;
    }
  }
}
static void cycle_cb (GtkToggleButton *button, ggobid *gg)
{
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;

  cpanel->xyplot.cycle_p = button->active;
  xyplot_cycle_activate (cpanel->xyplot.cycle_p, cpanel, gg);
}

static void cycle_speed_cb (GtkAdjustment *adj, ggobid *gg) {
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;

  cpanel->xyplot.cycle_delay = -1 * (guint32) adj->value;
  if (cpanel->xyplot.cycle_p) {
    g_source_remove (gg->xyplot.cycle_id);
    gg->xyplot.cycle_id = g_timeout_add (cpanel->xyplot.cycle_delay,
    (GSourceFunc) xycycle_func, (gpointer) gg);
  }
}

static void chdir_cb (GtkButton *button, ggobid* gg)
{
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;

  cpanel->xyplot.cycle_dir = -1 * cpanel->xyplot.cycle_dir;
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

  return false;
}

void
xyplot_event_handlers_toggle (splotd *sp, gboolean state) {
  displayd *display = (displayd *) sp->displayptr;

  if (state == on) {
    if(GGOBI_IS_WINDOW_DISPLAY(display) && GGOBI_WINDOW_DISPLAY(display)->window)
      sp->key_press_id = g_signal_connect (G_OBJECT (GGOBI_WINDOW_DISPLAY(display)->window),
        "key_press_event",
        G_CALLBACK(key_press_cb),
        (gpointer) sp);
  } else {
    disconnect_key_press_signal (sp);
  }
}

/*--------------------------------------------------------------------*/

void
cpanel_xyplot_make (ggobid *gg) {
  modepaneld *panel;
  GtkWidget *frame, *vb, *cycle_tgl, *chdir_btn, *cycle_sbar, *opt;

  /* Make a panel, and add it to the GList gg->control_panels. */

  panel = (modepaneld *) g_malloc(sizeof(modepaneld));
  gg->control_panels = g_list_append(gg->control_panels, (gpointer) panel);
  panel->name = g_strdup(GGOBI(getPModeName)(XYPLOT));
  panel->w = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (panel->w), 5);

  frame = gtk_frame_new ("Plot cycling");
  //gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (panel->w), frame,
    false, false, 3);

  vb = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
  gtk_container_add (GTK_CONTAINER (frame), vb);

  cycle_tgl = gtk_check_button_new_with_mnemonic ("_Cycle");
  gtk_widget_set_name (cycle_tgl, "XYPLOT:cycle_toggle");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), cycle_tgl,
    "Cycle through pairwise plots", NULL);
  g_signal_connect (G_OBJECT (cycle_tgl), "toggled",
                     G_CALLBACK (cycle_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (vb), cycle_tgl,
    false, false, 3);

/*
 * make an option menu
*/
  opt = gtk_combo_box_new_text ();
  gtk_widget_set_name (opt, "XYPLOT:cycle_axis");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Fix one of the axes during plot cycling or let them both float", NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt,
    false, false, 0);
  populate_combo_box (opt, (gchar**) fix_axis_lbl, G_N_ELEMENTS(fix_axis_lbl),
    G_CALLBACK(fix_axis_cb), gg);
  
  /* value, lower, upper, step_increment, page_increment, page_size */
  /* Note that the page_size value only makes a difference for
   * scrollbar widgets, and the highest value you'll get is actually
   * (upper - page_size). */
  gg->xyplot.cycle_delay_adj = (GtkAdjustment *)
    gtk_adjustment_new (-1.0 * 1000 /* cpanel->xyplot.cycle_delay */,
    -5000.0, -250.0, 100.0, 1000.0, 0.0);

  g_signal_connect (G_OBJECT (gg->xyplot.cycle_delay_adj), "value_changed",
                      G_CALLBACK (cycle_speed_cb), gg);

  cycle_sbar = gtk_hscale_new (GTK_ADJUSTMENT (gg->xyplot.cycle_delay_adj));
  scale_set_default_values (GTK_SCALE (cycle_sbar));

  gtk_box_pack_start (GTK_BOX (vb),
                      cycle_sbar, false, false, 1);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), cycle_sbar,
    "Adjust cycling speed", NULL);

  chdir_btn = gtk_button_new_with_mnemonic ("Change di_rection");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), chdir_btn,
    "Change cycling direction", NULL);
  gtk_box_pack_start (GTK_BOX (vb),
                      chdir_btn, false, false, 1);
  g_signal_connect (G_OBJECT (chdir_btn), "clicked",
                      G_CALLBACK (chdir_cb), gg);


  gtk_widget_show_all (panel->w);
}

/*--------------------------------------------------------------------*/
/*                      Control panel section                         */
/*--------------------------------------------------------------------*/

void
cpanel_xyplot_init (cpaneld *cpanel, ggobid *gg)
{
  /*-- cycling --*/
  cpanel->xyplot.cycle_dir = FORWARD;
  cpanel->xyplot.cycle_p = false;
  cpanel->xyplot.cycle_axis = NOFIXED;
  cpanel->xyplot.cycle_delay = 1000;
}

void
cpanel_xyplot_set (displayd *display, cpaneld *cpanel, ggobid* gg)
/*
 * To handle the case where there are multiple scatterplots
 * which may have different xyplot cycling options and parameters selected
*/
{
  GtkWidget *pnl, *w;

  pnl = (GtkWidget *) mode_panel_get_by_name(GGOBI(getPModeName)(XYPLOT), gg);

  if (pnl) {

    /*-- Cycling on or off --*/
    w = widget_find_by_name (pnl, "XYPLOT:cycle_toggle");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), 
      cpanel->xyplot.cycle_p);

    /*-- Cycling speed --*/
    gtk_adjustment_set_value (GTK_ADJUSTMENT (gg->xyplot.cycle_delay_adj),
      -1 * (gdouble) cpanel->xyplot.cycle_delay);

    /*-- Cycling axis --*/
    w = widget_find_by_name (pnl, "XYPLOT:cycle_axis");
    gtk_combo_box_set_active (GTK_COMBO_BOX(w), cpanel->xyplot.cycle_axis);
  }
}
