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

/*-- called from Options menu --*/
void brush_update_set_cb(GtkCheckMenuItem * w, guint action)
{
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(w), true);
  gg->brush.updateAlways_p = !gg->brush.updateAlways_p;
}

static void brush_on_cb(GtkToggleButton * button, ggobid * gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->br.brush_on_p = button->active;
  splot_redraw(gg->current_splot, QUICK, gg);
}

static void brush_undo_cb(GtkToggleButton * button, ggobid * gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  splotd *sp = gg->current_splot;
  displayd *display = sp->displayptr;
  datad *d = display->d;
  datad *e = display->e;

  if (cpanel->br.point_targets)
    brush_undo(sp, d, gg);
  if (cpanel->br.edge_targets)
    brush_undo(sp, e, gg);

  /*-- when rows_in_plot changes ... --*/
  rows_in_plot_set(d, gg);
  assign_points_to_bins(d, gg);
  clusters_set(d, gg);
  /*-- --*/

  if (gg->cluster_ui.window != NULL)
    cluster_table_update(d, gg);

  displays_plot(NULL, FULL, gg);
}
/*
The select mode has some unpleasant behavior, basically because I'm
just re-using the hidden vectors.  I have no desire to add another
set of vectors, though, so I'll just turn it off for now.
*/
static gchar *point_targets_lbl[] =
{ "Off", "Color and glyph", "Color only", "Glyph only", "Shadow", /*"Select"*/};
static void
brush_point_targets_cb (GtkWidget * w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;

  if (cpanel->br.mode == BR_TRANSIENT)
    reinit_transient_brushing (gg->current_display, gg);

  cpanel->br.point_targets = GPOINTER_TO_INT(cbd);

  /* binning not permitted here */
  brush_once_and_redraw (false, gg->current_splot, gg->current_display, gg);


  /*
   * select brushing is a special case: hide all points
   * before starting to move the brush.
   *
   * There's still a weird thing that can happen:  enter select
   * brushing and then leave it; all points remain hidden.  Hmm.
  */ /* select brushing has been disabled */
  /*
  if (cpanel->br_point_targets == br_select) {
    gint i, m;
    datad *d = gg->current_display->d;

    g_assert (d->hidden.nels == d->nrows);

    for (i=0; i<d->nrows_in_plot; i++) {
      m = d->rows_in_plot.els[i];
      d->hidden_now.els[m] = d->hidden.els[m] = true;
    }
    displays_plot (NULL, FULL, gg);
  }
  */

}

static gchar *edge_targets_lbl[] =
  { "Off", "Color and line", "Color only", "Line only", "Shadow", 
    /*"Select"*/};
static void brush_edge_targets_cb(GtkWidget * w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;

  if (cpanel->br.mode == BR_TRANSIENT)
    reinit_transient_brushing (gg->current_display, gg);

  cpanel->br.edge_targets = GPOINTER_TO_INT(cbd);

  /* binning not permitted here */
  brush_once_and_redraw (false, gg->current_splot, gg->current_display, gg);

  /*
   * select brushing is a special case: hide all points
   * before starting to move the brush.
  */  /* select brushing has been disabled */
  /*
  if (cpanel->br.edge_targets == br_select) {
    gint i, m;
    datad *e = gg->current_display->e;

    g_assert (e->hidden.nels == e->nrows);

    if (e) {
      for (i=0; i<e->nrows_in_plot; i++) {
        m = e->rows_in_plot.els[i];
        e->hidden_now.els[m] = e->hidden.els[m] = true;
      }
      displays_plot (NULL, FULL, gg);
    }
  }
  */


}

static gchar *mode_lbl[] = { "Persistent", "Transient" };
static void brush_mode_cb(GtkWidget * w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  splotd *sp = gg->current_splot;
  cpaneld *cpanel = &gg->current_display->cpanel;
  gint prev_mode = cpanel->br.mode;

  cpanel->br.mode = GPOINTER_TO_INT(cbd);

  if (cpanel->br.mode == BR_PERSISTENT && cpanel->br.mode != prev_mode) {
    brush_once(false, sp, gg);
  }
}

static void open_symbol_window_cb(GtkWidget * w, ggobid * gg)
{
  make_symbol_window(gg);
}

static void cluster_window_cb(GtkWidget * button, ggobid * gg)
{
  cluster_window_open(gg);
}

static void wvis_window_cb(GtkWidget * button, ggobid * gg)
{
  wvis_window_open(gg);
}

/*
 * Callbacks for menus in the main menubar
*/

