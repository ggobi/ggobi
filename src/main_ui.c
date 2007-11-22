/* main_ui.c */
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

#include <string.h>
#include <stdlib.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"
#include "display_tree.h"

#ifdef STORE_SESSION_ENABLED
#include "write_state.h"
#endif

#include "plugin.h"


#ifdef TEST_GGOBI_EVENTS
#include "testEvents.h"
#endif

#include "scatmatClass.h"

const char *const
GGOBI (PModeNames)[] =
{
"DefaultPMode",
    "1D Plot",
    "XY Plot",
    "1D Tour", "Rotation", "2D Tour", "2x1D Tour", "ExtendedDisplayPMode"};
const char *const
GGOBI (IModeNames)[] =
{
"DefaultIMode", "Scale", "Brush", "Identify", "Edit Edges", "Move Points",};
static const char *const *pmode_name = GGOBI (PModeNames);
static const char *const *imode_name = GGOBI (IModeNames);

const char *const
GGOBI (PModeKeys)[] =
{
"", "d", "x", "t", "r", "g", "c", "", ""};
const char *const
GGOBI (IModeKeys)[] =
{
"", "s", "b", "i", "e", "m", "", ""};

void addPreviousFilesMenu (GGobiInitInfo * info, ggobid * gg);

#ifdef STORE_SESSION_ENABLED
void store_session (ggobid * gg);
#endif
void show_plugin_list (ggobid * gg);
void create_new_ggobi ();

/* Listen for display_selected events; update control panel */
void
control_panel_display_selected_cb (ggobid * gg, displayd * display)
{
  cpanel_set (display, gg);
}

void
make_control_panels (ggobid * gg)
{
  cpanel_p1dplot_make (gg);
  cpanel_xyplot_make (gg);
  cpanel_tour1d_make (gg);
  cpanel_tour2d3_make (gg);
  cpanel_tour2d_make (gg);
  cpanel_ctour_make (gg);

  cpanel_brush_make (gg);
  cpanel_scale_make (gg);
  cpanel_identify_make (gg);
  cpanel_edgeedit_make (gg);
  cpanel_movepts_make (gg);

  /* Remove from here, and do like parcoords etc.
   * cpanel_scatmat_make (gg);  
   */

  /* Leave the extendeded display types to be done on demand. */
}

/* This can return NULL, so calling routines must check */
GtkWidget *
mode_panel_get_by_name (const gchar * name, ggobid * gg)
{
  GList *l;
  GtkWidget *w = NULL;
  modepaneld *pnl;

  for (l = gg->control_panels; l; l = l->next) {
    pnl = (modepaneld *) l->data;
    if (strcmp (name, pnl->name) == 0) {
      w = pnl->w;
      break;
    }
  }
  return (GtkWidget *) w;
}

void
tooltips_show (gboolean show, ggobid * gg)
{
  if (show)
    gtk_tooltips_enable (gg->tips);
  else
    gtk_tooltips_disable (gg->tips);
}

void
statusbar_show (gboolean show, ggobid * gg)
{
  GtkWidget *entry = (GtkWidget *)
    g_object_get_data (G_OBJECT (gg->main_window), "MAIN:STATUSBAR");
  if (entry) {
    if (show)
      gtk_widget_show (entry);
    else
      gtk_widget_hide (entry);
  }
  gg->statusbar_p = show;
}

/*
  gg->status_message_func((gchar *)domain_error_message, gg);
*/
void
gg_write_to_statusbar (gchar * message, ggobid * gg)
{
  GtkWidget *statusbar = (GtkWidget *)
    g_object_get_data (G_OBJECT (gg->main_window), "MAIN:STATUSBAR");

  // for now we pop by default to prevent memory leaking, but we could
  // support a temporary statusbar message in which case we would not pop
  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 0);
  if (message)
    gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, message);
  else {
    /*-- by default, describe the current datad --*/
    GGobiData *d = datad_get_from_notebook (gg->varpanel_ui.notebook, gg);
    if (d) {
      gchar *msg = g_strdup_printf ("%s: %d x %d  (%s)",
                                    d->name, d->nrows, d->ncols,
                                    gg->input->fileName);
      gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, msg);
      g_free (msg);
    }
  }
}

void
cpanel_show (gboolean show, ggobid * gg)
{
  if (gg->imode_frame) {
    if (show)
      gtk_widget_show (gg->imode_frame);
    else
      gtk_widget_hide (gg->imode_frame);
  }
}

ProjectionMode
pmode_get (displayd * dsp, ggobid * gg)
{
  if (dsp == NULL)
    return gg->pmode;
  else
    return dsp->cpanel.pmode;
}

InteractionMode
imode_get (ggobid * gg)
{
  return gg->imode;
}

/*
 * Use the mode to determine whether the variable selection
 * panel should display checkboxes or circles
*/
static gboolean
varpanel_highd (displayd * display)
{
  gboolean highd = false;

  if (display && GGOBI_IS_EXTENDED_DISPLAY (display)) {
    GGobiExtendedDisplayClass *klass;
    klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
    if (klass->varpanel_highd)
      highd = klass->varpanel_highd (display);
  }

  return (highd);
}

/*
 * Use the widget state to figure out which is currently displayed.
*/
static gboolean
varpanel_shows_circles (GGobiData * d)
{
  return (d != NULL &&
          d->vcirc_ui.ebox != NULL && GTK_WIDGET_REALIZED (d->vcirc_ui.ebox));
}

static gboolean
varpanel_shows_checkboxes (GGobiData * d)
{
  return (d != NULL &&
          d->vcbox_ui.ebox != NULL && GTK_WIDGET_REALIZED (d->vcbox_ui.ebox));
}

