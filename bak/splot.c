/* splot.c: an individual scatterplot */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"

/* external functions */
extern void init_plot_GC ();
extern void splot_plot (cpaneld *, splotd *);
extern void xy_reproject (splotd *);
extern void p1d_reproject (splotd *);
extern void display_set_current (displayd *);
extern void splot_toggle_rotation_handlers (splotd *, gboolean);
extern void splot_toggle_gtour_handlers (splotd *, gboolean);
extern void splot_toggle_ctour_handlers (splotd *, gboolean);
extern void splot_toggle_scale_handlers (splotd *, gboolean);
extern void splot_toggle_brush_handlers (splotd *, gboolean);
extern void splot_toggle_lineedit_handlers (splotd *, gboolean);
extern void splot_toggle_movepts_handlers (splotd *, gboolean);

void splot_world_to_plane (cpaneld *, splotd *);
void splot_plane_to_screen (cpaneld *, splotd *);
/*                    */

/**********************************************************************/
/***************************** Events *********************************/
/**********************************************************************/

/* Create a new backing pixmap of the appropriate size */
static gint
splot_configure_cb (GtkWidget *widget, GdkEventConfigure *event, splotd *sp)
{
  displayd *display = (displayd *) sp->displayptr; 
  cpaneld *cpanel = &display->cpanel;

  if (sp->pixmap != null)
    gdk_pixmap_unref (sp->pixmap);

  sp->pixmap = gdk_pixmap_new (widget->window,
              widget->allocation.width, widget->allocation.height,
              -1);

  sp->max.x = widget->allocation.width;
  sp->max.y = widget->allocation.height;
  sp->mid.x = sp->max.x / 2;
  sp->mid.y = sp->max.y / 2;
  sp->minxy = MIN (sp->max.x, sp->max.y);

  splot_world_to_plane (cpanel, sp);  /* That doesn't need to be here */
  splot_plane_to_screen (cpanel, sp);

  return false;
}

void
splot_add_border (splotd *sp) {
  if (sp != null && sp->da != null && sp->da->window != null) {
    gdk_gc_set_foreground (plot_GC, &xg.accent_color);
    gdk_gc_set_line_attributes (plot_GC,
      3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
    gdk_draw_rectangle (sp->da->window, plot_GC,
      FALSE, 1, 1, sp->da->allocation.width-3, sp->da->allocation.height-3);
    gdk_gc_set_line_attributes (plot_GC,
      0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  }
}

static gint
splot_expose_cb (GtkWidget *w, GdkEventExpose *event, splotd *sp)
{
  displayd *display = (displayd *) sp->displayptr; 
  cpaneld *cpanel = &display->cpanel;

  splot_plot (cpanel, sp);

  if (sp == current_splot)
    splot_add_border (sp);

  return false;
}

/*
static gint key_press_cb (GtkWidget *w, GdkEventKey *event)
{
  g_printerr ("%d\n", w != NULL);
  g_printerr ("%d\n", GTK_IS_WIDGET (w));
  g_printerr ("%d\n", event != NULL);

  g_printerr ("splot key_press: %d\n", event->keyval);
  return true;
}
*/

void
splot_set_current (splotd *sp, gboolean state) {
/*
 * Turn on or off the event handlers in sp
*/
  if (sp != null) {
    displayd *display = (displayd *) sp->displayptr;
    cpaneld *cpanel = &display->cpanel;
    activate_mode (sp, cpanel->mode, state);
  }
}


static gint
splot_set_current_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  splotd *prev_splot = current_splot;
  displayd *display = (displayd *) sp->displayptr; 
  GtkWidget *da;

  if (sp != current_splot) {

    if (current_splot != null)
      da = current_splot->da;

    if (current_display != display)
      display_set_current (display);  /* old one off, new one on */

    /*
     * Copy the backing pixmap to the window for the
     * previous splot: this is solely to eliminate the
     * border.
    */
    if (prev_splot != null && da != null && da->window != null)
      gdk_draw_pixmap (da->window, plot_GC, current_splot->pixmap,
                       0, 0, 0, 0,
                       da->allocation.width, da->allocation.height);

    current_splot = sp;
    splot_add_border (sp);
  }
  splot_current = sp;

  return false;  /* so that other button press handlers also get the event */
}

void
splot_alloc (splotd *sp) {
  gint nr = xg.nrows;

  sp->planar = (lcoords *) g_malloc (nr * sizeof (lcoords));
  sp->screen = (icoords *) g_malloc (nr * sizeof (icoords));

  sp->points = (GdkPoint *) g_malloc (nr * sizeof (GdkPoint));
  sp->segs = (GdkSegment *) g_malloc (2 * nr * sizeof (GdkSegment));
  sp->open_rects = (rectd *) g_malloc (nr * sizeof (rectd));
  sp->filled_rects = (rectd *) g_malloc (nr * sizeof (rectd));
  sp->open_arcs = (arcd *) g_malloc (nr * sizeof (arcd));
  sp->filled_arcs = (arcd *) g_malloc (nr * sizeof (arcd));
}

void
splot_free (splotd *sp) {
  /*
   * turn off event handlers
  */
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  mode_event_handlers_toggle (sp, cpanel->mode, off);

  gtk_widget_hide (sp->da);

  g_free ((gpointer) sp->planar);
  g_free ((gpointer) sp->screen);
  g_free ((gpointer) sp->points);
  g_free ((gpointer) sp->segs);
  g_free ((gpointer) sp->open_rects);
  g_free ((gpointer) sp->filled_rects);
  g_free ((gpointer) sp->open_arcs);
  g_free ((gpointer) sp->filled_arcs);

  gtk_widget_destroy (sp->da);

  g_free ((gpointer) sp);
}

splotd *
splot_new (displayd *display, gint width, gint height) {

  splotd *sp = (splotd *) g_malloc (sizeof (splotd));

/*
 * Initialize the widget portion of the splot object
*/
  sp->da = gtk_drawing_area_new ();

  /*
   * Let it be possible to get a pointer to the splotd object 
   * from the drawing area.
  */
  gtk_object_set_data (GTK_OBJECT (sp->da), "splotd", (gpointer) sp);

  if (width != -1 && height != -1)
    gtk_drawing_area_size (GTK_DRAWING_AREA (sp->da), width, height);

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
             | GDK_LEAVE_NOTIFY_MASK
             | GDK_BUTTON_PRESS_MASK
             | GDK_BUTTON_RELEASE_MASK
             | GDK_POINTER_MOTION_MASK
             | GDK_POINTER_MOTION_HINT_MASK);


/*
 * Initialize the data portion of the splot object
*/

  splot_alloc (sp);

  sp->displayptr = (gpointer) display;
  sp->pixmap = null;

  sp->p1dvar = 0;
  sp->p1d_data = null;

  sp->xyvars.x = 0;
  sp->xyvars.y = 1;

  sp->scale.x = sp->scale.y = .7;
  sp->shift_wrld.x = sp->shift_wrld.y = 0;

  sp->tour_scale.x = sp->tour_scale.y = .6;

  return sp;
}

