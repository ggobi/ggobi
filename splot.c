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
  gfloat ftmp;
  gg = sp->displayptr->ggobi;

  /*
   * Somehow when a new splot is added to a table, the initial
   * configuration event for the drawing_area occurs before the
   * drawing_area has been properly sized.  Maybe I'm not executing
   * calls in the proper order?  This protects me in the meantime.
  */
  if (w->allocation.width == 1 || w->allocation.height == 1)
    return false;

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

  /*-- rescale ishift --*/
  ftmp = (gfloat) w->allocation.width / (gfloat) sp->max.x;
  sp->ishift.x = (gint) (ftmp * (gfloat) sp->ishift.x);
  ftmp = (gfloat) w->allocation.height / (gfloat) sp->max.y;
  sp->ishift.y = (gint) (ftmp * (gfloat) sp->ishift.y);

  sp->max.x = w->allocation.width;
  sp->max.y = w->allocation.height;

  splot_plane_to_screen (display, cpanel, sp, gg);

  if (mode_get (gg) == BRUSH)
    assign_points_to_bins (d, gg);

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
  if (w->allocation.width == 1 || w->allocation.height == 1)
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
  extern gint GGOBI(full_mode_set)(gint action, ggobid *gg);
  gint action = -1;
  displayd *display = (displayd *) sp->displayptr;
  extern gboolean display_type_handles_action (displayd *, gint viewmode);

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
  case GDK_p:
  case GDK_P:
    action = PCPLOT;
  break;
  case GDK_v:
  case GDK_V:
    action = TSPLOT;
  break;
  case GDK_e:
  case GDK_E:
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
  case GDK_g:
  case GDK_G:
    action = TOUR1D;
  break;
  case GDK_t:
  case GDK_T:
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
  case GDK_m:
  case GDK_M:
    action = MOVEPTS;
  break;
  default:
    /* g_printerr ("splot key_press: %d\n", event->keyval); */
    common_event = false;
  }

  if (action >= 0 && display_type_handles_action (display, action)) {
    etime = event->time;
    GGOBI(full_mode_set)(action, gg);
  }

  return common_event;
}


void
sp_event_handlers_toggle (splotd *sp, gboolean state) {
  displayd *display = (displayd *) sp->displayptr;
  gint m = display->cpanel.mode;

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

    case TOUR1D:
      tour1d_event_handlers_toggle (sp, state);
    break;

    case TOUR2D:
      tour2d_event_handlers_toggle (sp, state);
    break;

    case COTOUR:
      ctour_event_handlers_toggle (sp, state);
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

    case TSPLOT:
      xyplot_event_handlers_toggle (sp, state);  /*-- ?? --*/
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
    mode_activate (sp, cpanel->mode, state, gg);
    mode_submenus_activate (sp, cpanel->mode, state, gg);

    /*
     * this is now the only place varpanel_refresh is called in
     * changing the current display and splot; we'll see if it's
     * adequate
    */
    if (state == on) {
      varpanel_refresh (gg);
    }

    /*-- only required after leaving an splot in transient brushing --*/
    displays_plot (NULL, FULL, gg);
  }
}