/* Actions from the Reset menu in the main menubar */
void brush_reset_cb(GtkWidget * w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  gint action = GPOINTER_TO_INT(cbd);
  brush_reset(gg, action);
}

void brush_reset(ggobid * gg, gint action)
{
  gint i, k;
  displayd *display = gg->current_display;
  datad *d = display->d;
  datad *e = display->e;
  cpaneld *cpanel = &display->cpanel;

  g_assert (d->hidden.nels == d->nrows);
  if (e)
    g_assert (e->hidden.nels == e->nrows);

  switch (action) {

  case RESET_EXCLUDE_SHADOW_POINTS:   /*-- exclude all shadowed points --*/
    include_hiddens (false, d, gg);
  break;
  case RESET_INCLUDE_SHADOW_POINTS:   /*-- include all shadowed points --*/
    include_hiddens (true, d, gg);
  break;

  case RESET_UNSHADOW_POINTS:   /*-- un-hide all points --*/
    for (i = 0; i < d->nrows; i++)
      d->hidden.els[i] = d->hidden_now.els[i] = false;
    rows_in_plot_set(d, gg);

      /*-- code borrowed from exclusion_ui.c, the 'show' routine --*/
    clusters_set(d, gg);
    cluster_table_labels_update(d, gg);
    rows_in_plot_set(d, gg);

    tform_to_world(d, gg);
    displays_tailpipe(FULL, gg);
      /*-- --*/
    break;

/*
 * Ambiguity:  If an edge is connected to a shadowed point, it's
 * drawn in shadow -- yet it isn't "hidden", so it doesn't respond
 * to this operation.  -- dfs
*/
  case RESET_EXCLUDE_SHADOW_EDGES:   /*-- exclude all shadowed edges --*/
    if (e)
      include_hiddens (false, e, gg);
  break;
  case RESET_INCLUDE_SHADOW_EDGES:   /*-- include all shadowed edges --*/
    if (e)
      include_hiddens (true, e, gg);
  break;

  case RESET_UNSHADOW_EDGES:   /*-- un-hide all edges --*/
    if (e != NULL) {
      for (k = 0; k < e->edge.n; k++)
        e->hidden_now.els[k] = e->hidden.els[k] = false;
      rows_in_plot_set(e, gg);

        /*-- code borrowed from exclusion_ui.c, the 'show' routine --*/
      clusters_set(e, gg);
      cluster_table_labels_update(e, gg);
      rows_in_plot_set(e, gg);

      tform_to_world(e, gg);
      displays_tailpipe(FULL, gg);
        /*-- --*/
    }
    break;

  case RESET_INIT_BRUSH:   /*-- reset brush size --*/
    brush_pos_init(gg->current_splot);

    if (cpanel->br.mode == BR_TRANSIENT) {
      reinit_transient_brushing(display, gg);
      displays_plot(NULL, FULL, gg);
    } else {
      splot_redraw(gg->current_splot, QUICK, gg);
    }
    break;


  case RESET_POINT_COLORS:   /*-- reset point colors -- to what? --*/
  case RESET_POINT_GLYPHS:   /*-- reset point glyphs -- to what? --*/
  case RESET_EDGE_COLORS:   /*-- reset edge colors -- to what? --*/
  case RESET_EDGE_TYPES:   /*-- reset edge colors -- to what? --*/
    break;
  }
}

/*--------------------------------------------------------------------*/
/*      Handling keyboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

static gint key_press_cb(GtkWidget * w, GdkEventKey * event, splotd * sp)
{
  ggobid *gg = GGobiFromSPlot(sp);
  cpaneld *cpanel = &gg->current_display->cpanel;

  /*-- handle the keys for setting the mode and launching generic events --*/
  if (splot_event_handled(w, event, cpanel, sp, gg))
    return true;

  /*-- insert mode-specific key presses (if any) here --*/

  return true;
}

static gint
motion_notify_cb(GtkWidget *w, GdkEventMotion *event, cpaneld *cpanel)
{
  gboolean button1_p, button2_p;
  ggobid *gg = GGobiFromWidget(w, true);
  splotd *sp = gg->current_splot;

  /*-- get the mouse position and find out which buttons are pressed --*/
  mousepos_get_motion(w, event, &button1_p, &button2_p, sp);

  if (button1_p || button2_p) {
    gboolean changed;
    changed = brush_motion(&sp->mousepos, button1_p, button2_p, cpanel, sp, gg);

    /*XXX
      Like this to be emitted from the display. Or what about the splotd? 
      or perhaps both the ggobi and the splotd? or perhaps only on
      scatterSPlotds
      And we might store the signal ids in the class itself.
     */
#if TEST_BRUSH_MOTION_CB
    fprintf(stderr,
            "emiting brush motion signal (w) %p (gg) %p (sp) %p (event) %p\n",
              (void *) w,  (void*) gg,  (void*) sp, (void *)event);
    fflush(stderr);
#endif
/*XX is this the correct source object? */
    if(changed)
      gtk_signal_emit(GTK_OBJECT(gg), GGobiSignals[BRUSH_MOTION_SIGNAL],
         sp, event, sp->displayptr->d);
  }
  return true;
}


