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

#include "write_state.h"

#ifdef SUPPORT_PLUGINS
#include "plugin.h"
#endif


#ifdef TEST_GGOBI_EVENTS
#include "testEvents.h"
#endif

#include "scatmatClass.h"

const char *const GGOBI(PModeNames)[] = {
  "Default",
  "1D Plot",
  "XY Plot",
  "1D Tour",
  "Rotation",
  "2D Tour",
  "Correlation Tour",
};
const char *const GGOBI(IModeNames)[] = {
  "Default",
  "Scale",
  "Brush",
  "Identify",
  "Edit edges",
  "Move points",
};
static const char *const *pmode_name = GGOBI(PModeNames);
static const char *const *imode_name = GGOBI(IModeNames);

const char * const GGOBI(PModeKeys)[] = {
  "", "d", "x", "t", "r", "g", "c", "", ""};
const char * const GGOBI(IModeKeys)[] = {
  "", "s", "b", "i", "e", "m", "", ""};

void addPreviousFilesMenu(GtkWidget *parent, GGobiInitInfo *info, ggobid *gg);

void store_session(ggobid *gg, gint action, GtkWidget *w);
void show_plugin_list(ggobid *gg, gint action, GtkWidget *w);
void create_new_ggobi(ggobid *gg, gint action, GtkWidget *w);

void
make_control_panels (ggobid *gg) 
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
GtkWidget * mode_panel_get_by_name(const gchar *name, ggobid *gg)
{
  GList *l;
  GtkWidget *w = NULL;
  modepaneld *pnl = (modepaneld *) l;

  for (l=gg->control_panels; l; l=l->next) {
    pnl = (modepaneld *) l->data;
    if (strcmp(name, pnl->name) == 0) {
      w = pnl->w;
      break;
    }
  }
  return (GtkWidget *) w;
}

void
tooltips_show_cb (GtkCheckMenuItem *w, guint action) 
{
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(w), true);
  if (w->active)
    gtk_tooltips_enable (gg->tips);
  else
    gtk_tooltips_disable (gg->tips);
}

void
statusbar_show (gboolean show, ggobid *gg)
{
  GtkWidget *entry = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(gg->main_window), "MAIN:STATUSBAR");
  if (show)
    gtk_widget_show (entry);
  else
    gtk_widget_hide (entry);
  gg->statusbar_p = show;
}
void
statusbar_show_cb (GtkCheckMenuItem *w, guint action) 
{
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(w), true);
  statusbar_show (w->active, gg);
}
/*
  gg->status_message_func((gchar *)domain_error_message, gg);
*/
void
gg_write_to_statusbar (gchar *message, ggobid *gg)
{
  GtkWidget *entry = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(gg->main_window), "MAIN:STATUSBAR");

  if (message)
    gtk_entry_set_text (GTK_ENTRY(entry), message);
  else {
    /*-- by default, describe the current datad --*/
    datad *d = datad_get_from_notebook (gg->varpanel_ui.notebook, gg);
    if (d) {
      gchar *msg = g_strdup_printf ("%s: %d x %d  (%s)",
        d->name, d->nrows, d->ncols, gg->input->fileName);
      gtk_entry_set_text (GTK_ENTRY(entry), msg);
      g_free (msg);
    }
  }
}

void
cpanel_show_cb (GtkCheckMenuItem *w, guint action) 
{
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(w), true);
  if (w->active)
    gtk_widget_show (gg->imode_frame);
  else
    gtk_widget_hide (gg->imode_frame);
}

ProjectionMode pmode_get (ggobid *gg) { return gg->pmode; }
InteractionMode imode_get (ggobid *gg) { return gg->imode; }

/*
 * Use the mode to determine whether the variable selection
 * panel should display checkboxes or circles
*/
static gboolean
varpanel_highd (displayd *display)
{
  gboolean highd = false;

  if(display && GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
     GtkGGobiExtendedDisplayClass *klass;
     klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(display));
     if(klass->varpanel_highd)
       highd = klass->varpanel_highd(display);
  }

  return (highd);
}

/*
 * Use the widget state to figure out which is currently displayed.
*/
static gboolean
varpanel_shows_circles (datad *d)
{
  return (d != NULL &&
          d->vcirc_ui.ebox != NULL &&
          GTK_WIDGET_REALIZED (d->vcirc_ui.ebox));
}
static gboolean
varpanel_shows_checkboxes (datad *d)
{
  return (d != NULL &&
          d->vcbox_ui.ebox != NULL &&
          GTK_WIDGET_REALIZED (d->vcbox_ui.ebox));
}

