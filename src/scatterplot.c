/* scatterplot.c */
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
#include <gtk/gtk.h>
#include "vars.h"

#include "scatterplotClass.h"

#include "externs.h"

#define WIDTH   370
#define HEIGHT  370


/*-- as long as these are static, they can probably stay here --*/
static gboolean ruler_shift_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp);
static void ruler_down_cb (GtkWidget *w, GdkEventButton *event, splotd *sp);
static gboolean ruler_motion_cb (GtkWidget *ruler, GdkEventMotion *event, GtkWidget *da);

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
  //splotd *sp = display->splots->data;
  //gint width = sp->da->allocation.width;
  //gint height = sp->da->allocation.height;

  switch (projection) {
    case P1PLOT:
      if (display->p1d_orientation == VERTICAL) {
        scatterplot_show_vrule (display, display->options.axes_show_p);
        scatterplot_show_hrule (display, false);
      } else {
        scatterplot_show_vrule (display, false);
        scatterplot_show_hrule (display, display->options.axes_show_p);
      }
    break;

    case XYPLOT:
      scatterplot_show_vrule (display, display->options.axes_show_p);
      scatterplot_show_hrule (display, display->options.axes_show_p);
    break;

    case TOUR1D:
    case TOUR2D3:
    case TOUR2D:
    case COTOUR:
    default:  /* in any other projection, no rulers */
      scatterplot_show_vrule (display, false);
      scatterplot_show_hrule (display, false);
    break;
  }

  //gtk_widget_set_usize(GTK_WIDGET (sp->da), width, height);
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
  if(sp && GGOBI_IS_EXTENDED_SPLOT(sp)) {
     GGobiExtendedSPlotClass *klass;
     klass = GGOBI_EXTENDED_SPLOT_GET_CLASS(sp);
     if(klass->screen_to_tform)
       klass->screen_to_tform(cpanel, sp, &scr, &tfmin, gg);
  }

  scr.x = sp->max.x;
  scr.y = sp->max.y;
  if(sp && GGOBI_IS_EXTENDED_SPLOT(sp)) {
     GGobiExtendedSPlotClass *klass;
     klass = GGOBI_EXTENDED_SPLOT_GET_CLASS(sp);
     if(klass->screen_to_tform)
       klass->screen_to_tform(cpanel, sp, &scr, &tfmax, gg);
  }

  /*
   * Reset only if necessary:  if the ruler is visible and the
   * ranges have changed.  Force when initializing display.
  */
  if (force || GTK_WIDGET_VISIBLE (display->hrule)) {
    if (((gfloat) GTK_RULER (display->hrule)->lower != tfmin.x) ||
        ((gfloat) GTK_RULER (display->hrule)->upper != tfmax.x))
    {
      /* What should the final 2 arguments be. */
      gtk_ruler_set_range (GTK_RULER (display->hrule),
                               (gdouble) tfmin.x, (gdouble) tfmax.x,
                               (gdouble) (tfmax.x - tfmin.x)/2 + tfmin.x,
                               tfmax.x);
    }
  }

  if (force || GTK_WIDGET_VISIBLE (display->vrule)) {
    if (((gfloat) GTK_RULER (display->vrule)->upper != tfmin.y) ||
        ((gfloat) GTK_RULER (display->vrule)->lower != tfmax.y))
    {
      gtk_ruler_set_range (GTK_RULER (display->vrule),
                               (gdouble) tfmin.y, (gdouble) tfmax.y,
                               (gdouble) (tfmax.y - tfmin.y)/2 + tfmin.y,
                               tfmax.y);
    }
  }
}

/*----------------------------------------------------------------------*/
/*                          Options section                             */
/*----------------------------------------------------------------------*/

static const gchar *scatterplot_ui =
"<ui>"
"	<menubar>"
"		<menu action='Options'>"
"			<menuitem action='ShowPoints'/>"
"			<menuitem action='ShowAxes'/>"
"		</menu>"
"	</menubar>"
"</ui>";

static void 
display_datad_added_cb (ggobid *gg, datad *d, void *win)
{
  windowDisplayd *display =  GGOBI_WINDOW_DISPLAY(GTK_OBJECT(win));

  /*-- this is all true even when the display is first opened --*/
  if (display->window && GTK_WIDGET_REALIZED (display->window)) {
      scatterplot_display_edge_menu_update (GGOBI_DISPLAY(display),
        gg->app.sp_accel_group, gg);
  }
}

CHECK_EVENT_SIGNATURE(display_datad_added_cb, datad_added_f)


splotd *
ggobi_scatter_plot_new(displayd *dpy, ggobid *gg)
{
   splotd *sp = g_object_new(GGOBI_TYPE_SCATTER_SPLOT, NULL);
   splot_init(sp, dpy, gg);
   return(sp);  
}

displayd *
scatterplot_new_with_vars(gboolean missing_p, gint numVars, gint *vars, datad *d, ggobid *gg)
{
  return(createScatterplot(NULL, missing_p, NULL, numVars, vars, d, gg));
}

