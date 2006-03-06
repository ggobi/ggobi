/* brush_ui.c */
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

/*
 * Code pertaining to the control panel for brushing.
*/

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include "vars.h"
#include "externs.h"

void
brush_update_set (gboolean update, displayd * dsp, ggobid * gg)
{
  dsp->cpanel.br.updateAlways_p = update;
}

void
brush_on_set (gboolean brushon, displayd * dsp, ggobid * gg)
{
  dsp->cpanel.br.brush_on_p = brushon;
  splot_redraw (gg->current_splot, QUICK, gg);
}

static void
brush_undo_cb (GtkToggleButton * button, ggobid * gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  splotd *sp = gg->current_splot;
  displayd *display = sp->displayptr;
  GGobiData *d = display->d;
  GGobiData *e = display->e;

  if (cpanel->br.point_targets)
    brush_undo (sp, d, gg);
  if (cpanel->br.edge_targets)
    brush_undo (sp, e, gg);

  /*-- when rows_in_plot changes ... --*/
  rows_in_plot_set (d, gg);
  if (GGOBI_IS_EXTENDED_SPLOT (sp)) {
    void (*f) (GGobiData *, splotd *, ggobid *);
    GGobiExtendedSPlotClass *klass;
    klass = GGOBI_EXTENDED_SPLOT_GET_CLASS (sp);
    f = klass->splot_assign_points_to_bins;
    if (f) {
      f (d, sp, gg);            // need to exclude area plots
    }
  }
  clusters_set (d, gg);
  /*-- --*/

  if (gg->cluster_ui.window != NULL)
    cluster_table_update (d, gg);

  displays_plot (NULL, FULL, gg);
}

static gchar *point_targets_lbl[] = {
  "Off", "Color and glyph", "Color only", "Glyph only", "Shadow", "Unshadow"
};
static void
brush_point_targets_cb (GtkWidget * w, ggobid * gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;

  if (cpanel->br.mode == BR_TRANSIENT)
    reinit_transient_brushing (gg->current_display, gg);

  cpanel->br.point_targets = gtk_combo_box_get_active (GTK_COMBO_BOX (w));

  /* binning not permitted here */
  brush_once_and_redraw (false, gg->current_splot, gg->current_display, gg);
}

static gchar *edge_targets_lbl[] = {
  "Off", "Color and line", "Color only", "Line only", "Shadow", "Unshadow"
};
static void
brush_edge_targets_cb (GtkWidget * w, ggobid * gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;

  if (cpanel->br.mode == BR_TRANSIENT)
    reinit_transient_brushing (gg->current_display, gg);

  cpanel->br.edge_targets = gtk_combo_box_get_active (GTK_COMBO_BOX (w));

  /* binning not permitted here */
  brush_once_and_redraw (false, gg->current_splot, gg->current_display, gg);
}

void
brush_mode_set (gint mode, splotd * sp, displayd * display, ggobid * gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;

  gint prev_mode = cpanel->br.mode;
  cpanel->br.mode = mode;
  if (mode == BR_PERSISTENT && mode != prev_mode) {
    brush_once (false, sp, gg);
  }
  display_plot (display, QUICK, gg);
}
static void
brush_mode_cb (GtkWidget * w, ggobid * gg)
{
  splotd *sp = gg->current_splot;
  brush_mode_set (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w)),
                  sp, gg->current_display, gg);
}

static void
open_symbol_window_cb (GtkWidget * w, ggobid * gg)
{
  make_symbol_window (gg);
}

