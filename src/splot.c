/* splot.c: an individual scatterplot */
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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "ggobi-renderer-factory.h"

/*--------------------------------------------------------------------*/
/*                             Events                                 */
/*--------------------------------------------------------------------*/

static gint
splot_configure_cb (GtkWidget *w, GdkEventConfigure *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot(sp);
  displayd *display = (displayd *) sp->displayptr; 
  cpaneld *cpanel = &display->cpanel;
  GGobiData *d = display->d;
  GGobiRendererFactory *factory = ggobi_renderer_factory_new();
  GGobiRenderer *renderer; 

  /*
   * Somehow when a new splot is added to a table, the initial
   * configuration event for the drawing_area occurs before the
   * drawing_area has been properly sized.  Maybe I'm not executing
   * calls in the proper order?  This protects me in the meantime.
  */
  if (w->allocation.width < 2 || w->allocation.height < 2) {
    return false;
  }

  /*
   * This is not the best place to do this, perhaps, but it works
   * nicely here -- it makes certain that plots in the scatterplot
   * matrix are correctly initialized.  (And I don't know why, either)
  */
  if (sp->pixmap0 == NULL) {  /*-- ie, splot being initialized --*/
    splot_world_to_plane (cpanel, sp, gg);
  }

  /*-- Create new backing pixmaps of the appropriate size --*/
  if (sp->pixmap0 != NULL)
    gdk_pixmap_unref (sp->pixmap0);
  if (sp->pixmap1 != NULL)
    gdk_pixmap_unref (sp->pixmap1);
/*
  sp->pixmap0 = gdk_pixmap_new (w->window,
    w->allocation.width, w->allocation.height, -1);
  sp->pixmap1 = gdk_pixmap_new (w->window,
    w->allocation.width, w->allocation.height, -1);
*/

  renderer = ggobi_renderer_factory_create(factory, w->window);
  sp->pixmap0 = GDK_DRAWABLE(renderer);
  renderer = ggobi_renderer_factory_create(factory, w->window);
  sp->pixmap1 = GDK_DRAWABLE(renderer);
  g_object_unref(G_OBJECT(factory));
  
  if (cpanel->imode == BRUSH) {
    sp->brush_pos.x1 = (gint) ((gfloat) sp->brush_pos.x1 *
      (gfloat) (w->allocation.width) / (gfloat) (sp->max.x));
    sp->brush_pos.x2 = (gint) ((gfloat) sp->brush_pos.x2 *
      (gfloat) (w->allocation.width) / (gfloat) (sp->max.x));

    sp->brush_pos.y1 = (gint) ((gfloat) sp->brush_pos.y1 *
      (gfloat) (w->allocation.height)/ (gfloat) (sp->max.y));
    sp->brush_pos.y2 = (gint) ((gfloat) sp->brush_pos.y2 *
      (gfloat) (w->allocation.height) / (gfloat) (sp->max.y));
  }

  sp->max.x = w->allocation.width;
  sp->max.y = w->allocation.height;

  splot_plane_to_screen (display, cpanel, sp, gg);

  if (cpanel->imode == BRUSH) {
    if (GGOBI_IS_EXTENDED_SPLOT(sp)) {
      void (*f)(GGobiData *, splotd *, ggobid *);
      GGobiExtendedSPlotClass *klass;
      klass = GGOBI_EXTENDED_SPLOT_GET_CLASS(sp);
      f = klass->splot_assign_points_to_bins;
      if(f) {
        f(d, sp, gg);  // need to exclude area plots
      }
    }
  }

  sp->redraw_style = FULL;
  gtk_widget_queue_draw (sp->da);

  return false;
}


static gint
splot_expose_cb (GtkWidget *w, GdkEventExpose *event, splotd *sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot (sp);

  /*-- sanity checks --*/
  if (sp->pixmap0 == NULL || sp->pixmap1 == NULL)
    return retval;
  if (w->allocation.width < 2 || w->allocation.height < 2)
    return retval;

  splot_redraw (sp, sp->redraw_style, gg);

  return retval;
}

void
splot_connect_expose_handler (gboolean idled, splotd *sp) 
{
  if (idled)  // if idle_proc running
    g_signal_handlers_disconnect_by_func (G_OBJECT (sp->da),
       G_CALLBACK(splot_expose_cb), GTK_OBJECT (sp));
  else
    g_signal_connect (G_OBJECT (sp->da),
      "expose_event", G_CALLBACK(splot_expose_cb), (gpointer) sp);
}