void
varpanel_reinit (ggobid * gg)
{
  GGobiData *d;
  gboolean highd;
  displayd *display = gg->current_display;

  if (display == NULL) {
    if (g_slist_length (gg->d) > 0) {
      d = datad_get_from_notebook (gg->varpanel_ui.notebook, gg);
      /* if the circles are showing, hide them */
      if (varpanel_shows_circles (d)) {
        varcircles_show (false, d, display, gg);
      }
    }
  }
  else {    /*-- if there is a display present --*/
    d = display->d;
    highd = varpanel_highd (display);

    if (highd && varpanel_shows_checkboxes (d)) {
      varcircles_show (true, d, display, gg);
    }
    else if (!highd && varpanel_shows_circles (d)) {
      varcircles_show (false, d, display, gg);
    }
  }
}

void
rebuild_mode_menus (displayd * display, ggobid * gg)
{
  static const gchar *iprefix = "/menubar/IMode/", *pprefix =
    "/menubar/PMode/";
  gchar *path;
  GtkAction *action = NULL;


  if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
    gtk_ui_manager_remove_ui (gg->main_menu_manager, gg->mode_merge_id);
    /* Allow the extended display to override the submenu_destroy call.
       If it doesn't provide a method, then call submenu_destroy. */
    void (*f) (displayd * dpy) =
      GGOBI_EXTENDED_DISPLAY_GET_CLASS (display)->display_unset;
    if (f) {
      f (display);
      f (display);
    }                           /*
                                   else { 
                                   if (gg->pmode_item)
                                   submenu_destroy (gg->pmode_item);
                                   submenu_destroy (gg->imode_item);
                                   } */
  }

  /* Then rebuild */
  if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
    const gchar *(*ui_get) (displayd * dpy) =
      GGOBI_EXTENDED_DISPLAY_GET_CLASS (display)->mode_ui_get;
    if (ui_get) {
      GError *error = NULL;
      const gchar *ui = ui_get (display);
      gg->mode_merge_id =
        gtk_ui_manager_add_ui_from_string (gg->main_menu_manager, ui, -1,
                                           &error);
      if (error) {
        g_message ("Could not merge main mode ui from display");
        g_error_free (error);
      }
    }
    void (*f) (displayd * dpy, ggobid * gg) =
      GGOBI_EXTENDED_DISPLAY_GET_CLASS (display)->display_set;
    if (f)
      f (display, gg);

    /* use an informative label for the default actions, if necessary */
    path = g_strdup_printf ("%s%s", pprefix, "ExtendedDisplayPMode");
    action = gtk_ui_manager_get_action (gg->main_menu_manager, path);
    if (action)
      g_object_set (G_OBJECT (action), "label",
                    GGOBI (getPModeScreenName) (EXTENDED_DISPLAY_PMODE,
                                                display), NULL);
    g_free (path);
    path = g_strdup_printf ("%s%s", iprefix, "DefaultIMode");
    action = gtk_ui_manager_get_action (gg->main_menu_manager, path);
    if (action)
      g_object_set (G_OBJECT (action), "label",
                    GGOBI (getIModeScreenName) (DEFAULT_IMODE, display),
                    NULL);
    g_free (path);
    /* force the radio actions to update */
    path = g_strdup_printf ("%s%s", pprefix,
                            GGOBI (getPModeName) (pmode_get
                                                  (gg->current_display, gg)));
    action = gtk_ui_manager_get_action (gg->main_menu_manager, path);
    if (action)
      gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), true);
    g_free (path);
    path =
      g_strdup_printf ("%s%s", iprefix,
                       GGOBI (getIModeName) (imode_get (gg)));
    action = gtk_ui_manager_get_action (gg->main_menu_manager, path);
    if (action)
      gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), true);
    g_free (path);
  }

}

void
viewmode_set (ProjectionMode pmode, InteractionMode imode, ggobid * gg)
{
  displayd *display = gg->current_display;

  if (pmode != NULL_PMODE) {
    gg->pmode_prev = gg->pmode;
    gg->pmode = pmode;
  }
  if (imode != NULL_IMODE) {
    gg->imode_prev = gg->imode;
    gg->imode = imode;
  }
  else {
    gg->imode_prev = gg->imode;
    gg->imode = DEFAULT_IMODE;
  }

  /* Experiment -- seems to work */
  rebuild_mode_menus (display, gg);

  if (gg->pmode != NULL_PMODE && gg->pmode != gg->pmode_prev) {
    /* 
     * If moving between modes whose variable selection interface
     * differs, swap in the correct display.
     */
    varpanel_reinit (gg);
    varpanel_tooltips_set (display, gg);
    varpanel_refresh (display, gg);
  }

/*
 * Just sets up the mode_frame and the variable selection panel.
 * Assume that it's always necessary.
*/
  if (gg->current_control_panel) {
    GtkWidget *modeBox = gg->current_control_panel;
    if (modeBox) {
      gtk_widget_ref (modeBox);
      gtk_container_remove (GTK_CONTAINER (gg->imode_frame), modeBox);
      gg->current_control_panel = NULL;
    }
  }

  if (imode != NULL_IMODE) {
    gchar *modeName = NULL;
    GtkWidget *panel = NULL;

    /* a change within the set of imodes */
    if (imode > DEFAULT_IMODE && imode < EXTENDED_DISPLAY_IMODE) {
      modeName = (gchar *) imode_name[imode]; /* could be DEFAULT */
      panel = mode_panel_get_by_name (modeName, gg);
    }

    /* the pmode is taking over the control panel */
    else if (imode == DEFAULT_IMODE && gg->pmode > NULL_PMODE) {
      if (gg->pmode == EXTENDED_DISPLAY_PMODE) {
        GGobiExtendedDisplayClass *klass;
        if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
          klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
          panel = klass->imode_control_box (display, &modeName, gg);
        }
      }
      else if (pmode < EXTENDED_DISPLAY_PMODE) {  /* scatterplot? */
        modeName = (gchar *) pmode_name[gg->pmode];
        panel = mode_panel_get_by_name (modeName, gg);
      }
    }

    gtk_frame_set_label (GTK_FRAME (gg->imode_frame), modeName);
    gtk_container_add (GTK_CONTAINER (gg->imode_frame), panel);
    gg->current_control_panel = panel;

    /*-- avoid increasing the object's ref_count infinitely  --*/
    /* wow, super-dangerous - do we really need this? - mfl */
    /*if (G_OBJECT (panel)->ref_count > 1)
      gtk_widget_unref (panel);*/
  }

  if (pmode != NULL_PMODE && gg->pmode != gg->pmode_prev) {

    /*
     * The projection type is one of P1PLOT, XYPLOT, ROTATE, TOUR1D,
     * TOUR2D or COTOUR.  It only changes if another projection type
     * is selected.  (For parcoords, time series, and scatmat plots,
     * the value of projection is irrelevant.  A second pmode is in
     * the process of being added to the barchart.)
     */
    if (display && GGOBI_IS_EXTENDED_DISPLAY (display)) {
      GGobiExtendedDisplayClass *klass;
      klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
      if (klass->pmode_set)
        klass->pmode_set (pmode, display, gg);
    }
  }
}