void
brush_reset (displayd * display, gint action)
{
  gint i, k;
  ggobid *gg = display->ggobi;
  GGobiData *d = display->d;
  GGobiData *e = display->e;
  cpaneld *cpanel = &display->cpanel;

  g_assert (d->hidden.nels == d->nrows);
  if (e)
    g_assert (e->hidden.nels == e->nrows);

  switch (action) {

  case RESET_EXCLUDE_SHADOW_POINTS: /*-- exclude all shadowed points --*/
    include_hiddens (false, d, gg);
    break;
  case RESET_INCLUDE_SHADOW_POINTS: /*-- include all shadowed points --*/
    include_hiddens (true, d, gg);
    break;

  case RESET_UNSHADOW_POINTS: /*-- un-hide all points --*/
    for (i = 0; i < d->nrows; i++)
      d->hidden.els[i] = d->hidden_now.els[i] = false;
    rows_in_plot_set (d, gg);

      /*-- code borrowed from exclusion_ui.c, the 'show' routine --*/
    clusters_set (d, gg);
    cluster_table_labels_update (d, gg);
    rows_in_plot_set (d, gg);

    tform_to_world (d, gg);
    displays_tailpipe (FULL, gg);
      /*-- --*/
    break;

/*
 * Ambiguity:  If an edge is connected to a shadowed point, it's
 * drawn in shadow -- yet it isn't "hidden", so it doesn't respond
 * to this operation.  -- dfs
*/
  case RESET_EXCLUDE_SHADOW_EDGES: /*-- exclude all shadowed edges --*/
    if (e)
      include_hiddens (false, e, gg);
    break;
  case RESET_INCLUDE_SHADOW_EDGES: /*-- include all shadowed edges --*/
    if (e)
      include_hiddens (true, e, gg);
    break;

  case RESET_UNSHADOW_EDGES: /*-- un-hide all edges --*/
    if (e != NULL) {
      for (k = 0; k < e->edge.n; k++)
        e->hidden_now.els[k] = e->hidden.els[k] = false;
      rows_in_plot_set (e, gg);

        /*-- code borrowed from exclusion_ui.c, the 'show' routine --*/
      clusters_set (e, gg);
      cluster_table_labels_update (e, gg);
      rows_in_plot_set (e, gg);

      tform_to_world (e, gg);
      displays_tailpipe (FULL, gg);
        /*-- --*/
    }
    break;

  case RESET_INIT_BRUSH: /*-- reset brush size --*/
    brush_pos_init (gg->current_splot);

    if (cpanel->br.mode == BR_TRANSIENT) {
      reinit_transient_brushing (display, gg);
      displays_plot (NULL, FULL, gg);
    }
    else {
      splot_redraw (gg->current_splot, QUICK, gg);
    }
    break;


  case RESET_POINT_COLORS: /*-- reset point colors -- to what? --*/
  case RESET_POINT_GLYPHS: /*-- reset point glyphs -- to what? --*/
  case RESET_EDGE_COLORS: /*-- reset edge colors -- to what? --*/
  case RESET_EDGE_TYPES: /*-- reset edge colors -- to what? --*/
    break;
  }
}

/*--------------------------------------------------------------------*/
/*      Handling keyboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

#include <gdk/gdkkeysyms.h>

static gint
key_press_cb (GtkWidget * w, GdkEventKey * event, splotd * sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  cpaneld *cpanel = &gg->current_display->cpanel;

  if (!sp || !gg || !cpanel)
    return;

  /*-- handle the keys for setting the mode and launching generic events --*/
  if (splot_event_handled (w, event, cpanel, sp, gg))
    return true;

  /*-- insert mode-specific key presses (if any) here --*/
  /* Persistent/transient */
  if ((event->state & GDK_MOD1_MASK) == GDK_MOD1_MASK) {
    if (event->keyval == GDK_t || event->keyval == GDK_T) {
      brush_mode_set (BR_TRANSIENT, sp, gg->current_display, gg);
      return true;
    }
    else if (event->keyval == GDK_p || event->keyval == GDK_P) {
      brush_mode_set (BR_PERSISTENT, sp, gg->current_display, gg);
      return true;
    }
  }

  return false;
}

static gint
motion_notify_cb (GtkWidget * w, GdkEventMotion * event, cpaneld * cpanel)
{
  gboolean button1_p, button2_p;
  ggobid *gg = GGobiFromWidget (w, true);
  splotd *sp = gg->current_splot;

  /*-- get the mouse position and find out which buttons are pressed --*/
  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);

  if (button1_p || button2_p) {
    gboolean changed;
    changed =
      brush_motion (&sp->mousepos, button1_p, button2_p, cpanel, sp, gg);

    /*XXX
       Like this to be emitted from the display. Or what about the splotd? 
       or perhaps both the ggobi and the splotd? or perhaps only on
       scatterSPlotds
       And we might store the signal ids in the class itself.
     */
#if TEST_BRUSH_MOTION_CB
    fprintf (stderr,
             "emiting brush motion signal (w) %p (gg) %p (sp) %p (event) %p\n",
             (void *) w, (void *) gg, (void *) sp, (void *) event);
    fflush (stderr);
#endif
/*XX is this the correct source object? */
    if (changed)
      g_signal_emit (G_OBJECT (gg), GGobiSignals[BRUSH_MOTION_SIGNAL], 0,
                     sp, event, sp->displayptr->d);
  }
  return true;
}


