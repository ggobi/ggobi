/* splot.c: an individual scatterplot */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


/*--------------------------------------------------------------------*/
/*                             Events                                 */
/*--------------------------------------------------------------------*/

static gint
splot_configure_cb (GtkWidget *w, GdkEventConfigure *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot(sp);
  displayd *display = (displayd *) sp->displayptr; 
  cpaneld *cpanel = &display->cpanel;
  datad *d = display->d;
  gg = sp->displayptr->ggobi;

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

  sp->pixmap0 = gdk_pixmap_new (w->window,
    w->allocation.width, w->allocation.height, -1);
  sp->pixmap1 = gdk_pixmap_new (w->window,
    w->allocation.width, w->allocation.height, -1);

  if (cpanel->viewmode == BRUSH) {
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

  if (cpanel->viewmode == BRUSH) {
    assign_points_to_bins (d, gg);
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

/*-- this will be called by a key_press_cb for each scatterplot mode --*/
gboolean
splot_event_handled (GtkWidget *w, GdkEventKey *event,
  cpaneld *cpanel, splotd *sp, ggobid *gg)
{
  static guint32 etime = (guint32) 0;
  gboolean common_event = true;
  gint action = -1;
  displayd *display = (displayd *) sp->displayptr;

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

  if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
    GtkGGobiExtendedDisplayClass *klass;
    klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
    if(klass->splot_key_event_handler) 
         action = klass->splot_key_event_handler(display, sp, event->keyval);
  }
  if(action < 0) {

  switch (event->keyval) {
   case GDK_0:
   case GDK_1:
  case GDK_2:
  case GDK_3:
  case GDK_4:
  case GDK_5:
  case GDK_6:
  case GDK_7:
  case GDK_8:
  case GDK_9:
    if (gg->NumberedKeyEventHandler != NULL &&
        gg->NumberedKeyEventHandler->handlerRoutine)
    {
      (gg->NumberedKeyEventHandler->handlerRoutine)(event->keyval, w, event,
         cpanel, sp, gg, gg->NumberedKeyEventHandler->userData);
    }
  break;

/*
 * I'm not happy about these, since a display type is not a mode.
 * Maybe I'll think of a better way some day.
*/
  case GDK_l:
  case GDK_L:
    action = PCPLOT;
  break;

  case GDK_a:
  case GDK_A:
    action = SCATMAT;
  break;

/* */

  case GDK_d:
  case GDK_D:
    action = P1PLOT;
  break;
  case GDK_x:
  case GDK_X:
    action = XYPLOT;
  break;
  case GDK_t:
  case GDK_T:
    action = TOUR1D;
  break;
  case GDK_g:
  case GDK_G:
    action = TOUR2D;
  break;
  case GDK_c:
  case GDK_C:
    action = COTOUR;
  break;
  case GDK_s:
  case GDK_S:
    action = SCALE;
  break;
  case GDK_b:
  case GDK_B:
    action = BRUSH;
  break;
  case GDK_i:
  case GDK_I:
    action = IDENT;
  break;
#ifdef EDIT_EDGES_IMPLEMENTED
  case GDK_e:
  case GDK_E:
    action = EDGEED;
  break;
#endif
  case GDK_m:
  case GDK_M:
    action = MOVEPTS;
  break;
  default:
    /* g_printerr ("splot key_press: %d\n", event->keyval); */
    common_event = false;
  }
  }

  if (action >= 0 &&
      display_type_handles_action (display, (PipelineMode) action))
  {
    etime = event->time;
    GGOBI(full_viewmode_set)((PipelineMode) action, gg);
  }

  return common_event;
}


void
sp_event_handlers_toggle (splotd *sp, gboolean state) 
{
  displayd *display = (displayd *) sp->displayptr;
  gint m = display->cpanel.viewmode;

  if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
    GtkGGobiExtendedDisplayClass *klass;
    klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
    if(klass->event_handlers_toggle && klass->event_handlers_toggle(display, sp, state, m) == false)
      return;
  }

  switch (m) {
    case P1PLOT:
      p1d_event_handlers_toggle (sp, state);
    break;

    case XYPLOT:
      xyplot_event_handlers_toggle (sp, state);
    break;

#ifdef ROTATION_IMPLEMENTED
    case ROTATE:
      rotation_event_handlers_toggle (sp, state);
    break;
#endif

    case SCALE:
      scale_event_handlers_toggle (sp, state);
    break;

    case TOUR1D:
      tour1d_event_handlers_toggle (sp, state);
    break;

    case TOUR2D:
      tour2d_event_handlers_toggle (sp, state);
    break;

    case COTOUR:
      ctour_event_handlers_toggle (sp, state);
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

    case SCATMAT:
      switch (sp->p1dvar) {
        case -1:
          xyplot_event_handlers_toggle (sp, state);
        break;
        default:
          p1d_event_handlers_toggle (sp, state);
      }
    break;

    case PCPLOT:
      p1d_event_handlers_toggle (sp, state);
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

    sp_event_handlers_toggle (sp, state);
    viewmode_activate (sp, cpanel->viewmode, state, gg);

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
  PipelineMode prev_viewmode = gg->viewmode;

  if (sp != sp_prev) {
    if (sp_prev != NULL) {
      splot_set_current (sp_prev, off, gg);
      display_prev = (displayd *) sp_prev->displayptr;
      cpanel = &display_prev->cpanel;

      /*
       * This feels like a kludge, but I don't know where else
       * to do it.  We want to handle a special case:  we're
       * brushing in a multi-plot display, and we move to a new
       * splot within the same display.  
       * In the future, there may be other things we want to undo,
       * but for now we just want to turn off the effects of
       * in the previous splot.
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

    viewmode_submenus_update (prev_viewmode, display_prev, gg);

    /*
     * if the previous splot is in transient brushing mode, a FULL
     * redraw is required.
     *
     * if the previous splot is in identify, a QUICK redraw is required
     *
     * otherwise, just redraw the borders of the two affected splots
    */
    if (prev_viewmode == NULLMODE || cpanel == NULL)
      displays_plot (NULL, FULL, gg);
    if (prev_viewmode == BRUSH && cpanel->br_mode == BR_TRANSIENT)
      displays_plot (NULL, FULL, gg);
    else if (prev_viewmode == IDENT)
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
splot_add_rows (gint nrows, splotd *sp)
{
  vectorf_realloc (&sp->p1d.spread_data, nrows);

  sp->planar = (gcoords *) g_realloc (sp->planar,
    nrows * sizeof (gcoords));
  sp->screen = (icoords *) g_realloc (sp->screen,
    nrows * sizeof (icoords));
}

void
splot_edges_realloc (splotd *sp, datad *e, ggobid *gg) 
{
  sp->edges = (GdkSegment *) g_realloc ((gpointer) sp->edges,
    e->edge.n * sizeof (GdkSegment));
  sp->arrowheads = (GdkSegment *) g_realloc ((gpointer) sp->arrowheads,
    e->edge.n * sizeof (GdkSegment));
}

void
splot_alloc (splotd *sp, displayd *display, ggobid *gg) 
{
  datad *d;
  gint nr; 
  if(!display)
    return;
  d = display->d;
  nr = d->nrows;
  sp->planar = (gcoords *) g_malloc (nr * sizeof (gcoords));
  sp->screen = (icoords *) g_malloc (nr * sizeof (icoords));
  vectorf_init_null (&sp->p1d.spread_data);
  vectorf_alloc (&sp->p1d.spread_data, nr);

  if(GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
    GtkGGobiExtendedSPlotClass *klass;
    klass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT(sp)->klass);
    if(klass->alloc_whiskers)
      sp->whiskers = klass->alloc_whiskers(sp, nr, d);
  }
}

void
splot_free (splotd *sp, displayd *display, ggobid *gg) 
{

  gtk_widget_hide (sp->da);

  g_free ((gpointer) sp->planar);
  g_free ((gpointer) sp->screen);
  vectorf_free (&sp->p1d.spread_data);

  if(GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
     gtk_object_destroy(GTK_OBJECT(sp));
  } else
     gtk_widget_destroy (GTK_WIDGET(sp));
}

void
splot_dimension_set (splotd* sp, gint width, gint height)
{
  sp->max.x = width;
  sp->max.y = height;

  sp->pmid.x = sp->pmid.y = 0;

  if (sp->da != NULL && width != -1 && height != -1) {
    gtk_drawing_area_size (GTK_DRAWING_AREA (sp->da), width, height);
  }
}

splotd *
splot_new (displayd *display, gint width, gint height, ggobid *gg) 
{
  splotd *sp;

  sp = gtk_type_new(GTK_TYPE_GGOBI_SPLOT);
  splot_init(sp, display, width, height, gg);

  return(sp);
}

void
splot_init(splotd *sp, displayd *display, gint width, gint height, ggobid *gg) 
{
/*
 * Initialize the widget portion of the splot object
*/

  brush_pos_init (sp);
  
  splot_dimension_set (sp, width, height);

  /*
   * Let it be possible to get a pointer to the splotd object 
   * from the drawing area; and to gg as well.
  */
  gtk_object_set_data (GTK_OBJECT (sp->da), "splotd", (gpointer) sp);
  GGobi_widget_set (sp->da, gg, true);


  gtk_signal_connect (GTK_OBJECT (sp->da),
                      "expose_event",
                      (GtkSignalFunc) splot_expose_cb,
                      (gpointer) sp);
  gtk_signal_connect (GTK_OBJECT (sp->da),
                      "configure_event",
                      (GtkSignalFunc) splot_configure_cb,
                      (gpointer) sp);
  gtk_signal_connect (GTK_OBJECT (sp->da),
                      "button_press_event",
                      (GtkSignalFunc) splot_set_current_cb,
                      (gpointer) sp);

  gtk_widget_set_events (sp->da, GDK_EXPOSURE_MASK
             | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
             | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);




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
  sp->tour2d.initmax = true;
  sp->tourcorr.initmax = true;

  gtk_signal_emit(GTK_OBJECT(gg), GGobiSignals[SPLOT_NEW_SIGNAL], sp);
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
  datad *d = display->d;

/*
 * This may be the place to respond to the possibility that a
 * plotted variable has just been deleted.  It's no big deal for
 * the scatterplot -- unless one of the plotted variables is now
 * beyond d->ncols.
*/

  if(GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
      GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT(sp)->klass)->world_to_plane(sp, d, gg);
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
  datad *d = display->d;
  greal gtmp;
  GtkGGobiExtendedSPlotClass *klass = NULL;
  greal precis = (greal) PRECISION1;

  if(GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
     klass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT(sp)->klass);
    
     if(klass->plane_to_screen) {
        klass->plane_to_screen(sp, d, gg);
        return;
     }
  }

  scale_x = (greal) (cpanel->projection == TOUR2D) ?
    sp->tour_scale.x : sp->scale.x;
  scale_y = (greal) (cpanel->projection == TOUR2D) ?
    sp->tour_scale.y : sp->scale.y;

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
    i = d->rows_in_plot[k];

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
 * without have any impact on the pipeline data structures.
*/
void
splot_screen_to_tform (cpaneld *cpanel, splotd *sp, icoords *scr,
  fcoords *tfd, ggobid *gg)
{
  gcoords planar, world;
  greal precis = (greal) PRECISION1;
  greal ftmp, max, min, rdiff;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gfloat scale_x, scale_y;
  vartabled *vt, *vtx, *vty;

  g_return_if_fail (cpanel->projection == XYPLOT ||
                    cpanel->projection == P1PLOT ||
                    cpanel->projection == TOUR1D ||
                    cpanel->projection == TOUR2D ||
                    cpanel->projection == COTOUR);


  scale_x = sp->scale.x;
  scale_y = sp->scale.y;
  scale_x /= 2;
  sp->iscale.x = (greal) sp->max.x * scale_x;
  scale_y /= 2;
  sp->iscale.y = -1 * (greal) sp->max.y * scale_y;

/*
 * screen to plane 
*/
  planar.x = (scr->x - sp->max.x/2) * precis / sp->iscale.x ;
  planar.x += sp->pmid.x;
  planar.y = (scr->y - sp->max.y/2) * precis / sp->iscale.y ;
  planar.y += sp->pmid.y;

/*
 * plane to world
*/

  switch (cpanel->projection) {
    case P1PLOT:
      vt = vartable_element_get (sp->p1dvar, d);
      max = vt->lim.max;
      min = vt->lim.min;
      rdiff = max - min;

      if (display->p1d_orientation == HORIZONTAL) {
        /* x */
        world.x = planar.x;
        ftmp = world.x / precis;
        tfd->x = (ftmp + 1.0) * .5 * rdiff;
        tfd->x += min;
      } else {
        /* y */
        world.y = planar.y;
        ftmp = world.y / precis;
        tfd->y = (ftmp + 1.0) * .5 * rdiff;
        tfd->y += min;
      }
    break;

    case XYPLOT:
      /* x */
      vtx = vartable_element_get (sp->xyvars.x, d);
      max = vtx->lim.max;
      min = vtx->lim.min;
      rdiff = max - min;
      world.x = planar.x;
      ftmp = world.x / precis;
      tfd->x = (ftmp + 1.0) * .5 * rdiff;
      tfd->x += min;

      /* y */
      vty = vartable_element_get (sp->xyvars.y, d);
      max = vty->lim.max;
      min = vty->lim.min;
      rdiff = max - min;
      world.y = planar.y;
      ftmp = world.y / precis;
      tfd->y = (ftmp + 1.0) * .5 * rdiff;
      tfd->y += min;
    break;


    case TOUR2D:
    break;

    case TOUR1D:
    break;

    case COTOUR:
    break;

    default:
    break;
  }
}

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

/*
 * All this code shouldn't be necessary, because (eg) if we're
 * in tour and using tour_scale, we should already know that.
 * Look through the movepts code and see if I'm inappropriately
 * resetting iscale somewhere.
*/
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  gfloat scale_x, scale_y;
  scale_x = (cpanel->projection == TOUR2D) ? sp->tour_scale.x : sp->scale.x;
  scale_y = (cpanel->projection == TOUR2D) ? sp->tour_scale.y : sp->scale.y;
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

void
splot_plane_to_world (splotd *sp, gint ipt, ggobid *gg) 
{ 
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  datad *d = display->d;

  switch (cpanel->projection) {
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
      /*if (!gg->is_pp) {*/
        for (j=0; j<display->t1d.nactive; j++) {
          var = display->t1d.active_vars.els[j];
          d->world.vals[ipt][var] += 
           (gg->movepts.eps.x * (greal) display->t1d.F.vals[0][var]);
        }
      /*}*/
    }
    break;

    case TOUR2D:
    {
      gint j, var;
      /*if (!gg->is_pp) {*/
        for (j=0; j<display->t2d.nactive; j++) {
          var = display->t2d.active_vars.els[j];
          d->world.vals[ipt][var] += 
           (gg->movepts.eps.x * (greal) display->t2d.F.vals[0][var] +
            gg->movepts.eps.y * (greal) display->t2d.F.vals[1][var]);
        }
      /*}*/
    }
    break;

    case COTOUR:
    {
      gint j, var;
      /*if (!gg->is_pp) {*/
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
      /*}*/
    }

    break;

    default:
      g_printerr ("reverse pipeline not yet implemented for this projection\n");
  }
}

void
splot_reverse_pipeline (splotd *sp, gint ipt, gcoords *eps,
                        gboolean horiz, gboolean vert, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;

  splot_screen_to_plane (sp, ipt, eps, horiz, vert);
  splot_plane_to_world (sp, ipt, gg);

  /*-- for bivariate plots only --*/
  /*-- in pipeline.c since it applies to the front of the pipeline --*/
  world_to_raw (ipt, sp, d, gg);
}

/* ---------------------------------------------------------------------*/
/*          Pack up some of short signal routines                       */
/* ---------------------------------------------------------------------*/

/*-- this one isn't attached to sp->da, but we'll bundle it anyway --*/
void
disconnect_key_press_signal (splotd *sp) {
  displayd *display = sp->displayptr;
  if (sp->key_press_id && GTK_IS_GGOBI_WINDOW_DISPLAY(display)) {
    gtk_signal_disconnect (GTK_OBJECT (GTK_GGOBI_WINDOW_DISPLAY(display)->window), sp->key_press_id);
    sp->key_press_id = 0;
  }
}

void
disconnect_button_press_signal (splotd *sp) 
{
  if (sp->press_id) {
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->press_id);
    sp->press_id = 0;
  }
}

void
disconnect_button_release_signal (splotd *sp) {
  if (sp->release_id) {
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->release_id);
    sp->release_id = 0;
  }
}
void
disconnect_motion_signal (splotd *sp) {
  if (sp->motion_id) {
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->motion_id);
    sp->motion_id = 0;
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
    gdk_window_set_cursor (window, NULL);
  } else {
    sp->jcursor = (gint) jcursor;
    sp->cursor = gdk_cursor_new (sp->jcursor);
    gdk_window_set_cursor (window, sp->cursor);
  }
}