/*
 * Turn the tour procs on and off here
*/
void
procs_activate (gboolean state, ProjectionMode pmode, displayd * display,
                ggobid * gg)
{
  if (!display)
    return;

  switch (pmode) {
  case TOUR1D:
    if (!display->cpanel.t1d.paused)
      tour1d_func (state, display, gg);
    break;
  case TOUR2D3:
    if (!display->cpanel.t2d3.paused)
      tour2d3_func (state, display, gg);
    break;
  case TOUR2D:
    if (!display->cpanel.t2d.paused)
      tour2d_func (state, display, gg);
    break;
  case COTOUR:
    if (!display->cpanel.tcorr1.paused)
      tourcorr_func (state, display, gg);
    break;
  default:
    break;
  }
}

RedrawStyle
imode_activate (splotd * sp, ProjectionMode pmode, InteractionMode imode,
                gboolean state, ggobid * gg)
{
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  RedrawStyle redraw_style = NONE;

  if (state == off) {
    switch (imode) {
    case DEFAULT_IMODE:
      switch (pmode) {
      case P1PLOT:
        p1d_activate (state, display, gg);
        break;
      case XYPLOT:
        xyplot_activate (state, display, gg);
        break;
      case TOUR2D3:
        if (cpanel->t2d3.manip_mode != MANIP_OFF)
          splot_cursor_set ((gint) NULL, sp);
        break;
      case TOUR2D:
        if (cpanel->t2d.manip_mode != MANIP_OFF)
          splot_cursor_set ((gint) NULL, sp);
        break;
      case COTOUR:
        if (cpanel->tcorr.manip_mode != MANIP_OFF)
          splot_cursor_set ((gint) NULL, sp);
        break;
      case TOUR1D:
      case NULL_PMODE:
      case DEFAULT_PMODE:
      case EXTENDED_DISPLAY_PMODE: /* Each class needs its own one
                                       of these */
      case N_PMODES:
        break;
      }
      break;
    case BRUSH:
      redraw_style = brush_activate (state, display, sp, gg);
      break;
    case IDENT:
      redraw_style = identify_activate (state, display, gg);
      break;
    case SCALE:
      splot_cursor_set ((gint) NULL, sp);
        /*-- for insurance, because sometimes scaling doesn't quit --*/
      disconnect_motion_signal (sp);
        /*-- --*/
      break;
    case EDGEED:
      redraw_style = edgeedit_activate (state, display, gg);
      break;
    default:
      break;
    }
  }
  else if (state == on) {
    switch (imode) {
    case DEFAULT_IMODE:
      switch (pmode) {
      case P1PLOT:
        p1d_activate (state, display, gg);
        break;
      case XYPLOT:
        xyplot_activate (state, display, gg);
        break;
      case TOUR2D3:
        if (cpanel->t2d3.manip_mode != MANIP_OFF)
          splot_cursor_set (GDK_HAND2, sp);
        break;
      case TOUR2D:
        if (cpanel->t2d.manip_mode != MANIP_OFF)
          splot_cursor_set (GDK_HAND2, sp);
        break;
      case COTOUR:
        if (cpanel->tcorr.manip_mode != MANIP_OFF)
          splot_cursor_set (GDK_HAND2, sp);
        break;
      case TOUR1D:
      case NULL_PMODE:
      case DEFAULT_PMODE:
      case EXTENDED_DISPLAY_PMODE:
      case N_PMODES:
        break;
      }
      break;
    case BRUSH:
      redraw_style = brush_activate (state, display, sp, gg);
      break;
    case IDENT:
      redraw_style = identify_activate (state, display, gg);
      break;
    case SCALE:
      splot_cursor_set (GDK_HAND2, sp);
      break;
    case EDGEED:
      redraw_style = edgeedit_activate (state, display, gg);
      break;
    default:
      break;
    }
  }
  return redraw_style;
}

/*
 * Verify that the number of variables is large enough before
 * allowing the projection to be reset.
*/
gboolean
projection_ok (ProjectionMode m, displayd * display)
{
  gboolean ok = true;
  GGobiData *d = display->d;

  /*-- if the mode is a projection-setting mode ... --*/
  if (m <= COTOUR) {
    switch (m) {
    case P1PLOT:
      if (d->ncols < 1)
        ok = false;
      break;
    case XYPLOT:
      if (d->ncols < 2)
        ok = false;
      break;
    case TOUR2D3:
      if (d->ncols < MIN_NVARS_FOR_TOUR2D3)
        ok = false;
      break;
    case TOUR1D:
      if (d->ncols < MIN_NVARS_FOR_TOUR1D)
        ok = false;
      break;
    case TOUR2D:
      if (d->ncols < MIN_NVARS_FOR_TOUR2D)
        ok = false;
      break;
    case COTOUR:
      if (d->ncols < MIN_NVARS_FOR_COTOUR)
        ok = false;
      break;
    default:
      break;
    }
  }
  return ok;
}