/*-- response to the mouse click event --*/
static gint
button_press_cb (GtkWidget * w, GdkEventButton * event, splotd * sp)
{
  displayd *display;
  cpaneld *cpanel;
  gboolean retval = true;
  gboolean button1_p, button2_p;
  ggobid *gg = GGobiFromSPlot (sp);
  GGobiData *d, *e;

  if (!sp || !gg)
    return;

  gg->current_splot = sp->displayptr->current_splot = sp;
  gg->current_display = sp->displayptr;
  display = gg->current_display;
  cpanel = &display->cpanel;
  d = display->d;
  e = display->e;

  brush_prev_vectors_update (d, gg);
  if (e != NULL)
    brush_prev_vectors_update (e, gg);

  mousepos_get_pressed (w, event, &button1_p, &button2_p, sp);

  sp->motion_id = g_signal_connect (G_OBJECT (sp->da),
                                    "motion_notify_event",
                                    G_CALLBACK (motion_notify_cb),
                                    (gpointer) cpanel);

  brush_set_pos ((gint) sp->mousepos.x, (gint) sp->mousepos.y, sp);
  /*
   * We might need to make certain that the current splot is
   * redrawn without binning, in case some other plot is also in
   * transient brushing.
   */
  if (cpanel->br.brush_on_p) {
    brush_once_and_redraw (false, sp, display, gg); /* no binning */
  }
  else {
    splot_redraw (sp, QUICK, gg);
  }
  /*--  --*/

  return retval;
}

static gint
button_release_cb (GtkWidget * w, GdkEventButton * event, splotd * sp)
{
  displayd *display = (displayd *) sp->displayptr;
  ggobid *gg = GGobiFromSPlot (sp);
  cpaneld *cpanel = &display->cpanel;
  GGobiData *d = display->d;
  gboolean retval = true;
  GdkModifierType state;

  gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y,
                          &state);

  gg->buttondown = 0;

  disconnect_motion_signal (sp);
  gdk_pointer_ungrab (event->time);  /*-- grabbed in mousepos_get_pressed --*/

  if (cpanel->br.mode == BR_PERSISTENT) {
    rows_in_plot_set (d, gg);

    if (GGOBI_IS_EXTENDED_SPLOT (sp)) {
      void (*f) (GGobiData *, splotd *, ggobid *);
      GGobiExtendedSPlotClass *klass;
      klass = GGOBI_EXTENDED_SPLOT_GET_CLASS (sp);
      f = klass->splot_assign_points_to_bins;
      if (f) {
        f (d, sp, gg);          // need to exclude area plots
      }
    }
    /*-- reset the number and properties of the brush groups --*/
    clusters_set (d, gg);

/*   ??
 *  g_signal_emit (GTK_OBJECT (gg->main_window),
 *    gg->signal_symbols_changed, gg); 
 */

    /*-- If we've also been brushing an edge set, set its clusters --*/
/*
    if (display->e != NULL && cpanel->br.edge_targets != br_off) {
      clusters_set(display->e, gg);
    }
*/
    /*-- If we've been brushing by variable, set everybody's clusters --*/
    /*if (cpanel->br_linkby == BR_LINKBYVAR) { */

    /*
     * Since any datad might be linked to this one, reset
     * everybody's clusters until more elaborate tests are
     * necessary.
     */
    {
      GSList *l;
      GGobiData *dd;
      for (l = gg->d; l; l = l->next) {
        dd = (GGobiData *) l->data;
        if (dd != d) {
          clusters_set (dd, gg);
        }
      }
    }

    /*-- this updates the tables for every datad --*/
    cluster_table_update (d, gg);
  }

  /*-- if we're only doing linked brushing on mouse up, do it now --*/
  if (!cpanel->br.updateAlways_p)
    displays_plot (sp, FULL, gg);

  return retval;
}