/*-- this will be called by a key_press_cb for each scatterplot mode --*/
gboolean
splot_event_handled (GtkWidget *w, GdkEventKey *event,
  cpaneld *cpanel, splotd *sp, ggobid *gg)
{
  static guint32 etime = (guint32) 0;
  gboolean common_event = true;
  displayd *display = (displayd *) NULL;

/* In composite displays, we sometimes find ourselves trying to
 * process an event on a deleted splot.  I'm not sure if that's a bug
 * that should be fixed properly, or whether doing some error-checking
 * is an adequate response.  At the moment, I think I'm seeing gtk
 * bugs in addition to our own. -- dfs
 */
  if (!sp) return false;
  if (sp->displayptr) {
    display = ValidateDisplayRef((displayd *) sp->displayptr, 
				 gg, false);
  }
  if (!display) return false;
/*
 * I can't say this is the best way to handle this bug, but it
 * seems to work.  By switching modes before the processing
 * of the keypress is completed, I somehow start an infinite 
 * loop in the new mode -- as soon as its key press signal handler
 * is connected, it starts handling the identical key press
 * event that was just handled in the previous mode.  This test of
 * event->time ensures that the same key press event won't be handled
 * a second time.  There's got to be a better way ...
*/
  if (event->time == etime) return false;  /*-- already processed --*/

  if(GGOBI_IS_EXTENDED_DISPLAY(display)) {
    GGobiExtendedDisplayClass *klass;
    klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS(display);
    if (klass->splot_key_event_handled) {
      common_event = klass->splot_key_event_handled(w, display, sp,
        event, gg);
    }
  }
  etime = event->time;

  return common_event;
}


void
sp_event_handlers_toggle (splotd *sp, gboolean state, ProjectionMode pmode, InteractionMode imode) 
{
  displayd *display = (displayd *) sp->displayptr;

  /* scatmat and parcoords are handling everything now and returning
     false; ts and barchart are handling their own and then returning
     to the switch statement; scatterplot doesn't have one of these
     routines yet.  dfs 8/31/2005
 */
  if(GGOBI_IS_EXTENDED_DISPLAY(display)) {
    GGobiExtendedDisplayClass *klass;
    klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS(display);
    if(klass->event_handlers_toggle && klass->event_handlers_toggle(display, sp, state, pmode, imode) == false) {
      return;
    }
  }

  switch (imode) {
  case DEFAULT_IMODE:
  switch (pmode) {
    case P1PLOT:
      p1d_event_handlers_toggle (sp, state);
    break;
    case XYPLOT:
      xyplot_event_handlers_toggle (sp, state);
    break;
    case TOUR1D:
      tour1d_event_handlers_toggle (sp, state);
    break;
    case TOUR2D3:
      tour2d3_event_handlers_toggle (sp, state);
    break;
    case TOUR2D:
      tour2d_event_handlers_toggle (sp, state);
    break;
    case COTOUR:
      ctour_event_handlers_toggle (sp, state);
    break;
    default:
    break;
  }
  break;
  case SCALE:
      scale_event_handlers_toggle (sp, state);
  break;
  case BRUSH:
      brush_event_handlers_toggle (sp, state);
  break;
  case IDENT:
      identify_event_handlers_toggle (sp, state);
  break;
  case EDGEED:
      edgeedit_event_handlers_toggle (sp, state);
  break;
  case MOVEPTS:
      movepts_event_handlers_toggle (sp, state);
  break;
  default:
  break;
  }

}

void
splot_set_current (splotd *sp, gboolean state, ggobid *gg) {
/*
 * Turn on or off the event handlers in sp
*/
  if (sp != NULL) {
    displayd *display = (displayd *) sp->displayptr;
    cpaneld *cpanel = &display->cpanel;

    sp_event_handlers_toggle (sp, state, cpanel->pmode, cpanel->imode);
    imode_activate (sp, cpanel->pmode, cpanel->imode, state, gg);

    /*
     * this is now the only place varpanel_refresh is called in
     * changing the current display and splot; we'll see if it's
     * adequate -- and it's probably overkill sometimes, too.
    */
    if (state == on) {
      varpanel_refresh (display, gg);
    }
  }
}