/*-- response to the mouse click event --*/
static gint
button_press_cb(GtkWidget * w, GdkEventButton * event, splotd * sp)
{
  displayd *display;
  cpaneld *cpanel;
  gboolean retval = true;
  gboolean button1_p, button2_p;
  ggobid *gg = GGobiFromSPlot(sp);
  datad *d, *e;

  gg->current_splot = sp->displayptr->current_splot = sp;
  gg->current_display = sp->displayptr;
  display = gg->current_display;
  cpanel = &display->cpanel;
  d = display->d;
  e = display->e;

  /*-- set the value of the boolean gg->linkby_cv --*/
  /*
  linking_method_set(display, d, gg);

Do I have to execute something here so that I can do the right
thing when there are multiple datasets?
  */

  brush_prev_vectors_update(d, gg);
  if (e != NULL)
    brush_prev_vectors_update(e, gg);

  mousepos_get_pressed(w, event, &button1_p, &button2_p, sp);

  sp->motion_id = gtk_signal_connect(GTK_OBJECT(sp->da),
                                     "motion_notify_event",
                                     (GtkSignalFunc) motion_notify_cb,
                                     (gpointer) cpanel);

  brush_set_pos((gint) sp->mousepos.x, (gint) sp->mousepos.y, sp);
  /*
   * We might need to make certain that the current splot is
   * redrawn without binning, in case some other plot is also in
   * transient brushing.
   */
  if (cpanel->br.brush_on_p) {
    brush_once_and_redraw(false, sp, display, gg);      /* no binning */
  } else {
    splot_redraw(sp, QUICK, gg);
  }
  /*--  --*/

  return retval;
}

static gint
button_release_cb(GtkWidget * w, GdkEventButton * event, splotd * sp)
{
  displayd *display = (displayd *) sp->displayptr;
  ggobid *gg = GGobiFromSPlot(sp);
  cpaneld *cpanel = &display->cpanel;
  datad *d = display->d;
  gboolean retval = true;
  GdkModifierType state;

  gdk_window_get_pointer(w->window, &sp->mousepos.x, &sp->mousepos.y,
                         &state);

  gg->buttondown = 0;

  disconnect_motion_signal(sp);
  gdk_pointer_ungrab(event->time);   /*-- grabbed in mousepos_get_pressed --*/

  if (cpanel->br.mode == BR_PERSISTENT) {
    rows_in_plot_set(d, gg);

    assign_points_to_bins(d, gg);
    /*-- reset the number and properties of the brush groups --*/
    clusters_set(d, gg);

/*   ??
 *  gtk_signal_emit (GTK_OBJECT (gg->main_window),
 *    gg->signal_symbols_changed, gg); 
 */

    /*-- If we've also been brushing an edge set, set its clusters --*/
/*
    if (display->e != NULL && cpanel->br.edge_targets != br_off) {
      clusters_set(display->e, gg);
    }
*/
    /*-- If we've been brushing by variable, set everybody's clusters --*/
    /*if (cpanel->br_linkby == BR_LINKBYVAR) {*/

    /*
     * Since any datad might be linked to this one, reset
     * everybody's clusters until more elaborate tests are
     * necessary.
    */
    {
      GSList *l;
      datad *dd;
      for (l = gg->d; l; l = l->next) {
        dd = (datad *) l->data;
        if (dd != d) {
          clusters_set (dd, gg);
        }
      }
    }

    /*-- this updates the tables for every datad --*/
    cluster_table_update (d, gg);
  }

  /*-- if we're only doing linked brushing on mouse up, do it now --*/
  if (!gg->brush.updateAlways_p)
    displays_plot(sp, FULL, gg);

  return retval;
}


/*--------------------------------------------------------------------*/
/*                 Add and remove event handlers                      */
/*--------------------------------------------------------------------*/

