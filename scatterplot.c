/* scatterplot.c */
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
#include <gtk/gtk.h>
#include "vars.h"

#include "scatterplotClass.h"

#ifndef GTK_2_0
#include "gtkext.h"
#else
#define GTK_EXT_RULER GTK_RULER
#endif

#include "externs.h"

#define WIDTH   370
#define HEIGHT  370


/*-- as long as these are static, they can probably stay here --*/
static void ruler_shift_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp);
static void ruler_down_cb (GtkWidget *w, GdkEventButton *event, splotd *sp);

void
scatterplot_show_hrule (displayd *display, gboolean show) 
{
  if (show) {
    if (!GTK_WIDGET_VISIBLE (display->hrule))
      gtk_widget_show (display->hrule);
  } else {
    if (GTK_WIDGET_VISIBLE (display->hrule))
      gtk_widget_hide (display->hrule);
  }
}
void
scatterplot_show_vrule (displayd *display, gboolean show) {
  if (show) {
    if (!GTK_WIDGET_VISIBLE (display->vrule))
      gtk_widget_show (display->vrule);
  } else {
    if (GTK_WIDGET_VISIBLE (display->vrule))
      gtk_widget_hide (display->vrule);
  }
}

void
scatterplot_show_rulers (displayd *display, gint projection)
{
/*
 * Retrieve the size of the drawing area before the axes are added
 * or removed, and then set the da size afterwards.  This prevents
 * plots that have been reduced in size from suddenly being resized
 * up to the original default size.
*/
  splotd *sp = display->splots->data;
  gint width = sp->da->allocation.width;
  gint height = sp->da->allocation.height;

  switch (projection) {
    case P1PLOT:
      if (display->p1d_orientation == VERTICAL) {
        scatterplot_show_vrule (display, true);
        scatterplot_show_hrule (display, false);
      } else {
        scatterplot_show_vrule (display, false);
        scatterplot_show_hrule (display, true);
      }
    break;

    case XYPLOT:
      scatterplot_show_vrule (display, true);
      scatterplot_show_hrule (display, true);
    break;

    case TOUR1D:
    case TOUR2D:
    case COTOUR:
    default:  /* in any other projection, no rulers */
      scatterplot_show_vrule (display, false);
      scatterplot_show_hrule (display, false);
    break;
  }

  gtk_drawing_area_size (GTK_DRAWING_AREA (sp->da), width, height);
}

void
ruler_ranges_set (gboolean force, displayd *display, splotd *sp, ggobid *gg) 
{
  /*
   * Run 0 and sp->max through the reverse pipeline to find out
   * what their values should be in terms of the data.  Set the
   * ruler min and max to those values.
   * Force the ranges to be set when a display is being initialized.
  */
  icoords scr;
  fcoords tfmin, tfmax;
  cpaneld *cpanel = &display->cpanel;

  if (display->hrule == NULL)
    return;

  tfmin.x = tfmin.y = tfmax.x = tfmax.y = 0.0;

  scr.x = scr.y = 0;
  splot_screen_to_tform (cpanel, sp, &scr, &tfmin, gg);

  scr.x = sp->max.x;
  scr.y = sp->max.y;
  splot_screen_to_tform (cpanel, sp, &scr, &tfmax, gg);

  /*
   * Reset only if necessary:  if the ruler is visible and the
   * ranges have changed.  Force when initializing display.
  */
  if (force || GTK_WIDGET_VISIBLE (display->hrule)) {
    if (((gfloat) GTK_EXT_RULER (display->hrule)->lower != tfmin.x) ||
        ((gfloat) GTK_EXT_RULER (display->hrule)->upper != tfmax.x))
    {
#ifndef GTK_2_0
      gtk_ext_ruler_set_range (GTK_EXT_RULER (display->hrule),
                               (gdouble) tfmin.x, (gdouble) tfmax.x);
#else
      /* What should the final 2 arguments be. */
      gtk_ruler_set_range (GTK_EXT_RULER (display->hrule),
                               (gdouble) tfmin.x, (gdouble) tfmax.x,
                               (gdouble) (tfmax.x - tfmin.x)/2 + tfmin.x,
                               tfmax.x);
#endif
    }
  }

  if (force || GTK_WIDGET_VISIBLE (display->vrule)) {
    if (((gfloat) GTK_EXT_RULER (display->vrule)->upper != tfmin.y) ||
        ((gfloat) GTK_EXT_RULER (display->vrule)->lower != tfmax.y))
    {
#ifndef GTK_2_0
      gtk_ext_ruler_set_range (GTK_EXT_RULER (display->vrule),
                               (gdouble) tfmax.y, (gdouble) tfmin.y);
#else
      gtk_ruler_set_range (GTK_EXT_RULER (display->hrule),
                               (gdouble) tfmin.y, (gdouble) tfmax.y,
                               (gdouble) (tfmax.y - tfmin.y)/2 + tfmin.y,
                               tfmax.y);
#endif
    }
  }
}