void
varpanel_reinit (ggobid *gg)
{
  datad *d;
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
  } else {  /*-- if there is a display present --*/
    d = display->d;
    highd = varpanel_highd (display);

    if (highd && varpanel_shows_checkboxes (d)) {
      varcircles_show (true, d, display, gg);
    } else if (!highd && varpanel_shows_circles (d)) {
      varcircles_show (false, d, display, gg);
    }
  }
}

void
rebuild_mode_menus(displayd *display, ggobid *gg)
{
    if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
     /* Allow the extended display to override the submenu_destroy call.
        If it doesn't provide a method, then call submenu_destroy. */
      void (*f)(displayd *dpy, GtkWidget *) =
        GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(display))->display_unset;
      if(f) {
        f(display, gg->pmode_item);
        f(display, gg->imode_item);
      }
      else { /* If no method, use this */
        if (gg->pmode_item)
          submenu_destroy (gg->pmode_item); 
        submenu_destroy (gg->imode_item);
      }
    }

    /* Then rebuild */
    if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
      void (*f)(displayd *dpy, ggobid *gg) =
        GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(display))->display_set;
      if(f)
        f(display, gg);
    }

}

void
viewmode_set (ProjectionMode pmode, InteractionMode imode, ggobid *gg)
{
  displayd *display = gg->current_display;

  if (pmode != NULL_PMODE) {
    gg->pmode_prev = gg->pmode; gg->pmode = pmode;
  }
  if (imode != NULL_IMODE) {
    gg->imode_prev = gg->imode; gg->imode = imode;
  } else {
    gg->imode_prev = gg->imode; gg->imode = DEFAULT_IMODE;
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
    }
  }

  if (imode != NULL_IMODE) {
    gchar * modeName = NULL;
    GtkWidget *panel = NULL;

    /* a change within the set of imodes */
    if (imode > DEFAULT_IMODE && imode < EXTENDED_DISPLAY_IMODE) {
      modeName = (gchar *) imode_name[imode];  /* could be DEFAULT */
      panel = mode_panel_get_by_name (modeName, gg); 
    }

    /* the pmode is taking over the control panel */
    else if (imode == DEFAULT_IMODE && gg->pmode > NULL_PMODE) {
      if (gg->pmode == EXTENDED_DISPLAY_PMODE) {
        GtkGGobiExtendedDisplayClass *klass;
        if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
          klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(display));
          panel = klass->imode_control_box(display, &modeName, gg);
        } 
      } else if (pmode < EXTENDED_DISPLAY_PMODE) {  /* scatterplot? */
        modeName = (gchar *) pmode_name[gg->pmode];
        panel = mode_panel_get_by_name (modeName, gg); 
      }
    }

    gtk_frame_set_label (GTK_FRAME (gg->imode_frame), modeName);
    gtk_container_add (GTK_CONTAINER (gg->imode_frame), panel);
    gg->current_control_panel = panel;

    /*-- avoid increasing the object's ref_count infinitely  --*/
#if GTK_MAJOR_VERSION == 1
    if (GTK_OBJECT (panel)->ref_count > 1)
#else
    if (G_OBJECT (panel)->ref_count > 1)
#endif
      gtk_widget_unref (panel);
  }

  if (pmode != NULL_PMODE && gg->pmode != gg->pmode_prev) {

    /*
     * The projection type is one of P1PLOT, XYPLOT, ROTATE, TOUR1D,
     * TOUR2D or COTOUR.  It only changes if another projection type
     * is selected.  (For parcoords, time series, and scatmat plots,
     * the value of projection is irrelevant.  A second pmode is in
     * the process of being added to the barchart.)
    */
    if (display && GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
      GtkGGobiExtendedDisplayClass *klass;
      klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(display));
      if(klass->pmode_set)
         klass->pmode_set(pmode, display, gg);
    }
  }
}