void
splot_get_dimensions (splotd *sp, gint *width, gint *height) {
  *width = sp->da->allocation.width;
  *height = sp->da->allocation.height;
}

/**************** pipeline for scatterplot **********************/

void
splot_set_plot_center (splotd *sp)
{
  sp->cntr.x = sp->mid.x + (gint) ((sp->shift_wrld.x * sp->is.x) >> EXP1);
  sp->cntr.y = sp->mid.y - (gint) ((sp->shift_wrld.y * sp->is.y) >> EXP1);
}

void
splot_world_to_plane (cpaneld *cpanel, splotd *sp)
/*
 * project the data from world_data[],
 * the data expressed in 'world coordinates,' to planar[], the
 * data expressed in 'projection coordinates.'
*/
{
  switch (cpanel->projection) {
  /*
   * single plots
  */

/*
    case grtour:
      all_tour_reproject (xg);
      break;

    case cotour:
      corr_reproject (xg);
      break;

    case ROTATE:
      if (cpanel->spin_type.yaxis || cpanel->spin_type.xaxis)
        ax_rot_reproject ();
      else if (cpanel->spin_type.oblique)
        ob_rot_reproject ();
      break;
*/

    case XYPLOT:
      xy_reproject (sp);
      break;

    case P1PLOT:
      p1d_reproject (sp);
      break;

  /*
   * plot arrangements
  */
    case SCATMAT:
      if (sp->p1dvar == -1)
        xy_reproject (sp);
      else
        p1d_reproject (sp);
      break;

    case PCPLOT:
      p1d_reproject (sp);
      break;
  }
}

void
splot_plane_to_screen (cpaneld *cpanel, splotd *sp)
/*
 * Use the data in 'projection coordinates' and rescale it to the
 * dimensions of the current plotting window, writing it into screen.
 * At the same time, update segs.
*/
{
  gint i, k;
  glong nx, ny;
  gfloat scale_x, scale_y;

  scale_x = (cpanel->projection == GRTOUR) ? sp->tour_scale.x : sp->scale.x;
  scale_y = (cpanel->projection == GRTOUR) ? sp->tour_scale.y : sp->scale.y;

  /*
   * Calculate is, a scale factor.  Either force the plot to be
   * square or scale so as to use the entire plot window.  (Or
   * as much of the plot window as scale.x and scale.y permit.)
  */
  scale_x /= 2;
  sp->is.x = (glong) ((gfloat)sp->max.x * scale_x);
  scale_y /= 2;
  sp->is.y = (glong) (-1 * (gfloat)sp->max.y * scale_y);


  /*
   * Calculate new coordinates.
  */
  for (k=0; k<xg.nrows_in_plot; k++) {
    i = xg.rows_in_plot[k];

    /*
     * shift in world coords
    */
    nx = sp->planar[i].x + sp->shift_wrld.x;
    ny = sp->planar[i].y - sp->shift_wrld.y;

    /*
     * scale from world to plot window and expand-contract as desired
    */
    sp->screen[i].x = (gint) ((nx * sp->is.x) >> EXP1);
    sp->screen[i].y = (gint) ((ny * sp->is.y) >> EXP1);

    /*
     * shift into middle of plot window 
    */
    sp->screen[i].x += sp->mid.x;
    sp->screen[i].y += sp->mid.y;
  }
}