/* Do everything in one routine for now; split later if apropriate */
gint
GGOBI (full_viewmode_set) (ProjectionMode pmode, InteractionMode imode,
                           ggobid * gg)
{
  /*
   * When a new pmode is selected, it sets a new pmode and then calls
   * this routine, because a new imode is also selected.
   */

/*
 * Some of the routines called here, like procs_activate
 * and reinit_transient brushing, are routines that we want
 * to have executed when a new viewmode is selected for the
 * current display, but not when the viewmode changes because
 * a new display becomes current.
 * Because of that, we don't put them in viewmode_activate.
*/
  gboolean reinit_transient_p = false;
  gboolean cpanel_shows_pmode = (imode == DEFAULT_IMODE);

  if (gg->current_display != NULL && gg->current_splot != NULL) {
    splotd *sp = gg->current_splot;
    displayd *display = gg->current_display;
    cpaneld *cpanel = &display->cpanel;
    RedrawStyle redraw_style = NONE;

    /* Shutting off event handlers and idle procs */
    /* It may make sense to split the event handlers into two routines
     */
    /* Each display class may need one of these */
    sp_event_handlers_toggle (sp, off, gg->pmode, gg->imode);
    redraw_style = imode_activate (sp, gg->pmode, gg->imode, off, gg);
    procs_activate (off, gg->pmode, display, gg);

    /* UI part and resetting the variables */
    if (pmode != NULL_PMODE)
      display->cpanel.pmode = pmode;
    display->cpanel.imode = imode;
    viewmode_set (pmode, imode, gg);
    /* */

    sp_event_handlers_toggle (sp, on, gg->pmode, gg->imode);
    imode_activate (sp, gg->pmode, gg->imode, on, gg);
    if (cpanel_shows_pmode)
      procs_activate (on, gg->pmode, display, gg);

    if (gg->imode != BRUSH && gg->imode_prev == BRUSH) {
      if (cpanel->br.mode == BR_TRANSIENT) {
        reinit_transient_p = true;
        reinit_transient_brushing (display, gg);
      }
    }

    /*
     * work out which menus (Options, Reset, I/O) need to be present
     * on the main menubar and the display menubar.
     */
    /*main_miscmenus_update (gg->pmode_prev, gg->imode_prev, display, gg); */
    display_mode_menus_update (gg->pmode_prev, gg->imode_prev, display, gg);

    /*-- redraw this display --*/
    display_tailpipe (display, FULL, gg);

    /*-- redraw as needed for transient brushing and identify --*/
    if (redraw_style != NONE || reinit_transient_p) {
      displays_plot (sp, FULL, gg);
    }

    return (gg->imode);

  }
  else {                        /* if there's no display */
    viewmode_set (NULL_PMODE, NULL_IMODE, gg);
    /*-- need to remove console menus: Options, Reset, ... --*/
    /*main_miscmenus_update (gg->pmode_prev, gg->imode_prev, NULL, gg); */
    if (gg->mode_merge_id)
      gtk_ui_manager_remove_ui (gg->main_menu_manager, gg->mode_merge_id);
    //submenu_destroy (gg->imode_item);

    return (NULL_IMODE);
  }

  return (-1);
}

/*
#ifndef AS_GGOBI_LIBRARY
*/
/*
  Wrapper for gtk_main_quit so that we can override this in
  other applications to avoid quitting when the user selects
  the Quit button.
 */
void
quit_ggobi (ggobid * gg)
{
  extern void closePlugins (ggobid * gg);
  gint n, i;
  ggobid *el;
  n = GGobi_getNumGGobis ();
  for (i = 0; i < n; i++) {
    el = GGobi_ggobi_get (i);
    if (el != gg)
      closePlugins (el);
  }
  closePlugins (gg);

  procs_activate (off, gg->pmode, gg->current_display, gg);
  gtk_main_quit ();
}

/* action callbacks */
static void
action_open_cb (GtkAction * action, ggobid * gg)
{
  filename_get_r (gg);
}
static void
action_new_cb (GtkAction * action, ggobid * gg)
{
  create_new_ggobi ();
}
static void
action_save_cb (GtkAction * action, ggobid * gg)
{
  writeall_window_open (gg);
}
#ifdef STORE_SESSION_ENABLED
static void
action_store_session_cb (GtkAction * action, ggobid * gg)
{
  store_session (gg);
}
#endif
static void
action_close_cb (GtkAction * action, ggobid * gg)
{
  ggobi_close (gg);
}
static void                     /* this is to connect to the delete signal (window closed) */
signal_delete_cb (ggobid * gg, GdkEvent * ev, GtkWidget * w)
{
  ggobi_close (gg);
}
static void
action_quit_cb (GtkAction * action, ggobid * gg)
{
  quit_ggobi (gg);
}
static void
action_manipulate_cb (GtkAction * action, ggobid * gg)
{
  vartable_open (gg);
}
static void
action_transform_cb (GtkAction * action, ggobid * gg)
{
  transform_window_open (gg);
}
static void
action_sphere_cb (GtkAction * action, ggobid * gg)
{
  sphere_panel_open (gg);
}
static void
action_jitter_cb (GtkAction * action, ggobid * gg)
{
  jitter_window_open (gg);
}
static void
action_color_schemes_cb (GtkAction * action, ggobid * gg)
{
  svis_window_open (gg);
}
static void
action_autobrush_cb (GtkAction * action, ggobid * gg)
{
  wvis_window_open (gg);
}
static void
action_color_glyph_groups_cb (GtkAction * action, ggobid * gg)
{
  cluster_window_open (gg);
}
static void
action_subset_cb (GtkAction * action, ggobid * gg)
{
  subset_window_open (gg);
}