void
GGOBI(splot_set_current_full)(displayd *display, splotd *sp, ggobid *gg)
{
  splotd *sp_prev = gg->current_splot;
  /*-- display and cpanel for outgoing current_splot --*/
  displayd *display_prev = NULL;
  cpaneld *cpanel = NULL;
  /*ProjectionMode pmode_prev = gg->pmode;*/
  InteractionMode imode_prev = gg->imode;

  if (sp != sp_prev) {
    if (sp_prev != NULL) {
      splot_set_current (sp_prev, off, gg);
      display_prev = (displayd *) sp_prev->displayptr;
      cpanel = &display_prev->cpanel;

      /*
       * This feels like a kludge, but I don't know where else to do
       * it.  We want to handle a special case: we're brushing in a
       * multi-plot display, and we move to a new splot within the
       * same display.
       * In the future, there may be other things we want to undo, but
       * for now we just want to turn off the effects of in the
       * previous splot.
      */
      if (g_list_length (display_prev->splots) > 1 /*-- multi-plot display --*/
          && display == display_prev)   /*-- display not changing --*/
      {
        reinit_transient_brushing (display, gg);
      }

      if (gg->current_display != display)
        display_set_current (display, gg);  /* old one off, new one on */
    }

    gg->current_splot = sp->displayptr->current_splot = sp;
    splot_set_current (sp, on, gg);

    /*main_miscmenus_update (pmode_prev, imode_prev, display_prev, gg);*/

    /*
     * if the previous splot is in transient brushing mode, a FULL
     * redraw is required.
     *
     * if the previous splot is in identify, a QUICK redraw is required
     *
     * otherwise, just redraw the borders of the two affected splots
    */
    if (imode_prev == NULL_IMODE || cpanel == NULL)
      displays_plot (NULL, FULL, gg);

    if (imode_prev == BRUSH && cpanel->br.mode == BR_TRANSIENT)
      displays_plot (NULL, FULL, gg);
    else if (imode_prev == IDENT)
      displays_plot (NULL, QUICK, gg);
    else {
      /* remove border from the previous splot */
      if (sp_prev != NULL) splot_redraw (sp_prev, QUICK, gg);
      /* add border to current_splot */
      splot_redraw (sp, QUICK, gg);
    }
  }
}

static gint
splot_set_current_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot(sp);
  displayd *display = (displayd *) sp->displayptr; 
  GGOBI(splot_set_current_full)(display, sp, gg);

  return false;  /* so that other button press handlers also get the event */
}

/* --------------------------------------------------------------- */
/*                   Dynamic allocation section                    */
/* --------------------------------------------------------------- */

void
splot_points_realloc (gint nrows_prev, splotd *sp, GGobiData *d)
{
  gint i;

  vectorf_realloc (&sp->p1d.spread_data, d->nrows);

  sp->planar = (gcoords *) g_realloc (sp->planar,
    d->nrows * sizeof (gcoords));
  sp->screen = (icoords *) g_realloc (sp->screen,
    d->nrows * sizeof (icoords));

  for (i=nrows_prev; i<d->nrows; i++) {
    sp->planar[i].x = sp->planar[i].y = 0.0;
    sp->screen[i].x = sp->screen[i].y = 0;
  }
}

void
splot_edges_realloc (gint nedges_prev, splotd *sp, GGobiData *e) 
{
  gint i;

  sp->edges = (GdkSegment *) g_realloc ((gpointer) sp->edges,
    e->edge.n * sizeof (GdkSegment));
  sp->arrowheads = (GdkSegment *) g_realloc ((gpointer) sp->arrowheads,
    e->edge.n * sizeof (GdkSegment));

  /*-- these aren't useful values, but they're finite --*/
  if (nedges_prev > 0) {
    for (i=nedges_prev; i<e->edge.n; i++) {
      sp->edges[i].x1 = sp->edges[i].x2 = 0;
      sp->arrowheads[i].x1 = sp->arrowheads[i].x2 = 0;
    }
  }
}