void brush_event_handlers_toggle(splotd * sp, gboolean state)
{
  displayd *display = (displayd *) sp->displayptr;

  if (state == on) {
    if (GTK_IS_GGOBI_WINDOW_DISPLAY(display) && GTK_GGOBI_WINDOW_DISPLAY(display)->useWindow)
      sp->key_press_id = gtk_signal_connect(GTK_OBJECT
        (GTK_GGOBI_WINDOW_DISPLAY(display)->window),
        "key_press_event",
        (GtkSignalFunc) key_press_cb, (gpointer) sp);


    sp->press_id = gtk_signal_connect(GTK_OBJECT(sp->da),
      "button_press_event",
      (GtkSignalFunc) button_press_cb,
      (gpointer) sp);
    sp->release_id = gtk_signal_connect(GTK_OBJECT(sp->da),
      "button_release_event",
      (GtkSignalFunc) button_release_cb,
      (gpointer) sp);
  } else {
    disconnect_key_press_signal(sp);
    disconnect_button_press_signal(sp);
    disconnect_button_release_signal(sp);
  }
}

/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/

/* dfs testing */
GtkWidget *create_linkby_notebook (GtkWidget *, ggobid *);

void cpanel_brush_make(ggobid * gg)
{
  modepaneld *panel;
  GtkWidget *btn;
  GtkWidget *option_menu;
  GtkWidget *vb, *lbl;
  GtkWidget *notebook;

  panel = (modepaneld *) g_malloc(sizeof(modepaneld));
  gg->control_panels = g_list_append(gg->control_panels, (gpointer) panel);
  panel->name = g_strdup(GGOBI(getIModeName)(BRUSH));

  panel->w = gtk_vbox_new(false, VBOX_SPACING);
  gtk_container_set_border_width(GTK_CONTAINER(panel->w),
                                 5);

 /*-- button: open symbol panel --*/
  btn = gtk_button_new_with_label("Choose color & glyph ...");
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), btn,
    "Open panel for choosing color and glyph", NULL);
  gtk_signal_connect(GTK_OBJECT(btn), "clicked",
    GTK_SIGNAL_FUNC(open_symbol_window_cb), (gpointer) gg);
  gtk_box_pack_start(GTK_BOX(panel->w),
    btn, false, false, 1);

/*-- option menu: persistent/transient --*/
  option_menu = gtk_option_menu_new();
  gtk_widget_set_name(option_menu, "BRUSH:mode_option_menu");
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), option_menu,
    "Persistent or transient brushing", NULL);
  gtk_box_pack_start(GTK_BOX(panel->w),
    option_menu, false, false, 0);
  populate_option_menu(option_menu, mode_lbl,
    sizeof(mode_lbl) / sizeof(gchar *),
    (GtkSignalFunc) brush_mode_cb, "GGobi", gg);
  /* initialize transient */
  gtk_option_menu_set_history(GTK_OPTION_MENU(option_menu), 1);

/*-- option menu: brush with color/glyph/both --*/
  vb = gtk_vbox_new(false, 0);
  gtk_box_pack_start(GTK_BOX(panel->w), vb,
                     false, false, 0);

  lbl = gtk_label_new("Point brushing:");
  gtk_misc_set_alignment(GTK_MISC(lbl), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(vb), lbl, false, false, 0);

  option_menu = gtk_option_menu_new();
  gtk_widget_set_name(option_menu, "BRUSH:point_targets_option_menu");
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), option_menu,
    "Brushing points: what characteristics, if any, should respond?",
    NULL);
  gtk_box_pack_start(GTK_BOX(vb), option_menu, false, false, 0);
  populate_option_menu(option_menu, point_targets_lbl,
    sizeof(point_targets_lbl) / sizeof(gchar *),
    (GtkSignalFunc) brush_point_targets_cb, "GGobi", gg);
  /*-- initial value: both --*/
  gtk_option_menu_set_history(GTK_OPTION_MENU(option_menu), 1);