#ifdef SMOOTH_IMPLEMENTED
static void
action_smooth_cb (GtkAction * action, ggobid * gg)
{
  smooth_window_open (gg);
}
#endif
static void
action_impute_cb (GtkAction * action, ggobid * gg)
{
  impute_window_open (gg);
}
static void
action_about_cb (GtkAction * action, ggobid * gg)
{
  splash_show (gg);
}
static void
action_plugins_cb (GtkAction * action, ggobid * gg)
{
  show_plugin_list (gg);
}
static void
action_toggle_tooltips_cb (GtkToggleAction * action, ggobid * gg)
{
  tooltips_show (gtk_toggle_action_get_active (action), gg);
}
static void
action_toggle_cpanel_cb (GtkToggleAction * action, ggobid * gg)
{
  cpanel_show (gtk_toggle_action_get_active (action), gg);
}
static void
action_toggle_statusbar_cb (GtkToggleAction * action, ggobid * gg)
{
  statusbar_show (gtk_toggle_action_get_active (action), gg);
}

static void
action_radio_pmode_cb (GtkRadioAction * action, GtkRadioAction * current,
                       ggobid * gg)
{
  ProjectionMode pm =
    (ProjectionMode) gtk_radio_action_get_current_value (action);

  /* I don't know why this other test used to be necessary when
     it doesn't seem to be any more, but I know it does great harm
     when I'm using radio buttons ... dfs */

  if ((pm != gg->pmode /*|| gg->imode != DEFAULT_IMODE */ ) &&
      projection_ok (pm, gg->current_display)) {
    /* When the pmode is reset, the imode is set to the default */
    GGOBI (full_viewmode_set) (pm, DEFAULT_IMODE, gg);
  }
}
static void
action_radio_imode_cb (GtkRadioAction * action, GtkRadioAction * current,
                       ggobid * gg)
{
  InteractionMode im;

  im = (InteractionMode) gtk_radio_action_get_current_value (action);
  if (im != gg->imode) {
    GGOBI (full_viewmode_set) (NULL_PMODE, im, gg);
  }
}

static const gchar *main_ui_str =
  "<ui>"
  "	<menubar>"
  "		<menu action='File'>"
  "			<menuitem action='Open'/>"
  "			<menuitem action='New'/>"
  "			<menuitem action='Save'/>" "			<menu action='Shortcuts'/>"
#ifdef STORE_SESSION_ENABLED
  "			<separator/>" "			<menuitem action='StoreSession'/>"
#endif
  "			<separator/>"
  "			<menu action='Options'>"
  "				<menuitem action='ShowTooltips'/>"
  "				<menuitem action='ShowControlPanel'/>"
  "				<menuitem action='ShowStatusbar'/>"
  "			</menu>"
  "			<separator/>"
  "			<menuitem action='Close'/>"
  "			<menuitem action='Quit'/>"
  "		</menu>"
  "		<menu action='Display'/>"
  "		<menu action='PMode'/>"
  "		<menu action='IMode'/>"
  "		<menu action='Tools'>"
  "			<menuitem action='VariableManipulation'/>"
  "			<menuitem action='VariableTransformation'/>"
  "			<menuitem action='Sphering'/>"
#ifdef INFERENCE_IMPLEMENTED
  "			<menuitem action='Inference'/>"
#endif
  "			<menuitem action='VariableJittering'/>"
  "			<separator/>"
  "			<menuitem action='ColorSchemes'/>"
  "			<menuitem action='AutoBrushing'/>"
  "			<menuitem action='ColorAndGlyphGroups'/>"
  "			<separator/>" "			<menuitem action='CaseSubsettingAndSampling'/>"
#ifdef SMOOTH_IMPLEMENTED
  "			<menuitem action='Smooth'/>"
#endif
  "			<menuitem action='MissingValues'/>"
  "			<separator/>"
  "		</menu>"
  "		<menu action='Help'>"
  "			<menuitem action='AboutGGobi'/>"
  "			<menuitem action='AboutPlugins'/>"
  "		</menu>" "	</menubar>" "</ui>";