displayd *
scatterplot_new (gboolean missing_p, splotd *sp, datad *d, ggobid *gg) 
{
  return(createScatterplot(NULL, missing_p, sp, 0, NULL, d, gg));
}



void
GGOBI(edge_menus_update)(ggobid *gg)
{
  GList *dlist;
  displayd *display;
  for (dlist = gg->displays; dlist != NULL; dlist = dlist->next) {
    display = (displayd *) dlist->data;
    if (GTK_WIDGET_REALIZED (GGOBI_WINDOW_DISPLAY(display)->window) &&
        GGOBI_IS_SCATTERPLOT_DISPLAY(display))
    {
      scatterplot_display_edge_menu_update (GGOBI_DISPLAY(display),
        gg->app.sp_accel_group, gg);
    }
  }
}



displayd *
createScatterplot(displayd *display, gboolean missing_p, splotd *sp, gint numVars, gint *vars, datad *d, ggobid *gg)
{
  GtkWidget *table, *vbox;
  ProjectionMode projection;

  if (d == NULL || d->ncols < 1)
    return (NULL);

  if (!display) {
   if(sp == NULL || sp->displayptr == NULL) {
    display = g_object_new(GGOBI_TYPE_SCATTERPLOT_DISPLAY, NULL);
    display_set_values(display, d, gg);
   } else {
    display = (displayd*) sp->displayptr;
    display->d = d;
   }
  }

  /* Want to make certain this is true, and perhaps it may be different
     for other plot types and so not be set appropriately in DefaultOptions.
     display->options.axes_center_p = true;
   */

  projection = (d->ncols >= 2) ? XYPLOT : P1PLOT;
  scatterplot_cpanel_init (&display->cpanel, projection, DEFAULT_IMODE, gg);

  vbox = GTK_WIDGET(display); /* gtk_vbox_new (false, 1); */

  if(GGOBI_IS_WINDOW_DISPLAY(display) && GGOBI_WINDOW_DISPLAY(display)->useWindow) {
    display_window_init (GGOBI_WINDOW_DISPLAY(display), WIDTH, HEIGHT, 3, gg);

    /*-- Add the main menu bar --*/
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 1);
    gtk_container_add (GTK_CONTAINER (GGOBI_WINDOW_DISPLAY(display)->window), vbox);
	
	display->menu_manager = display_menu_manager_create(display);
	display->menubar = create_menu_bar(display->menu_manager, scatterplot_ui, 
		GGOBI_WINDOW_DISPLAY(display)->window);
/*
    gg->app.sp_accel_group = gtk_accel_group_new ();
    factory = get_main_menu (menu_items, sizeof (menu_items) / sizeof (menu_items[0]),
			     gg->app.sp_accel_group, 
			     GGOBI_WINDOW_DISPLAY(display)->window, 
			     &display->menubar,
			     (gpointer) display);
*/

  /*-- add a tooltip to the file menu --*/
/*    w = gtk_item_factory_get_widget (factory, "<main>/File");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
			  gtk_menu_get_attach_widget (GTK_MENU(w)),
			  "File menu for this display", NULL);
*/
  /*
   * After creating the menubar, and populating the file menu,
   * add the other menus manually
  */
/*    scatterplot_display_menus_make (display, gg->app.sp_accel_group,
				    G_CALLBACK(display_options_cb), gg);*/
	scatterplot_display_edge_menu_update(display, gg->app.sp_accel_group, gg);
    gtk_box_pack_start (GTK_BOX (vbox), display->menubar, false, true, 0);
  }

  /*-- Initialize a single splot --*/
  if (sp == NULL) {
    sp = ggobi_scatter_plot_new (display, gg);
    if(numVars < 2 || vars == NULL) {

      /* Initialize display with the plotted variables in the current
         display, if appropriate */
      if (gg->current_display != NULL && gg->current_display != display && 
          gg->current_display->d == d && 
          GGOBI_IS_EXTENDED_DISPLAY(gg->current_display))
      {
        gint nplotted_vars;
        gint *plotted_vars = (gint *) g_malloc(d->ncols * sizeof(gint));
        displayd *dsp = gg->current_display;

        nplotted_vars = GGOBI_EXTENDED_DISPLAY_GET_CLASS(dsp)->plotted_vars_get(dsp, plotted_vars, d, gg);

        if (nplotted_vars) {
          if (projection == XYPLOT)
    	    sp->xyvars.x = plotted_vars[0];
          else
            sp->p1dvar = plotted_vars[0];
        }
        if (nplotted_vars > 1 && projection == XYPLOT)
	  sp->xyvars.y = plotted_vars[1];

        g_free (plotted_vars);
      }
    } else {
      if (projection == XYPLOT) {
        sp->xyvars.x = vars[0];
        sp->xyvars.y = vars[1];
      } else {
        sp->p1dvar = vars[0];
      }
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

    display_tour2d3_init_null (display, gg);
    if (d->ncols >= MIN_NVARS_FOR_TOUR2D3)
      display_tour2d3_init (display, gg);

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
/*
  */

  display->hrule = gtk_hruler_new ();

  g_signal_connect (G_OBJECT (display->hrule),
    "motion_notify_event", G_CALLBACK(ruler_shift_cb), sp);
  g_signal_connect (G_OBJECT (display->hrule),
    "button_press_event", G_CALLBACK(ruler_down_cb), sp);
  g_signal_connect_swapped (G_OBJECT (sp->da),
    "motion_notify_event", G_CALLBACK(ruler_motion_cb), display->hrule);

  
   /* What about the events above. */

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

  display->vrule = gtk_vruler_new ();

  g_signal_connect (G_OBJECT (display->vrule),
    "motion_notify_event", G_CALLBACK(ruler_shift_cb), sp);
  g_signal_connect (G_OBJECT (display->vrule),
    "button_press_event", G_CALLBACK(ruler_down_cb), sp);
  g_signal_connect_swapped (G_OBJECT (sp->da),
    "motion_notify_event", G_CALLBACK(ruler_motion_cb), display->vrule);
  
  gtk_table_attach (GTK_TABLE (table),
                    display->vrule, 0, 1, 0, 1,
                    (GtkAttachOptions) GTK_FILL, 
                    (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL),
                    0, 0 );

  if(GGOBI_IS_WINDOW_DISPLAY(display) && GGOBI_WINDOW_DISPLAY(display)->useWindow) 
     gtk_widget_show_all (GGOBI_WINDOW_DISPLAY(display)->window);

   /*-- hide any extraneous rulers --*/
  scatterplot_show_rulers (display, projection);
  ruler_ranges_set (true, display, sp, gg);

  g_signal_connect_object(G_OBJECT(gg), "datad_added",
        G_CALLBACK(display_datad_added_cb), G_OBJECT(display), 0);

  return display;
}

/*--------------------------------------------------------------------
         Responding to the rulers
----------------------------------------------------------------------*/

static gboolean ruler_motion_cb (GtkWidget *ruler, GdkEventMotion *event, GtkWidget *da)
{
	gint pos, max;
	gdouble position;
	gdouble lower, upper;
	
	gint x, y;
	GdkModifierType state;
	
	gdk_window_get_pointer(da->window, &x, &y, &state);
	
	if (GTK_IS_HRULER(ruler)) {
		pos = x;
		max = da->allocation.width;
	} else {
		pos = y;
		max = da->allocation.height;
	}
	
	gtk_ruler_get_range(GTK_RULER(ruler), &lower, &upper, NULL, NULL);
	position = lower + pos * (upper - lower) / max;
	
	g_object_set(G_OBJECT(ruler), "position", position, NULL);
	
	return(false);
}

static void ruler_down_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  displayd *display = (displayd *) sp->displayptr;
  if (w == display->hrule)
    display->drag_start.x = event->x;
  else
    display->drag_start.y = event->y;
}