void
splot_alloc (splotd *sp, displayd *display, ggobid *gg) 
{
  GGobiData *d;
  gint nr; 
  if(!display)
    return;
  d = display->d;
  nr = d->nrows;
  sp->planar = (gcoords *) g_malloc (nr * sizeof (gcoords));
  sp->screen = (icoords *) g_malloc (nr * sizeof (icoords));
  vectorf_init_null (&sp->p1d.spread_data);
  vectorf_alloc (&sp->p1d.spread_data, nr);

  if(GGOBI_IS_EXTENDED_SPLOT(sp)) {
    GGobiExtendedSPlotClass *klass;
    klass = GGOBI_EXTENDED_SPLOT_GET_CLASS(sp);
    if(klass->alloc_whiskers)
      sp->whiskers = klass->alloc_whiskers(sp->whiskers, sp, nr, d);
  }
}

void
splot_free (splotd *sp, displayd *display, ggobid *gg) 
{
  gtk_widget_hide (sp->da);

  g_free ((gpointer) sp->planar);
  g_free ((gpointer) sp->screen);
  vectorf_free (&sp->p1d.spread_data);

#ifdef WIN32
  win32_drawing_arrays_free (sp);
#endif

  if(GGOBI_IS_EXTENDED_SPLOT(sp))
     gtk_object_destroy(GTK_OBJECT(sp));
  else
     gtk_widget_destroy (GTK_WIDGET(sp));
}

splotd *
splot_new (displayd *display, gint width, gint height, ggobid *gg) 
{
  splotd *sp;

  sp = g_object_new(GGOBI_TYPE_SPLOT, NULL);
  splot_init(sp, display, gg);

  return(sp);
}

void
splot_init(splotd *sp, displayd *display, ggobid *gg) 
{
/*
 * Initialize the widget portion of the splot object
*/

  brush_pos_init (sp);
  
  //splot_dimension_set (sp, width, height);

  /*
   * Let it be possible to get a pointer to the splotd object 
   * from the drawing area; and to gg as well.
  */
  g_object_set_data(G_OBJECT (sp->da), "splotd", (gpointer) sp);
  GGobi_widget_set (sp->da, gg, true);

  gtk_widget_set_double_buffered(sp->da, false);

  g_signal_connect (G_OBJECT (sp->da),
                      "expose_event",
                      G_CALLBACK(splot_expose_cb),
                      (gpointer) sp);
  g_signal_connect (G_OBJECT (sp->da),
                      "configure_event",
                      G_CALLBACK(splot_configure_cb),
                      (gpointer) sp);
  g_signal_connect (G_OBJECT (sp->da),
                      "button_press_event",
                      G_CALLBACK(splot_set_current_cb),
                      (gpointer) sp);

  gtk_widget_set_events (sp->da, GDK_EXPOSURE_MASK | GDK_SCROLL_MASK
             | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
             | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);


/* Init some dimension attributes */
sp->pmid.x = sp->pmid.y = sp->max.x = sp->max.y = 0;

/*
 * Initialize the data portion of the splot object
*/
  sp->edges = NULL;
  sp->arrowheads = NULL;
  splot_alloc (sp, display, gg);

  sp->displayptr = display;
  sp->pixmap0 = NULL;
  sp->pixmap1 = NULL;

/*  could become splot_p1d_init ();*/
  sp->p1dvar = 0;

/*  could become splot_xyplot_init ();*/
  sp->xyvars.x = 0;
  sp->xyvars.y = 1;

/*  could become splot_scale_init ();*/
  sp->scale.x = sp->scale.y = SCALE_DEFAULT;
  sp->tour_scale.x = sp->tour_scale.y = TOUR_SCALE_DEFAULT;

  sp->key_press_id = 0;
  sp->press_id = 0;
  sp->release_id = 0;
  sp->motion_id = 0;

/* tour inits */
  sp->tour1d.initmax = true;
  sp->tour2d3.initmax = true;
  sp->tour2d.initmax = true;
  sp->tourcorr.initmax = true;

#ifdef WIN32
  sp->win32.npoints = 0;
#endif
  
/*
  g_signal_connect (G_OBJECT (GGOBI_WINDOW_DISPLAY(display)->window),
		      "key_press_event",
		      G_CALLBACK(raise_control_panel),
		      (gpointer) gg);
*/
  g_signal_emit(G_OBJECT(gg), GGobiSignals[SPLOT_NEW_SIGNAL], 0, sp);
}


void
splot_get_dimensions (splotd *sp, gint *width, gint *height) {
  *width = sp->da->allocation.width;
  *height = sp->da->allocation.height;
}