static GtkActionEntry entries[] = {
  {"File", NULL, "_File"},
  {"Open", GTK_STOCK_OPEN, "_Open", NULL, "Open a datafile",
   G_CALLBACK (action_open_cb)},
  {"New", GTK_STOCK_NEW, "_New", NULL, "Create a new GGobi instance",
   G_CALLBACK (action_new_cb)},
  {"Save", GTK_STOCK_SAVE, "_Save", "<control>V", "Save some data",
   G_CALLBACK (action_save_cb)},
  {"Shortcuts", NULL, "Shortc_uts"},
#ifdef STORE_SESSION_ENABLED
  {"StoreSession", GTK_STOCK_GOTO_BOTTOM, "Store Session", NULL,
   "Save this GGobi session",
   G_CALLBACK (action_store_session_cb)
   },
#endif
  {"Close", GTK_STOCK_CLOSE, "_Close", "<control>C",
   "Close this GGobi instance", G_CALLBACK (action_close_cb)},
  {"Quit", GTK_STOCK_QUIT, "_Quit", "<control>Q", "Quit GGobi",
   G_CALLBACK (action_quit_cb)},

  {"Display", NULL, "_Display"},
  {"PMode", NULL, "_View"},
  {"IMode", NULL, "_Interaction"},
  {"Options", NULL, "_Options"},

  {"Tools", NULL, "_Tools"},
  {"VariableManipulation", GTK_STOCK_INDEX, "Variable _Manipulation", NULL,
   "Open a table of variables for manipulation",
   G_CALLBACK (action_manipulate_cb)
   },
  {"VariableTransformation", GTK_STOCK_CONVERT, "Variable _Transformation",
   NULL,
   "Perform transformations on the dataset's variables",
   G_CALLBACK (action_transform_cb)
   },
  {"Sphering", GTK_STOCK_JUMP_TO, "_Sphering (PCA)", NULL,
   "Open a panel to perform sphering",
   G_CALLBACK (action_sphere_cb)
   },
#ifdef INFERENCE_IMPLEMENTED    /* to do */
  {"Inference", GTK_STOCK_EXECUTE, "_Inference", NULL, "Perform inference",
   NULL},
#endif
  {"VariableJittering", NULL, "Variable _Jittering", NULL,
   "'Jitter' some variables",
   G_CALLBACK (action_jitter_cb)
   },
  {"ColorSchemes", GTK_STOCK_SELECT_COLOR, "_Color Schemes", NULL,
   "Configure and pick color schemes",
   G_CALLBACK (action_color_schemes_cb)
   },
  {"AutoBrushing", NULL, "_Automatic Brushing", NULL,
   "Apply color scheme along a variable",
   G_CALLBACK (action_autobrush_cb)
   },
  {"ColorAndGlyphGroups", NULL, "Color & _Glyph Groups", NULL,
   "Configure color and glyph groups",
   G_CALLBACK (action_color_glyph_groups_cb)
   },
  {"CaseSubsettingAndSampling", NULL, "Case S_ubsetting and Sampling", NULL,
   "Extract and resample subsets of cases", G_CALLBACK (action_subset_cb)
   },
#ifdef SMOOTH_IMPLEMENTED
  {"Smoothing", NULL, "Sm_oothing", NULL, "Smooth the data",
   G_CALLBACK (action_smooth_cb)},
#endif
  {"MissingValues", NULL, "Missing _Values", NULL, "Impute missing values",
   G_CALLBACK (action_impute_cb)
   },

  {"Help", NULL, "_Help"},
  {"AboutGGobi", NULL, "About _GGobi", NULL,
   "Discover the magic behind GGobi",
   G_CALLBACK (action_about_cb)
   },
  {"AboutPlugins", NULL, "About _Plugins", NULL, "Current plugin status",
   G_CALLBACK (action_plugins_cb)
   }
};

static GtkRadioActionEntry pmode_entries[] = {
  /* here is where the i/p mode stuff goes */
  {"ExtendedDisplayPMode", NULL, "Default", "<control>H",
   /* assumes 'extended display pmode' is 'default' */
   "Switch to the default view mode for this display",
   EXTENDED_DISPLAY_PMODE},
  {"1D Plot", NULL, "1_D Plot", "<control>D",
   "View a 1D plot of the data", P1PLOT},
  {"XY Plot", NULL, "_XY Plot", "<control>X",
   "View a 2D plot of the data", XYPLOT},
  {"1D Tour", NULL, "1D _Tour", "<control>T",
   "Tour the data in a single dimension", TOUR1D},
  {"Rotation", NULL, "_Rotation", "<control>R",
   "Tour the data in two dimensions, three variables at a time", TOUR2D3},
  {"2D Tour", NULL, "2D To_ur", "<control>G",
   "Take a grand tour of the data", TOUR2D},
  {"2x1D Tour", NULL, "2x1D T_our", "<control>U",
   "Take a 2x1D (correlation) tour of the data", COTOUR}
};

static GtkRadioActionEntry imode_entries[] = {
  {"DefaultIMode", NULL, "Default", "<control>H",
   /* assumes 'extended display pmode' is 'default' */
   "Switch to the default interaction mode for this view mode",
   DEFAULT_IMODE},
  {"Scale", NULL, "_Scale", "<control>S",
   "Scale (pan and zoom) the data", SCALE},
  {"Brush", NULL, "_Brush", "<control>B",
   "Brush (color) points", BRUSH},
  {"Identify", NULL, "_Identify", "<control>I",
   "Identify points (query their values)", IDENT},
  {"Edit Edges", NULL, "_Edit Edges", "<control>E",
   "Edit the edges in the plot", EDGEED},
  {"Move Points", NULL, "_Move Points", "<control>M",
   "Move the points in the plot", MOVEPTS}
};

GtkActionGroup *
ggobi_actions_create (ggobid * gg)
{
  GtkToggleActionEntry t_entries[] = {  /* not global because depends on gg state */
    {"ShowTooltips", NULL, "Show _Tooltips", NULL,
     "Toggle display of helpful tips like this one",
     G_CALLBACK (action_toggle_tooltips_cb),
     GTK_TOOLTIPS (gg->tips)->enabled},
    {"ShowControlPanel", NULL, "Show _Control Panel", NULL,
     "Toggle display of control panel",
     G_CALLBACK (action_toggle_cpanel_cb), true},
    {"ShowStatusbar", NULL, "Show _Statusbar", NULL,
     "Toggle display of statusbar at bottom",
     G_CALLBACK (action_toggle_statusbar_cb), gg->statusbar_p}
  };

  GtkActionGroup *actions = gtk_action_group_new ("GGobiActions");
  gtk_action_group_add_actions (actions, entries, G_N_ELEMENTS (entries), gg);
  gtk_action_group_add_toggle_actions (actions, t_entries,
                                       G_N_ELEMENTS (t_entries), gg);
  gtk_action_group_add_radio_actions (actions, pmode_entries,
                                      G_N_ELEMENTS (pmode_entries),
                                      EXTENDED_DISPLAY_PMODE,
                                      G_CALLBACK (action_radio_pmode_cb), gg);
  gtk_action_group_add_radio_actions (actions, imode_entries,
                                      G_N_ELEMENTS (imode_entries),
                                      DEFAULT_IMODE,
                                      G_CALLBACK (action_radio_imode_cb), gg);

  g_object_set (G_OBJECT (gtk_action_group_get_action (actions, "Display")),
                "hide_if_empty", false, NULL);
  /*g_object_set(G_OBJECT(gtk_action_group_get_action(actions, "Shortcuts")), 
     "hide_if_empty", false, NULL); */

  return (actions);
}