/*--------------------------------------------------------------------*/
/*                 Add and remove event handlers                      */
/*--------------------------------------------------------------------*/

void
brush_event_handlers_toggle (splotd * sp, gboolean state)
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
  }
  else {
    disconnect_key_press_signal (sp);
    disconnect_button_press_signal (sp);
    disconnect_button_release_signal (sp);
  }
}

/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/

GtkWidget *create_linkby_notebook (GtkWidget *, ggobid *);

void
cpanel_brush_make (ggobid * gg)
{
  modepaneld *panel;
  GtkWidget *btn, *hb;
  GtkWidget *option_menu, *check_btn;
  GtkWidget *vb, *lbl;
  GtkWidget *notebook;

  panel = (modepaneld *) g_malloc (sizeof (modepaneld));
  gg->control_panels = g_list_append (gg->control_panels, (gpointer) panel);
  panel->name = g_strdup (GGOBI (getIModeName) (BRUSH));

  panel->w = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (panel->w), 5);

 /*-- button: open symbol panel --*/
  btn = gtk_button_new_with_mnemonic ("_Choose color & glyph...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                        "Open panel for choosing color and glyph", NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
                    G_CALLBACK (open_symbol_window_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (panel->w), btn, false, false, 1);


  /* hbox to hold the Persistent checkbox and the Undo button */
  hb = gtk_hbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (panel->w), hb, false, false, 0);

/*-- check button: persistent/transient --*/
  /*-- this was an option menu but was changed to allow accelerators in GTK2 */
  check_btn = gtk_check_button_new_with_mnemonic ("_Persistent");
  gtk_widget_set_name (check_btn, "BRUSH:mode_check_btn");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), check_btn,
                        "Persistent or transient brushing", NULL);
  g_signal_connect (G_OBJECT (check_btn), "clicked",
                    G_CALLBACK (brush_mode_cb), gg);
  gtk_box_pack_start (GTK_BOX (hb), check_btn, false, false, 0);
  /* initialize transient */
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_btn), false);

  {                             /* I really ought to do this by adding an argument to
                                 * populate_option_menu, but I'm not sure it's worth it just
                                 * for a single pair of accelerators.  
                                 * The other point about all these accelerator groups is
                                 * that I should be able to add the accel_groups to each display
                                 * instead of having separate key event management.  dfs 9/2/2005
                                 * - unfortunately it is not possible to drive an accelerator from
                                 * a window that is not the parent of the accelerated widget,
                                 * therefore key events seem necessary - mfl
                                 */
    /* Add accelerators to the option menu */

    /*GList *children = gtk_container_get_children(GTK_CONTAINER(menu));
       GList *child;
       GtkWidget *item; */
    /*guint accel = 'p';
       //gint i = 0;
       GtkAccelGroup *accel_group = gtk_accel_group_new();
       gtk_window_add_accel_group(GTK_WINDOW (gg->main_window), accel_group);
       gtk_widget_add_accelerator(check_btn, "toggled", accel_group, accel, 
       GDK_MOD2_MASK, GTK_ACCEL_VISIBLE); */
    /*for (child=children; child; child=child->next) {
       item = (GtkWidget *) child->data;
       gtk_widget_add_accelerator (item, "activate", accel_group,
       accels[i++], GDK_MOD2_MASK, GTK_ACCEL_VISIBLE);
       } */
  }

  btn = gtk_button_new_from_stock (GTK_STOCK_UNDO);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                        "Undo the most recent persistent brushing changes, from button down to button up",
                        NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, true, true, 0);
  g_signal_connect (G_OBJECT (btn), "clicked", G_CALLBACK (brush_undo_cb),
                    gg);


/*-- option menu: brush with color/glyph/both --*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (panel->w), vb, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("Poi_nt brushing:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  option_menu = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), option_menu);
  gtk_widget_set_name (option_menu, "BRUSH:point_targets_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), option_menu,
                        "Brushing points: what characteristics, if any, should respond?",
                        NULL);
  gtk_box_pack_start (GTK_BOX (vb), option_menu, false, false, 0);
  populate_combo_box (option_menu, point_targets_lbl,
                      G_N_ELEMENTS (point_targets_lbl), NULL, NULL);
  /*-- initial value: both --*/
  /* defer signal registration until after setting the active option */
  gtk_combo_box_set_active (GTK_COMBO_BOX (option_menu), 1);
  g_signal_connect (G_OBJECT (option_menu), "changed",
                    G_CALLBACK (brush_point_targets_cb), gg);