/*-- new, for edges --*/
  vb = gtk_vbox_new(false, 0);
  gtk_box_pack_start(GTK_BOX(panel->w), vb,
                     false, false, 0);

  lbl = gtk_label_new("Edge brushing:");
  gtk_misc_set_alignment(GTK_MISC(lbl), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(vb), lbl, false, false, 0);

  /*-- option menu:  Off, color&line, color only, ... --*/
  option_menu = gtk_option_menu_new();
  gtk_widget_set_name(option_menu, "BRUSH:edge_targets_option_menu");
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), option_menu,
    "Brushing edges: what characteristics, if any, should respond?",
    NULL);
  gtk_box_pack_start(GTK_BOX(vb), option_menu, false, false, 0);
  populate_option_menu(option_menu, edge_targets_lbl,
    sizeof(edge_targets_lbl) / sizeof(gchar *),
    (GtkSignalFunc) brush_edge_targets_cb, "GGobi", gg);
  /*-- initial value: off --*/
  gtk_option_menu_set_history(GTK_OPTION_MENU(option_menu), 0);


  btn = gtk_button_new_with_label("Undo");
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), btn,
    "Undo the most recent brushing changes, from button down to button up",
    NULL);
  gtk_box_pack_start(GTK_BOX(panel->w),
    btn, false, false, 0);
  gtk_signal_connect(GTK_OBJECT(btn), "clicked",
    GTK_SIGNAL_FUNC(brush_undo_cb), gg);

  /*-- Define the linking rule --*/
  notebook = create_linkby_notebook (panel->w, gg);
  gtk_widget_set_name(notebook, "BRUSH:linkby_notebook");

/*-- button for opening 'color schemes' panel --*/
  btn = gtk_button_new_with_label("Color schemes ...");
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), btn,
    "Open tools panel for automatic brushing by variable",
    NULL);
  gtk_signal_connect(GTK_OBJECT(btn), "clicked",
    GTK_SIGNAL_FUNC(wvis_window_cb), (gpointer) gg);
  gtk_box_pack_start(GTK_BOX(panel->w),
    btn, false, false, 1);

/*-- button for opening clusters table --*/
  btn = gtk_button_new_with_label("Color & glyph groups ...");
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), btn,
    "Open tools panel for hiding or excluding brushed groups",
    NULL);
  gtk_signal_connect(GTK_OBJECT(btn), "clicked",
    GTK_SIGNAL_FUNC(cluster_window_cb), (gpointer) gg);
  gtk_box_pack_start(GTK_BOX(panel->w),
    btn, false, false, 1);

  btn = gtk_check_button_new_with_label("Brush on");
  gtk_widget_set_name(btn, "BRUSH:brush_on_button");
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), btn,
    "Make the brush active or inactive.  Drag the left button to brush and the right or middle button  to resize the brush.",
    NULL);
  gtk_signal_connect(GTK_OBJECT(btn), "toggled",
    GTK_SIGNAL_FUNC(brush_on_cb), (gpointer) gg);
  gtk_box_pack_start(GTK_BOX(panel->w), btn, false,
  false, 0);

  gtk_widget_show_all(panel->w);
}

/*--------------------------------------------------------------------*/
/*                      Control panel section                         */
/*--------------------------------------------------------------------*/

void cpanel_brush_init(cpaneld *cpanel, ggobid *gg)
{
  cpanel->br.brush_on_p = true;

  cpanel->br.mode = BR_TRANSIENT;
  cpanel->br.linkby_page = 0;
  cpanel->br.linkby_row = 0;

  /*-- point brushing on, edge brushing off --*/
  cpanel->br.point_targets = br_candg;
  cpanel->br.edge_targets = br_off;
}

void cpanel_brush_set(displayd *display, cpaneld *cpanel, ggobid *gg)
{
  GtkWidget *w;
  GtkWidget *pnl = mode_panel_get_by_name(GGOBI(getIModeName)(BRUSH), gg);
  GtkWidget *btn;

  if (pnl == (GtkWidget *) NULL)
    return;

  btn = widget_find_by_name(pnl, "BRUSH:brush_on_button");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(btn),
    cpanel->br.brush_on_p);

  w = widget_find_by_name(pnl, "BRUSH:mode_option_menu");
  gtk_option_menu_set_history(GTK_OPTION_MENU(w), cpanel->br.mode);

  /* Open the correct page in the linking rule notebook */
  w = widget_find_by_name(pnl, "BRUSH:linkby_notebook");
  gtk_notebook_set_page(GTK_NOTEBOOK(w), cpanel->br.linkby_page);
  /* Get the clist (how?) and set the current row number. */
  /* It's the notebook page corresponding to display->d -- uh-oh, what
     if it's display->e?? */
  /* Set the values for gg->linkby_cv and d->linkvar_vt based on
     cpanel->br_linkby and the state of the notebook page? */

  w = widget_find_by_name(pnl, "BRUSH:point_targets_option_menu");
  gtk_option_menu_set_history(GTK_OPTION_MENU(w),
                              cpanel->br.point_targets);

  w = widget_find_by_name(pnl, "BRUSH:edge_targets_option_menu");
  gtk_option_menu_set_history(GTK_OPTION_MENU(w), cpanel->br.edge_targets);
}

/*--------------------------------------------------------------------*/