void
GGOBI(splot_set_current_full)(displayd *display, splotd *sp, ggobid *gg)
{
  splotd *sp_prev = gg->current_splot;
  if (sp != sp_prev) {

   if (sp_prev != NULL) {
    splot_set_current (sp_prev, off, gg);

    if (gg->current_display != display)
      display_set_current (display, gg);  /* old one off, new one on */
    }

    gg->current_splot = sp;

    /* add border to current_splot */
    sp->redraw_style = QUICK;
    gtk_widget_queue_draw (sp->da);

    splot_set_current (sp, on, gg);

    /* remove border from the previous splot */
    if (sp_prev != NULL) {
      sp_prev->redraw_style = QUICK;
      gtk_widget_queue_draw (sp_prev->da);
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
  vectorf_realloc (&sp->p1d_data, nrows);

  sp->planar = (lcoords *) g_realloc (sp->planar,
    nrows * sizeof (lcoords));
  sp->screen = (icoords *) g_realloc (sp->screen,
    nrows * sizeof (icoords));
}

void
splot_alloc (splotd *sp, displayd *display, ggobid *gg) {
  datad *d = display->d;
  gint nr = d->nrows;
  gint nl = d->edge.n;

  sp->planar = (lcoords *) g_malloc (nr * sizeof (lcoords));
  sp->screen = (icoords *) g_malloc (nr * sizeof (icoords));
  vectorf_init (&sp->p1d_data);
  vectorf_alloc (&sp->p1d_data, nr);

  switch (display->displaytype) {
    case scatterplot:
      sp->edges = (GdkSegment *) g_malloc (nl * sizeof (GdkSegment));
      sp->arrowheads = (GdkSegment *) g_malloc (nl * sizeof (GdkSegment));
      break;
    case scatmat:
      sp->edges = (GdkSegment *) g_malloc (nl * sizeof (GdkSegment));
      sp->arrowheads = (GdkSegment *) g_malloc (nl * sizeof (GdkSegment));
      break;
    case parcoords:
      sp->whiskers = (GdkSegment *) g_malloc (2 * nr * sizeof (GdkSegment));
      break;
   case tsplot:
      sp->whiskers = (GdkSegment *) g_malloc ((nr-1) * sizeof (GdkSegment));
      break;
  }
}

void
splot_edges_realloc (splotd *sp, datad *d, ggobid *gg) {
  sp->edges = (GdkSegment *) g_realloc ((gpointer) sp->edges,
    d->edge.n * sizeof (GdkSegment));
  sp->arrowheads = (GdkSegment *) g_realloc ((gpointer) sp->arrowheads,
    d->edge.n * sizeof (GdkSegment));
}

void
splot_free (splotd *sp, displayd *display, ggobid *gg) {

  gtk_widget_hide (sp->da);

  g_free ((gpointer) sp->planar);
  g_free ((gpointer) sp->screen);
  vectorf_free (&sp->p1d_data);

  switch (display->displaytype) {
    case scatterplot:
      g_free ((gpointer) sp->edges);
      g_free ((gpointer) sp->arrowheads);
      break;
    case scatmat:
      g_free ((gpointer) sp->edges);
      g_free ((gpointer) sp->arrowheads);
      break;
    case parcoords:
      g_free ((gpointer) sp->whiskers);
      break;
    case tsplot:
      g_free ((gpointer) sp->whiskers);
      break;
  }

  gtk_widget_destroy (sp->da);

  g_free ((gpointer) sp);
}

void
splot_dimension_set (splotd* sp, gint width, gint height)
{
  sp->max.x = width;
  sp->max.y = height;
  sp->ishift.x = sp->max.x / 2;
  sp->ishift.y = sp->max.y / 2;

  if (sp->da != NULL && width != -1 && height != -1)
    gtk_drawing_area_size (GTK_DRAWING_AREA (sp->da), width, height);

}

splotd *
splot_new (displayd *display, gint width, gint height, ggobid *gg) {
  splotd *sp = (splotd *) g_malloc (sizeof (splotd));

/*
 * Initialize the widget portion of the splot object
*/
  sp->da = gtk_drawing_area_new ();
  sp->pixmap0 = NULL;
  sp->pixmap1 = NULL;

  brush_pos_init (sp);
  
  splot_dimension_set (sp, width, height);

  /*
   * Let it be possible to get a pointer to the splotd object 
   * from the drawing area.
  */
  gtk_object_set_data (GTK_OBJECT (sp->da), "splotd", (gpointer) sp);


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

  return sp;
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

  switch (display->displaytype) {

    case scatterplot:
      switch (cpanel->projection) {
        case P1PLOT:
          p1d_reproject (sp,
            (display->missing_p) ? d->missing_world.vals : d->world.vals,
            d, gg);
        break;

        case XYPLOT:
          xy_reproject (sp,
            (display->missing_p) ? d->missing_world.vals : d->world.vals,
            d, gg);
        break;

        case TOUR1D:
          tour1d_projdata (sp,
            (display->missing_p) ? d->missing_world.vals : d->world.vals,
            d, gg);
        break;

        case TOUR2D:
          tour2d_projdata(sp,
            (display->missing_p) ? d->missing_world.vals : d->world.vals,
            d, gg);
        break;

        case COTOUR:
          tourcorr_projdata(sp,
            (display->missing_p) ? d->missing_world.vals : d->world.vals,
            d, gg);
        break;
      }
      break;

    case scatmat:

      if (sp->p1dvar == -1)
        xy_reproject (sp,
          (display->missing_p) ? d->missing_world.vals : d->world.vals,
          d, gg);
      else
        p1d_reproject (sp,
          (display->missing_p) ? d->missing_world.vals : d->world.vals,
          d, gg);
      break;

    case parcoords:
      p1d_reproject (sp,
        (display->missing_p) ? d->missing_world.vals : d->world.vals,
        d, gg);
      break;

    case tsplot:
      xy_reproject (sp,
        (display->missing_p) ? d->missing_world.vals : d->world.vals,
        d, gg);
      break;
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
  gfloat scale_x, scale_y;
  datad *d = display->d;

  scale_x = (cpanel->projection == TOUR2D) ? sp->tour_scale.x : sp->scale.x;
  scale_y = (cpanel->projection == TOUR2D) ? sp->tour_scale.y : sp->scale.y;

  /*
   * Calculate is, a scale factor.  Scale so as to use the entire
   * plot window (well, as much of the plot window as scale.x and
   * scale.y permit.)
  */
  scale_x /= 2;
  sp->iscale.x = (glong) ((gfloat) sp->max.x * scale_x);
  scale_y /= 2;
  sp->iscale.y = (glong) (-1 * (gfloat) sp->max.y * scale_y);

  /*
   * Calculate new coordinates.
  */
  for (k=0; k<d->nrows_in_plot; k++) {
    i = d->rows_in_plot[k];

    /*-- scale from world to plot window --*/
    sp->screen[i].x = (gint) ((sp->planar[i].x * sp->iscale.x) >> EXP1);
    sp->screen[i].y = (gint) ((sp->planar[i].y * sp->iscale.y) >> EXP1);

    /*-- shift into middle of plot window --*/
    sp->screen[i].x += sp->ishift.x;
    sp->screen[i].y += sp->ishift.y;
  }

  if (display->displaytype == parcoords) {
    sp_whiskers_make (sp, display, gg);
  }
  else if (display->displaytype == tsplot) {
    extern void tsplot_whiskers_make (splotd *, displayd *, ggobid *);
    tsplot_whiskers_make (sp, display, gg);
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
  lcoords planar, world;
  gfloat precis = PRECISION1;
  gfloat ftmp, max, min, rdiff;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;

  g_return_if_fail (cpanel->projection == XYPLOT ||
                    cpanel->projection == P1PLOT ||
                    cpanel->projection == TOUR1D ||
                    cpanel->projection == TOUR2D ||
                    cpanel->projection == COTOUR);

/*
 * screen to plane 
*/
  planar.x = (scr->x - sp->ishift.x) * PRECISION1 / sp->iscale.x ;
  planar.y = (scr->y - sp->ishift.y) * PRECISION1 / sp->iscale.y ;

/*
 * plane to world
*/

  switch (cpanel->projection) {
    case P1PLOT:
      if (display->missing_p) {
        max = d->missing_lim.max;
        min = d->missing_lim.min;
      } else {
        max = d->vartable[sp->p1dvar].lim.max;
        min = d->vartable[sp->p1dvar].lim.min;
      }
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
      if (display->missing_p) {
        max = d->missing_lim.max;
        min = d->missing_lim.min;
      } else {
        max = d->vartable[sp->xyvars.x].lim.max;
        min = d->vartable[sp->xyvars.x].lim.min;
      }
      rdiff = max - min;
      world.x = planar.x;
      ftmp = world.x / precis;
      tfd->x = (ftmp + 1.0) * .5 * rdiff;
      tfd->x += min;

      /* y */
      if (display->missing_p) {
        max = d->missing_lim.max;
        min = d->missing_lim.min;
      } else {
        max = d->vartable[sp->xyvars.y].lim.max;
        min = d->vartable[sp->xyvars.y].lim.min;
      }
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
splot_screen_to_plane (splotd *sp, gint pt, lcoords *eps,
  gboolean horiz, gboolean vert)
{
  icoords prev_planar;

  if (horiz) {
    sp->screen[pt].x -= sp->ishift.x;
    prev_planar.x = sp->planar[pt].x;
    sp->planar[pt].x = sp->screen[pt].x * PRECISION1 / sp->iscale.x ;
    eps->x = sp->planar[pt].x - prev_planar.x;
  }

  if (vert) {
    sp->screen[pt].y -= sp->ishift.y;
    prev_planar.y = sp->planar[pt].y;
    sp->planar[pt].y = sp->screen[pt].y * PRECISION1 / sp->iscale.y ;
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
    case XYPLOT:
      d->world.vals[ipt][sp->xyvars.x] = sp->planar[ipt].x;
      d->world.vals[ipt][sp->xyvars.y] = sp->planar[ipt].y;
      break;

    default:
      g_printerr ("reverse pipeline only implemented for xyplotting\n");
  }
}

void
splot_reverse_pipeline (splotd *sp, gint ipt, lcoords *eps,
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