/*----------------------------------------------------------------------*/
/*                          Options section                             */
/*----------------------------------------------------------------------*/

static GtkItemFactoryEntry menu_items[] = {
  {"/_File",        NULL,    NULL,            0,            "<Branch>" },
#ifdef PRINTING_IMPLEMENTED
  {"/File/Print",   "",      
    (GtkItemFactoryCallback)display_print_cb,
    0, "<Item>" },
  {"/File/sep",     NULL,    NULL,            0, "<Separator>" },
#endif
  {"/File/Close",   "",      
    (GtkItemFactoryCallback) display_close_cb,
    0, "<Item>" },
};


static void 
display_datad_added_cb (GtkObject *obj /*gg->main_window*/,
			datad *d, ggobid *gg, GtkObject *win)
{
  windowDisplayd *display =  GTK_GGOBI_WINDOW_DISPLAY(win);

    /*-- this is all true even when the display is first opened --*/
  if (GTK_WIDGET_REALIZED (display->window)) {
      scatterplot_display_edge_menu_update (GTK_GGOBI_DISPLAY(display), gg->app.sp_accel_group, 
					    display_options_cb, gg);
  }
}

splotd *
gtk_scatter_plot_new(displayd *dpy, gint width, gint height, ggobid *gg)
{
   splotd *sp = gtk_type_new(GTK_TYPE_GGOBI_SCATTER_SPLOT);
   splot_init(sp, dpy, width, height, gg);
   return(sp);  
}

displayd *
createScatterplot(gboolean missing_p, splotd *sp, gint numVars, gint *vars, datad *d, ggobid *gg);

displayd *
scatterplot_new_with_vars(gboolean missing_p, gint numVars, gint *vars, datad *d, ggobid *gg)
{
  return(createScatterplot(missing_p, NULL, numVars, vars, d, gg));
#if 00
  splotd *sp;
  displayd *display = NULL;
  if(numVars < 2)
     return(NULL);
/*XX dislay needs to be non-null here. Need to get the order correct. Change scatterplot_new! */
  sp = gtk_scatter_plot_new(NULL, 400, 400, gg);
  sp->xyvars.x = vars[0];
  sp->xyvars.y = vars[1];
 
  display = scatterplot_new(missing_p, sp, d, gg); 
  sp->displayptr = display;
  splot_alloc(sp, display, gg);

  return(display);
#endif
}

displayd *
scatterplot_new (gboolean missing_p, splotd *sp, datad *d, ggobid *gg) 
{
  return(createScatterplot(missing_p, sp, 0, NULL, d, gg));
}