GtkUIManager *
ggobi_menu_manager_create (ggobid * gg)
{
  GtkUIManager *manager = gtk_ui_manager_new ();
  GtkActionGroup *actions = ggobi_actions_create (gg);
  gtk_ui_manager_insert_action_group (manager, actions, 0);
  gtk_ui_manager_set_add_tearoffs (manager, true);
  g_object_unref (G_OBJECT (actions));
  return (manager);
}

void
make_ui (ggobid * gg)
{
  GtkWidget *window;
  GtkWidget *hbox, *vbox, *statusbar;
  GtkWidget *basement;

  gg->tips = gtk_tooltips_new ();

  gg->main_window = window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "GGobi");
  gtk_window_set_policy (GTK_WINDOW (window), true, true, false);
  gtk_window_set_default_size (GTK_WINDOW (window), 400, 500);

  GGobi_widget_set (window, gg, true);

#ifdef TEST_GGOBI_EVENTS
/*  g_signal_connect (G_OBJECT(gg), "splot_new", test_new_plot_cb, (gpointer) "A new plot"); */
  g_signal_connect_swapped (G_OBJECT (gg), "splot_new", test_new_plot_cb,
                            (gpointer) "A new plot");
  g_signal_connect (G_OBJECT (gg), "datad_added", test_data_add_cb, NULL);
  g_signal_connect (G_OBJECT (gg), "sticky_point_added", test_sticky_points,
                    NULL);
  g_signal_connect (G_OBJECT (gg), "sticky_point_removed", test_sticky_points,
                    NULL);
#endif


  g_signal_connect_swapped (G_OBJECT (window), "delete_event",
                            G_CALLBACK (signal_delete_cb), (gpointer) gg);
  g_signal_connect_swapped (G_OBJECT (window), "destroy_event",
                            G_CALLBACK (signal_delete_cb), (gpointer) gg);

  //gtk_container_set_border_width (GTK_CONTAINER (window), 10);

/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  gg->main_menu_manager = ggobi_menu_manager_create (gg);
  gg->main_menubar =
    create_menu_bar (gg->main_menu_manager, main_ui_str, window);
  gg->main_accel_group =
    gtk_ui_manager_get_accel_group (gg->main_menu_manager);

  if (sessionOptions->info && sessionOptions->info->numInputs > 0) {
    addPreviousFilesMenu (sessionOptions->info, gg);
  }

  display_menu_init (gg);

  gtk_box_pack_start (GTK_BOX (vbox), gg->main_menubar, false, false, 0);

  gtk_accel_group_lock (gg->main_accel_group);

  hbox = gtk_hbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, true, true, 0);

/*
 * Create a frame to hold the mode panels, set its label
 * and contents, using the default mode for the default display.
*/
  gg->imode_frame = gtk_frame_new ((gg->imode == NULL_IMODE)
                                   ? "" : imode_name[gg->imode]);

  gtk_box_pack_start (GTK_BOX (hbox), gg->imode_frame, false, false, 3);
  gtk_container_set_border_width (GTK_CONTAINER (gg->imode_frame), 2);
  gtk_frame_set_shadow_type (GTK_FRAME (gg->imode_frame), GTK_SHADOW_NONE);

  g_signal_connect (G_OBJECT (gg), "display_selected",
                    G_CALLBACK (control_panel_display_selected_cb), NULL);

  make_control_panels (gg);
  if (gg->imode != NULL_IMODE) {
    if (gg->imode == DEFAULT_IMODE)
      gtk_container_add (GTK_CONTAINER (gg->imode_frame),
                         mode_panel_get_by_name ((gchar *)
                                                 pmode_name[gg->pmode], gg));
    else
      gtk_container_add (GTK_CONTAINER (gg->imode_frame),
                         mode_panel_get_by_name ((gchar *)
                                                 imode_name[gg->imode], gg));
  }

  gtk_box_pack_start (GTK_BOX (hbox), gtk_vseparator_new (), false, false, 2);

  /*-- Variable selection panel --*/
  varpanel_make (hbox, gg);

  /*-- status bar --*/
  statusbar = gtk_statusbar_new ();
  g_object_set_data (G_OBJECT (gg->main_window), "MAIN:STATUSBAR", statusbar);
  gtk_box_pack_start (GTK_BOX (vbox), statusbar, false, false, 0);
  /*--            --*/

  gtk_widget_show_all (hbox);

  /* -- do not map or show this widget -- */
  basement = gtk_vbox_new (false, 0);
  gtk_widget_set_name (basement, "BASEMENT");
  gtk_box_pack_start (GTK_BOX (hbox), basement, false, false, 0);
  /* -- do not map or show this widget -- */


  /*-- at this point, the mode could be NULLMODE, P1PLOT, or XYPLOT --*/
  {
    void main_miscmenus_initialize (ggobid * gg);
    /*main_miscmenus_initialize (gg); */
  }

  if (sessionOptions->showControlPanel)
    gtk_widget_show_all (window);
}

const gchar *const *GGOBI (getPModeNames) (int *n)
{
  *n = sizeof (GGOBI (PModeNames)) / sizeof (GGOBI (PModeNames)[0]);
  return (GGOBI (PModeNames));
}
const gchar *const *GGOBI (getIModeNames) (int *n)
{
  *n = sizeof (GGOBI (IModeNames)) / sizeof (GGOBI (IModeNames)[0]);
  return (GGOBI (IModeNames));
}