/*
 * Turn the tour procs on and off here
*/
void
procs_activate (gboolean state, ProjectionMode pmode, displayd *display, ggobid *gg)
{
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
imode_activate (splotd *sp, ProjectionMode pmode, InteractionMode imode, gboolean state, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  RedrawStyle redraw_style = NONE;

  if (state == off) {
    switch (imode) {
    case DEFAULT_IMODE:
      switch(pmode) {
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
        redraw_style = brush_activate (state, display, gg);
      break;
      case IDENT:
        redraw_style = identify_activate (state, display, gg);
      break;
      case SCALE:
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
  } else if (state == on) {
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
        redraw_style = brush_activate (state, display, gg);
      break;
      case IDENT:
        redraw_style = identify_activate (state, display, gg);
      break;
      case SCALE:
        scale_click_init (sp, gg);
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
projection_ok (ProjectionMode m, displayd *display)
{
  gboolean ok = true;
  datad *d = display->d;

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

void
pmode_set_cb (GtkWidget *w, gint action) {
  ggobid *gg = GGobiFromWidget(w, true);
  ProjectionMode pm = (ProjectionMode) action;

  if ((pm != gg->pmode || gg->imode != DEFAULT_IMODE) &&
       projection_ok(pm, gg->current_display)) 
  {
    /* When the pmode is reset, the imode is set to the default */
    GGOBI(full_viewmode_set)(pm, DEFAULT_IMODE, gg);
  }
}
void
imode_set_cb (GtkWidget *w, gint action) {
  ggobid *gg = GGobiFromWidget(w, true);
  InteractionMode im;

  im = (InteractionMode) action;
  if (im != gg->imode) {
    GGOBI(full_viewmode_set)(NULL_PMODE, im, gg);
  }
}

/* Do everything in one routine for now; split later if apropriate */
gint
GGOBI(full_viewmode_set)(ProjectionMode pmode, InteractionMode imode, ggobid *gg)
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
    viewmode_set(pmode, imode, gg);
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
     * work out which mode menus (Options, Reset, I/O) need
     * to be present, and add the needed callbacks.  
     * Arguments are goofy -- prev is just as easy to get hold of as mode.
    */
    main_miscmenus_update (gg->pmode_prev, gg->imode_prev, gg->current_display, gg);
    /*-- redraw this display --*/
    display_tailpipe (display, FULL, gg);

    /*-- redraw as needed for transient brushing and identify --*/
    if (redraw_style != NONE || reinit_transient_p) {
      displays_plot (sp, FULL, gg);
    }

 /**/return (gg->imode);

  } else {  /* if there's no display */
    viewmode_set (NULL_PMODE, NULL_IMODE, gg);
    /*-- need to remove console menus: Options, Reset, ... --*/
    main_miscmenus_update (gg->pmode_prev, gg->imode_prev, NULL, gg);
    submenu_destroy (gg->imode_item);

/**/return (NULL_IMODE);
  }

  return(-1);
}

static GtkItemFactoryEntry menu_items[] = {
  { "/_File",            NULL,     NULL,             0, "<Branch>" },
  { "/File/Open ...",
       NULL,    
       (GtkItemFactoryCallback) filename_get_r,  
       0 },
  { "/File/New",
       NULL,    
       (GtkItemFactoryCallback) create_new_ggobi,  
       0 },
  { "/File/Save ...",   
       NULL,    
       (GtkItemFactoryCallback) writeall_window_open,    
       2 },

  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },

  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Store session",   
       NULL,   
       (GtkItemFactoryCallback) store_session, 
       0 },
#ifdef PRINTING_IMPLEMENTED
  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Print",
       NULL,    
       (GtkItemFactoryCallback) display_write_svg,         
       0 },
#endif

  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Close",   
       "<ctrl>C",   
       (GtkItemFactoryCallback) ggobi_close, 
       0 },
  { "/File/Quit",   
       "<ctrl>Q",   
       (GtkItemFactoryCallback) quit_ggobi, 
       0 },


  { "/_Tools",        NULL,         NULL, 0, "<Branch>" },
  { "/Tools/Variable manipulation ...", 
       NULL,        
       (GtkItemFactoryCallback) vartable_open,   
       0,
       NULL },
  { "/Tools/Variable transformation ...", 
       NULL,        
       (GtkItemFactoryCallback) transform_window_open,
       0,
       NULL },
  { "/Tools/Sphering ...", 
       NULL,        
       (GtkItemFactoryCallback) sphere_panel_open,
       0,
       NULL },
#ifdef INFERENCE_IMPLEMENTED
  { "/Tools/Inference ...", 
       NULL,        
       (GtkItemFactoryCallback) NULL,  /*-- inference_window_open --*/
       0,
       NULL },
#endif
  { "/Tools/Variable jittering ...", 
       NULL,        
       (GtkItemFactoryCallback) jitter_window_open,   
       0,
       NULL },
  { "/Tools/Color schemes ...", 
       NULL,        
       (GtkItemFactoryCallback) wvis_window_open,   
       0,
       NULL },

  /*-- Tools that apply to cases --*/
  { "/Tools/sep",     NULL, NULL, 0, "<Separator>" },
  { "/Tools/Color & glyph groups ...", 
       NULL,        
       (GtkItemFactoryCallback) cluster_window_open,
       0,
       NULL },
  { "/Tools/Case subsetting and sampling ...", 
       NULL,        
       (GtkItemFactoryCallback) subset_window_open,   
       0,
       NULL },
  { "/Tools/sep",     NULL, NULL, 0, "<Separator>" },
#ifdef SMOOTH_IMPLEMENTED
  { "/Tools/Smooth ...", 
       NULL,        
       (GtkItemFactoryCallback) smooth_window_open,   
       0,
       NULL },
#endif

  { "/Tools/Missing values ...", 
       NULL,        
       (GtkItemFactoryCallback) impute_window_open,   
       0,
       NULL },
  { "/Tools/sep",     NULL, NULL, 0, "<Separator>" },  /*-- before plugins --*/

  /* Experiment: moving this to the Display menu -- dfs */
  /*
  {"/Dis_playTree", NULL, NULL, 0, "<Branch>"},
  { "/DisplayTree/Displays",    
       NULL, 
       (GtkItemFactoryCallback) show_display_tree,
       2},
  */

  { "/_Help",                NULL, NULL, 0, "<LastBranch>" },
  { "/Help/About GGobi",
       NULL,
       (GtkItemFactoryCallback) splash_show,
       0 },
/*
  { "/Help/About help ...",  NULL, NULL, 0, NULL },
*/

#ifdef SUPPORT_PLUGINS
  { "/Help/About plugins ...",
       NULL,
       (GtkItemFactoryCallback) show_plugin_list,
       (gint) NULL },
#endif
};