displayd *
createScatterplot(gboolean missing_p, splotd *sp, gint numVars, gint *vars, datad *d, ggobid *gg)
{
  GtkWidget *table, *vbox, *w;
  GtkItemFactory *factory;
  displayd *display;
  PipelineMode projection;

  if (d == NULL || d->ncols < 1)
    return (NULL);

  if (sp == NULL || sp->displayptr == NULL) {
     display = gtk_type_new(GTK_TYPE_GGOBI_SCATTERPLOT_DISPLAY);
     display_set_values(display, d, gg);
  } else {
    display = (displayd*) sp->displayptr;
    display->d = d;
  }

  /* Want to make certain this is true, and perhaps it may be different
     for other plot types and so not be set appropriately in DefaultOptions.
    display->options.axes_center_p = true;
   */

  projection = (d->ncols >= 2) ? XYPLOT : P1PLOT;
  scatterplot_cpanel_init (&display->cpanel, projection, gg);

  display_window_init (GTK_GGOBI_WINDOW_DISPLAY(display), 3, gg);  /*-- 3 = width = any small int --*/

  /*-- Add the main menu bar --*/
  vbox = GTK_WIDGET(display); /* gtk_vbox_new (false, 1); */
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (GTK_GGOBI_WINDOW_DISPLAY(display)->window), vbox);

  gg->app.sp_accel_group = gtk_accel_group_new ();
  factory = get_main_menu (menu_items,
			   sizeof (menu_items) / sizeof (menu_items[0]),
			   gg->app.sp_accel_group, 
			   GTK_GGOBI_WINDOW_DISPLAY(display)->window, 
			   &display->menubar,
			   (gpointer) display);

  /*-- add a tooltip to the file menu --*/
  w = gtk_item_factory_get_widget (factory, "<main>/File");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
			gtk_menu_get_attach_widget (GTK_MENU(w)),
			"File menu for this display", NULL);

  /*
   * After creating the menubar, and populating the file menu,
   * add the other menus manually
  */
  scatterplot_display_menus_make (display, gg->app.sp_accel_group,
				  (GtkSignalFunc) display_options_cb, gg);
  gtk_box_pack_start (GTK_BOX (vbox), display->menubar, false, true, 0);


  /*-- Initialize a single splot --*/
  if (sp == NULL) {
    sp = gtk_scatter_plot_new (display, WIDTH, HEIGHT, gg);
    if(numVars < 2 || vars == NULL) {
      sp->xyvars.x = 0;
      sp->xyvars.y = 1;
    } else {
      sp->xyvars.x = vars[0];
      sp->xyvars.y = vars[1];
    }
  }

  display->splots = NULL;
  display->splots = g_list_append (display->splots, (gpointer) sp);

  /*-- Initialize tours if possible --*/
 {
/*XX seems like only scatterplot gets in here. (i.e. not scatmat) */
    display_tour1d_init_null (display, gg);
    if (d->ncols >= MIN_NVARS_FOR_TOUR1D)
      display_tour1d_init (display, gg);

    display_tour2d_init_null (display, gg);
    if (d->ncols >= MIN_NVARS_FOR_TOUR2D)
      display_tour2d_init (display, gg);

    display_tourcorr_init_null (display, gg);
    if (d->ncols >= MIN_NVARS_FOR_COTOUR)
      display_tourcorr_init (display, gg);
  }

  table = gtk_table_new (3, 2, false);  /* rows, columns, homogeneous */
  gtk_box_pack_start (GTK_BOX (vbox), table, true, true, 0);
  gtk_table_attach (GTK_TABLE (table),
                    sp->da, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),
                    0, 0 );


  /*
   * The horizontal ruler goes on the bottom. As the mouse moves
   * across the drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
  */
#ifndef GTK_2_0
  display->hrule = gtk_ext_hruler_new ();
  gtk_signal_connect_object (GTK_OBJECT (sp->da), "motion_notify_event",
    (GtkSignalFunc) EVENT_METHOD (display->hrule, motion_notify_event),
    GTK_OBJECT (display->hrule));

  /*-- Enable panning and zooming using the rulers --*/
  gtk_signal_connect (GTK_OBJECT (display->hrule),
    "motion_notify_event", ruler_shift_cb, sp);
  gtk_signal_connect (GTK_OBJECT (display->hrule),
    "button_press_event", ruler_down_cb, sp);
#else
  display->hrule = gtk_hruler_new ();
   /* What about the events above. */
#endif

  gtk_table_attach (GTK_TABLE (table),
                    display->hrule, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL), 
                    (GtkAttachOptions) GTK_FILL,
                    0, 0);

  /*
   * The vertical ruler goes on the left. As the mouse moves across
   * the drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
  */
#ifndef GTK_2_0
  display->vrule = gtk_ext_vruler_new ();
  gtk_signal_connect_object (GTK_OBJECT (sp->da),
    "motion_notify_event",
    (GtkSignalFunc) EVENT_METHOD (display->vrule, motion_notify_event),
    GTK_OBJECT (display->vrule));

  /*-- Enable panning and zooming using the rulers --*/
  gtk_signal_connect (GTK_OBJECT (display->vrule),
    "motion_notify_event", ruler_shift_cb, sp);
  gtk_signal_connect (GTK_OBJECT (display->vrule),
    "button_press_event", ruler_down_cb, sp);
#else
  display->vrule = gtk_vruler_new ();