/*----------------------------------------------------------------------*/
/*                      pipeline for scatterplot                        */
/*----------------------------------------------------------------------*/

void
splot_world_to_plane (cpaneld *cpanel, splotd *sp, ggobid *gg)
/*
 * project the data from world_data[],
 * the data expressed in 'world coordinates,' to planar[], the
 * data expressed in 'projection coordinates.'
*/
{
  displayd *display = (displayd *) sp->displayptr;
  GGobiData *d = display->d;

/*
 * This may be the place to respond to the possibility that a
 * plotted variable has just been deleted.  It's no big deal for
 * the scatterplot -- unless one of the plotted variables is now
 * beyond d->ncols.
*/

  if(GGOBI_IS_EXTENDED_SPLOT(sp)) {
    GGOBI_EXTENDED_SPLOT_GET_CLASS(sp)->world_to_plane(sp,
      d, gg);
  }
}


void
splot_plane_to_screen (displayd *display, cpaneld *cpanel, splotd *sp,
  ggobid *gg)
/*
 * Use the data in projection coordinates and rescale it to the
 * dimensions of the current plotting window, writing it into screen.
*/
{
  gint i, k;
  greal scale_x, scale_y;
  GGobiData *d = display->d;
  greal gtmp;
  GGobiExtendedSPlotClass *klass = NULL;
  greal precis = (greal) PRECISION1;

  if(GGOBI_IS_EXTENDED_SPLOT(sp)) {
     klass = GGOBI_EXTENDED_SPLOT_GET_CLASS(sp);
    
     if(klass->plane_to_screen) {
        klass->plane_to_screen(sp, d, gg);
        return;
     }
  }

  /*  scale_x = (greal) (cpanel->projection == TOUR2D) ?
    sp->tour_scale.x : sp->scale.x;
  scale_y = (greal) (cpanel->projection == TOUR2D) ?
  sp->tour_scale.y : sp->scale.y;*/
  /* with the tour rescaling itself into the planar box limits,
     this shouldn't be needed any more */
  scale_x = sp->scale.x;
  scale_y = sp->scale.y;

  /*
   * Calculate is, a scale factor.  Scale so as to use the entire
   * plot window (well, as much of the plot window as scale.x and
   * scale.y permit.)
  */
  scale_x /= 2;
  sp->iscale.x = (greal) sp->max.x * scale_x;
  scale_y /= 2;
  sp->iscale.y = -1 * (greal) sp->max.y * scale_y;

  /*
   * Calculate new coordinates.
  */
  for (k=0; k<d->nrows_in_plot; k++) {
    i = d->rows_in_plot.els[k];

    /*-- scale from world to plot window --*/
    gtmp = sp->planar[i].x - sp->pmid.x;
    sp->screen[i].x = (gint) (gtmp * sp->iscale.x / precis);
    gtmp = sp->planar[i].y - sp->pmid.y;
    sp->screen[i].y = (gint) (gtmp * sp->iscale.y / precis);

    /*-- shift into middle of plot window --*/
    sp->screen[i].x += (sp->max.x / 2);
    sp->screen[i].y += (sp->max.y / 2);
  }

  if(klass && klass->sub_plane_to_screen) {
     klass->sub_plane_to_screen(sp, display, d, gg);
  } 
}

/*----------------------------------------------------------------------*/
/*                  reverse pipeline for scatterplot                    */
/*----------------------------------------------------------------------*/