/*-- new, for edges --*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (panel->w), vb, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("_Edge brushing:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  /*-- option menu:  Off, color&line, color only, ... --*/
  option_menu = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), option_menu);
  gtk_widget_set_name (option_menu, "BRUSH:edge_targets_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), option_menu,
                        "Brushing edges: what characteristics, if any, should respond?",
                        NULL);
  gtk_box_pack_start (GTK_BOX (vb), option_menu, false, false, 0);
  populate_combo_box (option_menu, edge_targets_lbl,
                      G_N_ELEMENTS (edge_targets_lbl),
                      G_CALLBACK (brush_edge_targets_cb), gg);

  /*-- Define the linking rule --*/
  notebook = create_linkby_notebook (panel->w, gg);
  gtk_widget_set_name (notebook, "BRUSH:linkby_notebook");

/*-- button for opening 'color schemes' panel --*/
/*
  btn = gtk_button_new_with_mnemonic("Color _schemes ...");
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), btn,
    "Open tools panel for automatic brushing by variable",
    NULL);
  g_signal_connect(G_OBJECT(btn), "clicked",
    G_CALLBACK(wvis_window_cb), (gpointer) gg);
  gtk_box_pack_start(GTK_BOX(panel->w),
    btn, false, false, 1);
 */

/*-- button for opening clusters table --*/
/*
  btn = gtk_button_new_with_mnemonic("Color & glyph _groups ...");
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), btn,
    "Open tools panel for hiding or excluding brushed groups",
    NULL);
  g_signal_connect(G_OBJECT(btn), "clicked",
    G_CALLBACK(cluster_window_cb), (gpointer) gg);
  gtk_box_pack_start(GTK_BOX(panel->w),
    btn, false, false, 1);
*/

  /*
     btn = gtk_check_button_new_with_mnemonic("_Brush on");
     gtk_widget_set_name(btn, "BRUSH:brush_on_button");
     gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), btn,
     "Make the brush active or inactive.  Drag the left button to brush and the right or middle button  to resize the brush.",
     NULL);
     g_signal_connect(G_OBJECT(btn), "toggled",
     G_CALLBACK(brush_on_cb), (gpointer) gg);
     gtk_box_pack_start(GTK_BOX(panel->w), btn, false,
     false, 0);
   */

  gtk_widget_show_all (panel->w);
}

/*--------------------------------------------------------------------*/
/*                      Control panel section                         */
/*--------------------------------------------------------------------*/

void
cpanel_brush_init (cpaneld * cpanel, ggobid * gg)
{
  cpanel->br.brush_on_p = true;
  cpanel->br.updateAlways_p = true;

  cpanel->br.mode = BR_TRANSIENT;
  cpanel->br.linkby_row = 0;

  /*-- point brushing on, edge brushing off --*/
  cpanel->br.point_targets = br_candg;
  cpanel->br.edge_targets = br_off;
}

void
cpanel_brush_set (displayd * display, cpaneld * cpanel, ggobid * gg)
{
  GtkWidget *w;
  GtkWidget *pnl = mode_panel_get_by_name (GGOBI (getIModeName) (BRUSH), gg);
  GtkWidget *btn;

  if (pnl == (GtkWidget *) NULL)
    return;

  w = widget_find_by_name (pnl, "BRUSH:mode_check_btn");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), !cpanel->br.mode);

  /* Open the correct page in the linking rule notebook */
  w = widget_find_by_name (pnl, "BRUSH:linkby_notebook");
  linkby_current_page_set (display, w, gg);

  w = widget_find_by_name (pnl, "BRUSH:point_targets_option_menu");
  gtk_combo_box_set_active (GTK_COMBO_BOX (w), cpanel->br.point_targets);

  w = widget_find_by_name (pnl, "BRUSH:edge_targets_option_menu");
  gtk_combo_box_set_active (GTK_COMBO_BOX (w), cpanel->br.edge_targets);
}

/*--------------------------------------------------------------------*/