#endif

  gtk_table_attach (GTK_TABLE (table),
                    display->vrule, 0, 1, 0, 1,
                    (GtkAttachOptions) GTK_FILL, 
                    (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL),
                    0, 0 );

  gtk_widget_show_all (GTK_GGOBI_WINDOW_DISPLAY(display)->window);

   /*-- hide any extraneous rulers --*/
  scatterplot_show_rulers (display, projection);
  ruler_ranges_set (true, display, sp, gg);

  gtk_signal_connect (GTK_OBJECT (gg->main_window), "datad_added",
		        (GtkSignalFunc) display_datad_added_cb, display);

  return display;
}

/*--------------------------------------------------------------------
         Responding to the rulers
----------------------------------------------------------------------*/

static void ruler_down_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  displayd *display = (displayd *) sp->displayptr;
  if (w == display->hrule)
    display->drag_start.x = event->x;
  else
    display->drag_start.y = event->y;
}

static void ruler_shift_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp)
{
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  ggobid *gg = display->ggobi;
  gboolean button1_p, button2_p;
  gint direction = (w == display->hrule) ? HORIZONTAL : VERTICAL;
  gboolean redraw = false;

  /*-- find out if any buttons are pressed --*/
  mousepos_get_motion (w, event,  &button1_p, &button2_p, sp);

  if (button1_p) {

    if (direction == HORIZONTAL) {
      gfloat scale_x;
      gint dx = (gint) (event->x - display->drag_start.x);
      /*-- exactly as in pan_by_drag --*/
      scale_x = (cpanel->projection == TOUR2D) ? sp->tour_scale.x : sp->scale.x;
      scale_x /= 2;
      sp->iscale.x = (glong) ((gfloat) sp->max.x * scale_x);
      sp->pmid.x -= ((dx * PRECISION1) / sp->iscale.x);
      /* */
      display->drag_start.x = event->x;
      redraw = true;
    } else {
      gfloat scale_y;
      gint dy = -1 * (gint) (event->y - display->drag_start.y);

      /*-- exactly as in pan_by_drag --*/
      scale_y = (cpanel->projection == TOUR2D) ? sp->tour_scale.y : sp->scale.y;
      scale_y /= 2;
      sp->iscale.y = (glong) ((gfloat) sp->max.y * scale_y);
      sp->pmid.y -= ((dy * PRECISION1) / sp->iscale.y);
      /* */

      display->drag_start.y = event->y;
      redraw = true;
    }

  } else if (button2_p) {
    gint npix = 5;

    /*-- lifting code from zoom_by_drag as much as possible --*/
    if (direction == HORIZONTAL) {
      gfloat *scale_x;
      icoords mid;
      fcoords scalefac;

      mid.x = sp->max.x / 2;
      scalefac.x = 1.0;
      scale_x = (cpanel->projection == TOUR2D) ? &sp->tour_scale.x :
                                                 &sp->scale.x;
      if (ABS(event->x - mid.x) >= npix) {
        scalefac.x = (gfloat) (event->x - mid.x) /
                     (gfloat) (display->drag_start.x - mid.x);
        if (*scale_x * scalefac.x >= SCALE_MIN)
          *scale_x = *scale_x * scalefac.x;

        display->drag_start.x = event->x;
        redraw = true;
      }

    } else {
      gfloat *scale_y;
      icoords mid;
      fcoords scalefac;

      mid.y = sp->max.y / 2;
      scalefac.y = 1.0;
      scale_y = (cpanel->projection == TOUR2D) ? &sp->tour_scale.y :
                                                 &sp->scale.y;
      if (ABS(event->y - mid.y) >= npix) {
        scalefac.y = (gfloat) (event->y - mid.y) /
                     (gfloat) (display->drag_start.y - mid.y);
        if (*scale_y * scalefac.y >= SCALE_MIN)
          *scale_y = *scale_y * scalefac.y;

        display->drag_start.y = event->y;
        redraw = true;
      }
    }
  }

  /*
   * In motion_notify in scale_ui.c, ruler_ranges_set is also
   * executed, but I presumably don't have to do that here, as
   * long as these processes remain adequately in sync.
  */
  if (redraw) {
    splot_plane_to_screen (display, &display->cpanel, sp, gg);
    splot_redraw (sp, FULL, gg);
  }
}