/* 
 * In order to compute the limits for the rulers, I need to
 * run a pair of points through the reverse pipeline -- but
 * without having any impact on the pipeline data structures.
 *
 * This should probably be replaced with pt_screen_to_raw
 * in lineedit.c.  I just have to make sure the results are
 * consistent. -- dfs
*/
/*
void
splot_screen_to_tform (cpaneld *cpanel, splotd *sp, icoords *scr,
  fcoords *tfd, ggobid *gg)
{
  gcoords planar, world;
  greal precis = (greal) PRECISION1;
  greal ftmp, max, min, rdiff;
  displayd *display = (displayd *) sp->displayptr;
  GGobiData *d = display->d;
  gfloat scale_x, scale_y;
  vartabled *vt, *vtx, *vty;

  g_return_if_fail (cpanel->pmode == XYPLOT ||
                    cpanel->pmode == P1PLOT ||
                    cpanel->pmode == TOUR1D ||
                    cpanel->pmode == TOUR2D3 ||
                    cpanel->pmode == TOUR2D ||
                    cpanel->pmode == COTOUR);

  scale_x = sp->scale.x;
  scale_y = sp->scale.y;
  scale_x /= 2;
  sp->iscale.x = (greal) sp->max.x * scale_x;
  scale_y /= 2;
  sp->iscale.y = -1 * (greal) sp->max.y * scale_y;

*
 * screen to plane 
*
  planar.x = (scr->x - sp->max.x/2) * precis / sp->iscale.x ;
  planar.x += sp->pmid.x;
  planar.y = (scr->y - sp->max.y/2) * precis / sp->iscale.y ;
  planar.y += sp->pmid.y;

*
 * plane to world
*

  switch (cpanel->pmode) {
    case P1PLOT:
      vt = vartable_element_get (sp->p1dvar, d);
      max = vt->lim.max;
      min = vt->lim.min;
      rdiff = max - min;

      if (display->p1d_orientation == HORIZONTAL) {
        world.x = planar.x;
        ftmp = world.x / precis;
        tfd->x = (ftmp + 1.0) * .5 * rdiff;
        tfd->x += min;
      } else {
        world.y = planar.y;
        ftmp = world.y / precis;
        tfd->y = (ftmp + 1.0) * .5 * rdiff;
        tfd->y += min;
      }
    break;

    case XYPLOT:
      vtx = vartable_element_get (sp->xyvars.x, d);
      max = vtx->lim.max;
      min = vtx->lim.min;
      rdiff = max - min;
      world.x = planar.x;
      ftmp = world.x / precis;
      tfd->x = (ftmp + 1.0) * .5 * rdiff;
      tfd->x += min;

      vty = vartable_element_get (sp->xyvars.y, d);
      max = vty->lim.max;
      min = vty->lim.min;
      rdiff = max - min;
      world.y = planar.y;
      ftmp = world.y / precis;
      tfd->y = (ftmp + 1.0) * .5 * rdiff;
      tfd->y += min;
    break;

    default:
    break;
  }
}
*/

/*
 * The remainder of the reverse pipeline routines operate on
 * the ggobi data structures.
*/

void
splot_screen_to_plane (splotd *sp, gint pt, gcoords *eps,
  gboolean horiz, gboolean vert)
{
  gcoords prev_planar;
  greal precis = (greal) PRECISION1;

  gfloat scale_x, scale_y;
  scale_x = sp->scale.x;
  scale_y = sp->scale.y;
  scale_x /= 2;
  sp->iscale.x = (greal) sp->max.x * scale_x;
  scale_y /= 2;
  sp->iscale.y = -1 * (greal) sp->max.y * scale_y;

  if (horiz) {
    sp->screen[pt].x -= sp->max.x/2;

    prev_planar.x = sp->planar[pt].x;
    sp->planar[pt].x = (greal) sp->screen[pt].x * precis / sp->iscale.x ;
    sp->planar[pt].x += (greal) sp->pmid.x;

    eps->x = sp->planar[pt].x - prev_planar.x;
  }

  if (vert) {
    sp->screen[pt].y -= sp->max.y/2;

    prev_planar.y = sp->planar[pt].y;
    sp->planar[pt].y = (greal) sp->screen[pt].y * precis / sp->iscale.y ;
    sp->planar[pt].y += (greal) sp->pmid.y;

    eps->y = sp->planar[pt].y - prev_planar.y;
  }
}