/**************** end of pipeline for scatterplot **********************/

/****************** reverse pipeline for scatterplot ********************/

/* 
 * In order to compute the limits for the rulers, I need to
 * run a pair of points through the reverse pipeline -- but
 * without have any impact on the pipeline data structures.
*/
void
splot_screen_to_tform (cpaneld *cpanel, splotd *sp, icoords *scr, fcoords *tfd)
{
  lcoords planar, world;
  gint var;
  gfloat precis = PRECISION1;
  gfloat ftmp, rdiff;
  displayd *display = (displayd *) sp->displayptr;

  g_return_if_fail (cpanel->projection == XYPLOT ||
                    cpanel->projection == P1PLOT);

/*
 * screen to plane 
*/
  splot_set_plot_center (sp);
  planar.x = (scr->x - sp->cntr.x) * PRECISION1 / sp->is.x ;
  planar.y = (scr->y - sp->cntr.y) * PRECISION1 / sp->is.y ;

/*
 * plane to world
*/
  switch (cpanel->projection) {
    case P1PLOT:
      if (display->p1d_orientation == HORIZONTAL) {
        /* x */
        world.x = planar.x;
        rdiff = xg.lim[sp->p1dvar].max - xg.lim[sp->p1dvar].min;
        ftmp = world.x / precis;
        tfd->x = (ftmp + 1.0) * .5 * rdiff;
        tfd->x += xg.lim[sp->p1dvar].min;
      } else {
        /* y */
        world.y = planar.y;
        rdiff = xg.lim[sp->p1dvar].max - xg.lim[sp->p1dvar].min;
        ftmp = world.y / precis;
        tfd->y = (ftmp + 1.0) * .5 * rdiff;
        tfd->y += xg.lim[sp->p1dvar].min;
      }
      break;

    case XYPLOT:
      /* x */
      world.x = planar.x;
      var = sp->xyvars.x;
      rdiff = xg.lim[var].max - xg.lim[var].min;
      ftmp = world.x / precis;
      tfd->x = (ftmp + 1.0) * .5 * rdiff;
      tfd->x += xg.lim[var].min;

      /* y */
      world.y = planar.y;
      var = sp->xyvars.y;
      rdiff = xg.lim[var].max - xg.lim[var].min;
      ftmp = world.y / precis;
      tfd->y = (ftmp + 1.0) * .5 * rdiff;
      tfd->y += xg.lim[var].min;

      break;

    default:
      break;
  }

}

void
splot_screen_to_plane (splotd *sp, gint pt, lcoords *eps,
  gboolean horiz, gboolean vert)
{
  icoords prev_planar;
  splot_set_plot_center (sp);

  if (horiz) {
    sp->screen[pt].x -= sp->cntr.x;
    prev_planar.x = sp->planar[pt].x;
    sp->planar[pt].x = sp->screen[pt].x * PRECISION1 / sp->is.x ;
    eps->x = sp->planar[pt].x - prev_planar.x;
  }

  if (vert) {
    sp->screen[pt].y -= sp->cntr.y;
    prev_planar.y = sp->planar[pt].y;
    sp->planar[pt].y = sp->screen[pt].y * PRECISION1 / sp->is.y ;
    eps->y = sp->planar[pt].y - prev_planar.y;
  }
}

void
splot_plane_to_world (cpaneld *cpanel, splotd *sp, gint ipt, lcoords *eps) 
{ 

  switch (cpanel->projection) {
    case XYPLOT:
      xg.world_data[ipt][sp->xyvars.x] = sp->planar[ipt].x;
      xg.world_data[ipt][sp->xyvars.y] = sp->planar[ipt].y;
      break;

    default:
      g_printerr ("reverse pipeline only implemented for xyplotting\n");
  }
}


void
splot_reverse_pipeline (cpaneld *cpanel, splotd *sp, gint ipt,
                     gboolean horiz, gboolean vert)
{
  lcoords eps;

  splot_screen_to_plane (sp, ipt, &eps, horiz, vert);
  splot_plane_to_world (cpanel, sp, ipt, &eps);
  /*world_to_raw (ipt);*/
}

/************ end of reverse pipeline for scatterplot ********************/