/*
#ifndef AS_GGOBI_LIBRARY
*/
/*
  Wrapper for gtk_main_quit so that we can override this in
  other applications to avoid quitting when the user selects
  the Quit button.
 */
void
quit_ggobi(ggobid *gg, gint action, GtkWidget *w)
{
#ifdef SUPPORT_PLUGINS
  extern void closePlugins(ggobid *gg);
  gint n, i;
  ggobid *el;
  n = GGobi_getNumGGobis();
  for(i = 0; i < n ; i++) {
    el = GGobi_ggobi_get(i);
    if(el != gg) 
      closePlugins(el);
  }
  closePlugins(gg);
#endif
  procs_activate (off, gg->pmode, gg->current_display, gg);
  gtk_main_quit();
}


void 
make_ui (ggobid *gg) 
{
  GtkWidget *window;
  GtkWidget *hbox, *vbox, *entry;
  GtkWidget *basement;

  gg->tips = gtk_tooltips_new ();

  gg->main_window = window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  GGobi_widget_set (window, gg, true);

#ifdef TEST_GGOBI_EVENTS
/*  gtk_signal_connect (GTK_OBJECT(gg), "splot_new", test_new_plot_cb, (gpointer) "A new plot"); */
  gtk_signal_connect_object(GTK_OBJECT(gg), "splot_new", test_new_plot_cb, (gpointer) "A new plot");
  gtk_signal_connect(GTK_OBJECT(gg), "datad_added", test_data_add_cb, NULL);
  gtk_signal_connect(GTK_OBJECT(gg), "sticky_point_added", test_sticky_points, NULL);
  gtk_signal_connect(GTK_OBJECT(gg), "sticky_point_removed", test_sticky_points, NULL);
#endif

/*
 * I used to set allow_shrink to true, but somehow it causes what we
 * experience as auto_shrink-ing.  That is, when changing viewmode or
 * selecting variables, the size of the ggobi console window can suddenly
 * change.  This seems to fix that, with the perhaps undesirable side
 * effect that I can't reduce the size of the console below
 * its initial size.  -- dfs
*/
/*gtk_window_set_policy (GTK_WINDOW (window), allow_shrink, allow_grow, auto_shrink);*/
  gtk_window_set_policy (GTK_WINDOW (window), false,        true,       false);

  gtk_signal_connect_object(GTK_OBJECT (window), "delete_event",
                            GTK_SIGNAL_FUNC (ggobi_close), (gpointer) gg);
  gtk_signal_connect_object(GTK_OBJECT (window), "destroy_event",
                            GTK_SIGNAL_FUNC (ggobi_close), (gpointer) gg); 

  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (false, 1);
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  gg->main_accel_group = gtk_accel_group_new ();
  gg->main_menu_factory = get_main_menu (menu_items,
    sizeof (menu_items) / sizeof (menu_items[0]),
    gg->main_accel_group, window,
    &gg->main_menubar, (gpointer) gg);

/* I don't know that this is the best place for this ... should I
create and destroy these groups when the menus are torn down and
rebuilt? -- dfs */

  gg->pmode_accel_group = gtk_accel_group_new ();
  gg->imode_accel_group = gtk_accel_group_new ();
  /*
  gtk_window_add_accel_group (GTK_WINDOW (window), gg->pmode_accel_group);
  gtk_window_add_accel_group (GTK_WINDOW (window), gg->imode_accel_group);
  gtk_accel_group_lock (gg->pmode_accel_group);
  gtk_accel_group_lock (gg->imode_accel_group);
  */

#ifdef SUPPORT_INIT_FILES
  if (sessionOptions->info && sessionOptions->info->numInputs > 0) {
    GtkWidget *w;
    w = gtk_item_factory_get_widget(gg->main_menu_factory, "/File");
    addPreviousFilesMenu(w, sessionOptions->info, gg);
  }
#endif

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
  gtk_container_set_border_width (GTK_CONTAINER (gg->imode_frame), 3);
  gtk_frame_set_shadow_type (GTK_FRAME (gg->imode_frame),
    GTK_SHADOW_IN);

  make_control_panels (gg);
  if (gg->imode != NULL_IMODE) {
    if (gg->imode == DEFAULT_IMODE)
      gtk_container_add (GTK_CONTAINER (gg->imode_frame),
    	mode_panel_get_by_name((gchar *) pmode_name[gg->pmode], gg));
    else
      gtk_container_add (GTK_CONTAINER (gg->imode_frame),
    	mode_panel_get_by_name((gchar *) imode_name[gg->imode], gg));
  }

  /*-- Variable selection panel --*/
  varpanel_make (hbox, gg);

  /*-- status bar --*/
  entry = gtk_entry_new ();
  gtk_editable_set_editable(GTK_EDITABLE(entry), false);
  gtk_object_set_data (GTK_OBJECT(gg->main_window), "MAIN:STATUSBAR", entry);
  gtk_box_pack_start (GTK_BOX (vbox), entry, false, false, 0);
  /*--            --*/

  gtk_widget_show_all (hbox);

  /* -- do not map or show this widget -- */
  basement = gtk_vbox_new(false, 0);
  gtk_widget_set_name (basement, "BASEMENT");
  gtk_box_pack_start (GTK_BOX (hbox), basement, false, false, 0);
  /* -- do not map or show this widget -- */
  
  /*-- at this point, the mode could be NULLMODE, P1PLOT, or XYPLOT --*/
  {
    void main_miscmenus_initialize (ggobid *gg);
    main_miscmenus_initialize (gg);
  }

  if(sessionOptions->showControlPanel)
      gtk_widget_show_all (window);
}