/*
void
splot_plane_to_world (splotd *sp, gint ipt, ggobid *gg) 
{
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  GGobiData *d = display->d;

  switch (cpanel->pmode) {
    case P1PLOT:
      if (display->p1d_orientation == VERTICAL)
        d->world.vals[ipt][sp->p1dvar] = (greal) sp->planar[ipt].y;
      else
        d->world.vals[ipt][sp->p1dvar] = (greal) sp->planar[ipt].x;
    break;

    case XYPLOT:
      d->world.vals[ipt][sp->xyvars.x] = (greal) sp->planar[ipt].x;
      d->world.vals[ipt][sp->xyvars.y] = (greal) sp->planar[ipt].y;
    break;

    case TOUR1D:
    {
      gint j, var;
        for (j=0; j<display->t1d.nactive; j++) {
          var = display->t1d.active_vars.els[j];
          d->world.vals[ipt][var] += 
           (gg->movepts.eps.x * (greal) display->t1d.F.vals[0][var]);
        }
    }
    break;

    case TOUR2D3:
    {
      gint j, var;
      for (j=0; j<display->t2d3.nactive; j++) {
        var = display->t2d3.active_vars.els[j];
        d->world.vals[ipt][var] += 
         (gg->movepts.eps.x * (greal) display->t2d3.F.vals[0][var] +
          gg->movepts.eps.y * (greal) display->t2d3.F.vals[1][var]);
      }
    }
    break;

    case TOUR2D:
    {
      gint j, var;
        for (j=0; j<display->t2d.nactive; j++) {
          var = display->t2d.active_vars.els[j];
          d->world.vals[ipt][var] += 
           (gg->movepts.eps.x * (greal) display->t2d.F.vals[0][var] +
            gg->movepts.eps.y * (greal) display->t2d.F.vals[1][var]);
        }
    }
    break;

    case COTOUR:
    {
      gint j, var;
        for (j=0; j<display->tcorr1.nactive; j++) {
          var = display->tcorr1.active_vars.els[j];
          d->world.vals[ipt][var] += 
           (gg->movepts.eps.x * (greal) display->tcorr1.F.vals[0][var]);
        }
        for (j=0; j<display->tcorr2.nactive; j++) {
          var = display->tcorr2.active_vars.els[j];
          d->world.vals[ipt][var] += 
           (gg->movepts.eps.y * (greal) display->tcorr2.F.vals[0][var]);
        }
    }

    break;

    default:
      g_printerr ("reverse pipeline not yet implemented for this projection\n");
  }
}
*/

/*
void
splot_reverse_pipeline (splotd *sp, gint ipt, gcoords *eps,
                        gboolean horiz, gboolean vert, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  GGobiData *d = display->d;
  splot_screen_to_plane (sp, ipt, eps, horiz, vert);
  splot_plane_to_world (sp, ipt, gg);
  world_to_raw (ipt, sp, d, gg);
}
*/

/* ---------------------------------------------------------------------*/
/*          Pack up some of short signal routines                       */
/* ---------------------------------------------------------------------*/

/*-- this one isn't attached to sp->da, but we'll bundle it anyway --*/
void
disconnect_key_press_signal (splotd *sp) {
  displayd *display;

  if (sp) {
    display = (displayd *) sp->displayptr;
    if (sp->key_press_id && GGOBI_IS_WINDOW_DISPLAY(display)) {
      g_signal_handler_disconnect (G_OBJECT (GGOBI_WINDOW_DISPLAY(display)->window), sp->key_press_id);
      sp->key_press_id = 0;
    }
  }
}

void
disconnect_button_press_signal (splotd *sp) 
{
  if (sp && sp->press_id) {
    g_signal_handler_disconnect (G_OBJECT (sp->da), sp->press_id);
    sp->press_id = 0;
  }
}

void
disconnect_button_release_signal (splotd *sp) {
  if (sp && sp->release_id) {
    g_signal_handler_disconnect (G_OBJECT (sp->da), sp->release_id);
    sp->release_id = 0;
  }
}
void
disconnect_motion_signal (splotd *sp) {
  if (sp && sp->motion_id) {
    g_signal_handler_disconnect (G_OBJECT (sp->da), sp->motion_id);
    sp->motion_id = 0;
  }
}
void
disconnect_scroll_signal (splotd *sp) {
  if (sp && sp->scroll_id) {
    g_signal_handler_disconnect (G_OBJECT (sp->da), sp->scroll_id);
    sp->scroll_id = 0;
  }
}

/*--------------------------------------------------------------------*/
/*                           Cursors                                  */
/*--------------------------------------------------------------------*/

/*
 * Return to the default cursor
*/
void
splot_cursor_set (gint jcursor, splotd *sp)
{
  GdkWindow *window = sp->da->window;

  if (jcursor == (gint) NULL) {
    if (sp->cursor != NULL)
      gdk_cursor_destroy (sp->cursor);
    sp->jcursor = (gint) NULL;
    sp->cursor = (gint) NULL;
    gdk_window_set_cursor (window, NULL);
  } else {
    sp->jcursor = (gint) jcursor;
    sp->cursor = gdk_cursor_new (sp->jcursor);
    gdk_window_set_cursor (window, sp->cursor);
  }
}