static gboolean ruler_shift_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp)
{
  displayd *display = (displayd *) sp->displayptr;
  /*cpaneld *cpanel = &display->cpanel;*/
  ggobid *gg = display->ggobi;
  gboolean button1_p, button2_p;
  gint direction = (w == display->hrule) ? HORIZONTAL : VERTICAL;
  gboolean redraw = false;
  greal precis = (greal) PRECISION1;

  /*-- find out if any buttons are pressed --*/
  mousepos_get_motion (w, event,  &button1_p, &button2_p, sp);

  if (button1_p) {

    if (direction == HORIZONTAL) {
      greal scale_x;
      greal dx = (greal) (event->x - display->drag_start.x);
      /*-- exactly as in pan_by_drag --*/
      /*      scale_x = (cpanel->projection == TOUR2D) ? sp->tour_scale.x : sp->scale.x;*/
      scale_x = sp->scale.x;
      scale_x /= 2;
      sp->iscale.x = (greal) sp->max.x * scale_x;
      sp->pmid.x -= (dx * precis / sp->iscale.x);
      /* */
      display->drag_start.x = event->x;
      redraw = true;
    } else {
      greal scale_y;
      greal dy = -1 * (greal) (event->y - display->drag_start.y);

      /*-- exactly as in pan_by_drag --*/
      /*      scale_y = (cpanel->projection == TOUR2D) ? sp->tour_scale.y : sp->scale.y;*/
      scale_y = sp->scale.y;
      scale_y /= 2;
      sp->iscale.y = (greal) sp->max.y * scale_y;
      sp->pmid.y -= (dy * precis / sp->iscale.y);
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
      /*      scale_x = (cpanel->projection == TOUR2D) ? &sp->tour_scale.x :
              &sp->scale.x;*/
      scale_x = &sp->scale.x;
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
      /*      scale_y = (cpanel->projection == TOUR2D) ? &sp->tour_scale.y :
              &sp->scale.y;*/
      scale_y = &sp->scale.y;
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
  
  return(false);
}