const gchar * const* 
GGOBI(getPModeNames)(int *n)
{
  *n = sizeof(GGOBI(PModeNames))/sizeof(GGOBI(PModeNames)[0]);
  return (GGOBI(PModeNames));
}
const gchar * const* 
GGOBI(getIModeNames)(int *n)
{
  *n = sizeof(GGOBI(IModeNames))/sizeof(GGOBI(IModeNames)[0]);
  return (GGOBI(IModeNames));
}

const gchar * const* 
GGOBI(getPModeKeys)(int *n)
{
  *n = sizeof(GGOBI(PModeKeys))/sizeof(GGOBI(PModeKeys)[0]);
  return (GGOBI(PModeKeys));
}


#ifdef SUPPORT_INIT_FILES

void load_previous_file(GtkWidget *w, gpointer cbd);
/*
  Add the previous input sources to the menu.
 */
void
addPreviousFilesMenu(GtkWidget *parent, GGobiInitInfo *info, ggobid *gg)
{
  gint i;
  GtkWidget *el;
  InputDescription *input;
  if(info) {
    for(i = 0 ; i < info->numInputs ; i++) {
      input = &(info->descriptions[i].input);
      if(input && input->fileName) {
        el = gtk_menu_item_new_with_label(input->fileName);
        gtk_signal_connect(GTK_OBJECT(el), "activate",
                           GTK_SIGNAL_FUNC(load_previous_file),
                           info->descriptions + i);
        GGobi_widget_set(el, gg, true);
        gtk_menu_insert(GTK_MENU(parent), el, 3 + i + 1);
      }
    }
  }
}