const gchar *const *GGOBI (getPModeKeys) (int *n)
{
  *n = sizeof (GGOBI (PModeKeys)) / sizeof (GGOBI (PModeKeys)[0]);
  return (GGOBI (PModeKeys));
}



void load_previous_file (GtkAction * action, gpointer cbd);
/*
  Add the previous input sources to the menu.
 */
void
addPreviousFilesMenu (GGobiInitInfo * info, ggobid * gg)
{
  gint i;
  InputDescription *input;
  if (info) {
    GtkUIManager *manager = gg->main_menu_manager;
    GtkActionGroup *actions = gtk_action_group_new ("Shortcuts");
    guint merge_id = gtk_ui_manager_new_merge_id (manager);
    gtk_ui_manager_insert_action_group (manager, actions, -1);
    for (i = 0; i < info->numInputs; i++) {
      input = &(info->descriptions[i].input);
      if (input && input->fileName) {
        gchar *action_name = g_strdup_printf ("Shortcut_%d", i);
        GtkAction *action = gtk_action_new (action_name, input->fileName,
                                            "Open this shortcut",
#if GTK_MAJOR_VERSION > 2 && GTK_MINOR_VERSION > 4
                                            GTK_STOCK_FILE);
#else
                                            NULL);
#endif
        g_signal_connect (G_OBJECT (action), "activate",
                          G_CALLBACK (load_previous_file),
                          info->descriptions + i);
        g_object_set_data (G_OBJECT (action), "ggobi", gg);
        gtk_action_group_add_action (actions, action);
        gtk_ui_manager_add_ui (manager, merge_id, "/menubar/File/Shortcuts",
                               action_name, action_name,
                               GTK_UI_MANAGER_MENUITEM, false);
        g_free (action_name);
        g_object_unref (action);
      }
    }
    g_object_unref (actions);
  }
}


ggobid *create_ggobi (InputDescription * desc);

void
load_previous_file (GtkAction * action, gpointer cbd)
{
  InputDescription *desc;
  GGobiDescription *gdesc;
  ggobid *gg;

  gg = (ggobid *) g_object_get_data (G_OBJECT (action), "ggobi");
  gdesc = (GGobiDescription *) cbd;
  desc = &(gdesc->input);

  if (g_slist_length (gg->d) > 0)
    create_ggobi (desc);
  else {
    read_input (desc, gg);
    /* Need to avoid the initial scatterplot. */
    start_ggobi (gg, true, gdesc->displays == NULL);
  }


  if (gdesc->displays) {
    gint i, n;
    GGobiDisplayDescription *dpy;
    n = g_list_length (gdesc->displays);
    for (i = 0; i < n; i++) {
      dpy = (GGobiDisplayDescription *) g_list_nth_data (gdesc->displays, i);
      createDisplayFromDescription (gg, dpy);
      /*
       * This line is added to counteract something done in
       * display_add:  if there's a previous splot, display_add
       * kindly arranges for it to get a QUICK redraw just to
       * eliminate the border.  The API, though, allows many
       * plots to be added before anything is drawn.  As a
       * result, if the first display is a parcoords plot, the
       * first splot is copied from pixmap0 to pixmap1 before it
       * has been drawn to pixmap1, resulting in garbage on the
       * screen.
       */
      gg->current_splot = NULL;
    }
  }
}

/*
 This replicates code elsewhere and the two should be merged.
 */
ggobid *
create_ggobi (InputDescription * desc)
{
  gboolean init_data = true;
  ggobid *gg;

  gg = ggobi_alloc (NULL);

     /*-- some initializations --*/
  gg->displays = NULL;
  gg->control_panels = NULL;
  globals_init (gg);      /*-- variables that don't depend on the data --*/
  special_colors_init (gg);
  make_ui (gg);
  gg->input = desc;

  read_input (desc, gg);

  if (sessionOptions->info != NULL) {
    extern gboolean registerPlugins (ggobid * gg, GList * plugins);
    registerPlugins (gg, sessionOptions->info->plugins);
  }

  start_ggobi (gg, init_data, sessionOptions->info->createInitialScatterPlot);

  return (gg);
}


void
show_plugin_list (ggobid * gg)
{
  if (sessionOptions->info && sessionOptions->info->plugins)
    showPluginInfo (sessionOptions->info->plugins,
                    sessionOptions->info->inputPlugins, (ggobid *) gg);
}


#ifdef STORE_SESSION_ENABLED
void
store_session_in_file (GtkWidget * chooser)
{
  gchar *fileName;
  ggobid *gg;

  fileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
  if (fileName && fileName[0]) {
    gg = g_object_get_data (G_OBJECT (chooser), "ggobi");
    write_ggobi_as_xml (gg, fileName, NULL);
    g_free (fileName);
  }
}

void
store_session (ggobid * gg)
{
  GtkWidget *dlg;
  gchar *buf;

  if (!sessionOptions->info->sessionFile) {
    buf =
      g_strdup_printf ("%s%c%s", getenv ("HOME"), G_DIR_SEPARATOR,
                       ".ggobi-session");
    dlg =
      gtk_file_chooser_dialog_new ("Save ggobi session", NULL,
                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                   GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                   GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                   NULL);
    g_object_set_data (G_OBJECT (dlg), "ggobi", (gpointer) gg);
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dlg), buf);
    g_free (buf);
    if (gtk_dialog_run (GTK_DIALOG (dlg)) == GTK_RESPONSE_ACCEPT)
      store_session_in_file (dlg);
    gtk_widget_destroy (dlg);
  }
  else {
    ggobi_write_session (sessionOptions->info->sessionFile);
    /* write_ggobi_as_xml(gg, sessionOptions->info->sessionFile); */
  }
}
#endif


void
create_new_ggobi ()
{
  create_ggobi (NULL);
}