ggobid *create_ggobi(InputDescription *desc);

void
load_previous_file(GtkWidget *w, gpointer cbd)
{
  InputDescription *desc;
  GGobiDescription *gdesc;
  ggobid *gg;

  gg = GGobiFromWidget(w, false);
  gdesc = (GGobiDescription*) cbd;
  desc =  &(gdesc->input);

  if(g_slist_length(gg->d) > 0)
    create_ggobi(desc);
  else {
    read_input(desc, gg);
    /* Need to avoid the initial scatterplot. */
    start_ggobi(gg, true, gdesc->displays == NULL);
  }


  if (gdesc->displays) {
    gint i, n;
    GGobiDisplayDescription* dpy;
    n = g_list_length(gdesc->displays);
    for (i = 0; i < n ; i++) {    
      dpy = (GGobiDisplayDescription*) g_list_nth_data(gdesc->displays, i);
      createDisplayFromDescription(gg, dpy);
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
#endif

/*
 This replicates code elsewhere and the two should be merged.
 */
ggobid *
create_ggobi(InputDescription *desc)
{
  gboolean init_data = true;
  ggobid *gg;

  gg = ggobi_alloc(NULL);

     /*-- some initializations --*/
  gg->displays = NULL;
  gg->control_panels = NULL;
  globals_init (gg);      /*-- variables that don't depend on the data --*/
  special_colors_init (gg);
  make_ui (gg);
  gg->input = desc;

  read_input(desc, gg);

  if(sessionOptions->info != NULL) {
    extern gboolean registerPlugins(ggobid *gg, GList *plugins);
    registerPlugins(gg, sessionOptions->info->plugins);
  }

  start_ggobi(gg, init_data, sessionOptions->info->createInitialScatterPlot);

  return(gg);
}


#ifdef SUPPORT_PLUGINS
void
show_plugin_list(ggobid *gg, gint action, GtkWidget *w)
{
  if(sessionOptions->info && sessionOptions->info->plugins)
    showPluginInfo(sessionOptions->info->plugins,
      sessionOptions->info->inputPlugins, (ggobid*) gg);
}
#endif



void
store_session_in_file(GtkWidget *btn, GtkWidget *selector)
{
  const gchar *fileName;
  ggobid *gg;

  fileName = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  if(fileName && fileName[0]) {
    gg = gtk_object_get_data(GTK_OBJECT(selector), "ggobi");
    write_ggobi_as_xml(gg, fileName, NULL);
    gtk_widget_destroy(selector);
  } else {
    quick_message("Pick a file", true);
  }
}

void
store_session(ggobid *gg, gint action, GtkWidget *w)
{
  GtkWidget *dlg;
  if(!sessionOptions->info->sessionFile) {
    char buf[1000];
    sprintf(buf,"%s%c%s", getenv("HOME"), G_DIR_SEPARATOR, ".ggobi-session");
    dlg = gtk_file_selection_new("Save ggobi session");
    gtk_object_set_data(GTK_OBJECT(dlg), "ggobi", (gpointer) gg);
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(dlg), buf);
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dlg)->ok_button),
      "clicked", GTK_SIGNAL_FUNC (store_session_in_file), dlg);

    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dlg)->cancel_button),
      "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
      (gpointer) dlg);

    gtk_widget_show(dlg);
  } else {
    ggobi_write_session(sessionOptions->info->sessionFile);
    /* write_ggobi_as_xml(gg, sessionOptions->info->sessionFile); */
  }
}



void
create_new_ggobi(ggobid *gg, gint action, GtkWidget *w)
{
  create_ggobi(NULL);
}
