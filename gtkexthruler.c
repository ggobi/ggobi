/* -*- Mode: C; c-file-style: "bsd"; coding: latin-1-unix -*- */
/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

/* Modified by Gustavo Carneiro ee96090@fe.up.pt */

/*
 * Modified my Deborah Swayne, dfs@research.att.com, to
 * use myrint rather than rint, since it seemed that I couldn't
 * rely on rint being present in all environments.
*/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "gtkexthruler.h"

extern gdouble myrint (gdouble);

#define RULER_HEIGHT          14
#define MINIMUM_INCR          5
#define MAXIMUM_SUBDIVIDE     5
#define MAXIMUM_SCALES        10

#define ROUND(x) ((int) ((x) + 0.5))

static void gtk_ext_hruler_class_init    (GtkExtHRulerClass *klass);
static void gtk_ext_hruler_init          (GtkExtHRuler      *hruler);
static gint gtk_ext_hruler_motion_notify (GtkWidget         *widget,
					  GdkEventMotion    *event);
static void gtk_ext_hruler_draw_ticks    (GtkExtRuler       *ruler);
static void gtk_ext_hruler_draw_pos      (GtkExtRuler       *ruler);
static gint gtk_ext_hruler_drag_start    (GtkWidget         *widget,
					  GdkEventButton    *event);
static gint gtk_ext_hruler_drag_end      (GtkWidget         *widget,
					  GdkEventButton    *event);
void        gtk_ext_hruler_drag_motion   (GtkWidget         *widget,
					  gint               event_x);


guint
gtk_ext_hruler_get_type (void)
{
    static guint hruler_type = 0;

    if (!hruler_type)
    {
	static const GtkTypeInfo hruler_info =
	{
	    "GtkExtHRuler",
	    sizeof (GtkExtHRuler),
	    sizeof (GtkExtHRulerClass),
	    (GtkClassInitFunc) gtk_ext_hruler_class_init,
	    (GtkObjectInitFunc) gtk_ext_hruler_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	hruler_type = gtk_type_unique (gtk_ext_ruler_get_type (), &hruler_info);
    }

    return hruler_type;
}

static void
gtk_ext_hruler_class_init (GtkExtHRulerClass *klass)
{
    GtkWidgetClass *widget_class;
    GtkExtRulerClass *ruler_class;

    widget_class = (GtkWidgetClass*) klass;
    ruler_class = (GtkExtRulerClass*) klass;

    widget_class->motion_notify_event = gtk_ext_hruler_motion_notify;
    widget_class->button_press_event = gtk_ext_hruler_drag_start;
    widget_class->button_release_event = gtk_ext_hruler_drag_end;

    ruler_class->draw_ticks = gtk_ext_hruler_draw_ticks;
    ruler_class->draw_pos = gtk_ext_hruler_draw_pos;
}

static void
gtk_ext_hruler_init (GtkExtHRuler *hruler)
{
    GtkWidget *widget;

    widget = GTK_WIDGET (hruler);
    widget->requisition.width = widget->style->klass->xthickness * 2 + 1;
    widget->requisition.height = widget->style->klass->ythickness * 2 + RULER_HEIGHT;
    hruler->facing = GTK_EXT_HRULER_FACING_DOWN;
}


GtkWidget*
gtk_ext_hruler_new (void)
{
    return GTK_WIDGET (gtk_type_new (gtk_ext_hruler_get_type ()));
}

static void
gtk_ext_hruler_draw_ticks (GtkExtRuler *ruler)
{
    GtkExtRulerClass *klass;
    GtkWidget *widget;
    GdkGC *gc, *bg_gc;
    GdkFont *font;
    gint width, height, xthickness, ythickness;
    gchar unit_str[32];
    gint digit_height,length;
    gdouble rx0,rxl,rx,rxf,rdiv;
    gfloat x0,x,x1,xf,subx,div,subdiv;
    gint wx,t;
    gboolean facing_down;
    GtkExtHRuler *hruler;
    g_return_if_fail (ruler != NULL);
    hruler = GTK_EXT_HRULER (ruler);
    
    /*if (!GTK_WIDGET_DRAWABLE (ruler)) 
	return;*/

    klass = GTK_EXT_RULER_CLASS(GTK_OBJECT(ruler)->klass);
  
    gtk_ext_ruler_calc_scale(ruler,'h');

    widget = GTK_WIDGET (ruler);
    
    gc = widget->style->fg_gc[widget->state];
    bg_gc = widget->style->bg_gc[widget->state];
    font = widget->style->font;
    
    facing_down = (hruler->facing!=GTK_EXT_HRULER_FACING_UP);

    xthickness = widget->style->klass->xthickness;
    ythickness = widget->style->klass->ythickness;
    digit_height = font->ascent; /* assume descent == 0 ? */

    width = widget->allocation.width;
    height = widget->allocation.height - ythickness * 2;
   
    gtk_paint_box (widget->style, ruler->backing_store,
		   widget->state, GTK_SHADOW_OUT, 
		   NULL, widget, "hruler",
		   0, 0, 
		   widget->allocation.width, widget->allocation.height);

    if(facing_down)
	gdk_draw_line (ruler->backing_store, gc,
		       xthickness,
		       height + ythickness,
		       widget->allocation.width - xthickness,
		       height + ythickness);
    else
	gdk_draw_line (ruler->backing_store, gc,
		       xthickness,
		       0,
		       widget->allocation.width - xthickness,
		       0);

    x0 = ruler->tick_start;
    div = ruler->tick_div;
    subdiv = ruler->tick_subdiv;
    xf = ruler->tick_limit;

    rx0 = ruler->rtick_start;
    rdiv = ruler->rtick_div;
    rxf = ruler->tick_limit;
  
    switch(ruler->mode) 
    {
    case GTK_EXT_RULER_LINEAR:
	for(x=x0,rx=rx0,rxl=rx0; x<xf; x+=div,rx+=rdiv) {
	    wx = myrint(x);
	    length = 10;
	    if(wx>=0) {
		if(facing_down) {
		    gdk_draw_line (ruler->backing_store, gc,
				   wx, height + ythickness, 
				   wx, height - length + ythickness);
		} else {
		    gdk_draw_line (ruler->backing_store, gc,
				   wx, 0, 
				   wx, length - 1);
		}
	    }
            /*
             * is this number very small compared to the previous
             * number? If so, it's probably just 0.0
            */
	    if(fabs(rx/rxl)<=1e-10)
		sprintf (unit_str, "%G", 0.0);
	    else
		sprintf (unit_str, "%G", rx);
	    rxl = rx;
	    if(facing_down)
		gdk_draw_string (ruler->backing_store, font, gc,
				 wx + 2, ythickness + font->ascent - 1,
				 unit_str);
	    else
		gdk_draw_string (ruler->backing_store, font, gc,
				 wx + 2, height - 1,
				 unit_str);
	    for(subx=x; subx<x+div; subx+=subdiv) {
		length=5;
		wx = myrint(subx);
		if(wx>=0) {
		    if(facing_down) {
			gdk_draw_line (ruler->backing_store, gc,
				       wx, height + ythickness, 
				       wx, height - length + ythickness);
		    } else {
			gdk_draw_line (ruler->backing_store, gc,
				       wx, 0, 
				       wx, length - 1);
		    }
		}
		if(subdiv>=6) {
		    wx += subdiv/2;
		    length = 3;
		    if(wx>=0) {
			if(facing_down) {
			    gdk_draw_line (ruler->backing_store, gc,
					   wx, height + ythickness, 
					   wx, height - length + ythickness);
			} else {
			    gdk_draw_line (ruler->backing_store, gc,
					   wx, 0, 
					   wx, length - 1);
			}
		    }
		}
	    }
	}		
	break;
    case GTK_EXT_RULER_LOG:
	if(div<10) break; 

	for(x=x0,rx=rx0; x<xf; x+=div,rx++) {
	    wx = myrint(x);
	    length = 10;
	    if(wx>=0) {
		if(facing_down) {
		    gdk_draw_line (ruler->backing_store, gc,
				   wx, height + ythickness, 
				   wx, height - length + ythickness);
		} else {
		    gdk_draw_line (ruler->backing_store, gc,
				   wx, 0, 
				   wx, length - 1);
		}
	    }
	    if(rx<=3 && rx >=-2)
		sprintf (unit_str, "%G", pow(10,rx));
	    else
		sprintf (unit_str, "1E%i", (int)rx);
	    /*sprintf (unit_str, "%G", pow(10,rx));*/
	    if(facing_down)
		gdk_draw_string (ruler->backing_store, font, gc,
				 wx + 2, ythickness + font->ascent - 1,
				 unit_str);	  
	    else
		gdk_draw_string (ruler->backing_store, font, gc,
				 wx + 2, height - 1,
				 unit_str);	/* ? */

	    for(t=2 ; t<=9 ; t++) {
		x1 = x + div*klass->log29[t-2];
		if(x1>xf) break;
		if(x1>=0) {
		    length = 3;
		    wx = myrint(x1);
		    if(facing_down)
			gdk_draw_line (ruler->backing_store, gc,
				       wx, height + ythickness, 
				       wx, height - length + ythickness);
		    else
			gdk_draw_line (ruler->backing_store, gc,
				       wx, 0, 
				       wx, length - 1);
		}
	    }
	}
	break;
    }
}

static void
gtk_ext_hruler_draw_pos (GtkExtRuler *ruler)
{
    GtkWidget *widget;
    GdkGC *gc;
    int i;
    gint x, y;
    gint width, height;
    gint bs_width, bs_height;
    gint xthickness;
    gint ythickness;
    gfloat increment;
    GtkExtHRuler *hruler;
    gboolean facing_down;

    g_return_if_fail (ruler != NULL);
    g_return_if_fail (GTK_IS_EXT_HRULER (ruler));

    hruler = GTK_EXT_HRULER (ruler);
    facing_down = (hruler->facing!=GTK_EXT_HRULER_FACING_UP);
    
    if (GTK_WIDGET_DRAWABLE (ruler))
    {
	widget = GTK_WIDGET (ruler);

	gc = widget->style->fg_gc[widget->state];
	xthickness = widget->style->klass->xthickness;
	ythickness = widget->style->klass->ythickness;
	width = widget->allocation.width;
	height = widget->allocation.height - ythickness * 2;

	bs_width = height / 2;
	bs_width |= 1;  /* make sure it's odd */
	bs_height = bs_width / 2 + 1;

	if ((bs_width > 0) && (bs_height > 0))
	{
	    /*  If a backing store exists, restore the ruler  */
	    if (ruler->backing_store && ruler->non_gr_exp_gc)
		gdk_draw_pixmap (ruler->widget.window,
				 ruler->non_gr_exp_gc,
				 ruler->backing_store,
				 ruler->xsrc, ruler->ysrc,
				 ruler->xsrc, ruler->ysrc,
				 bs_width, bs_height);

	    increment = (gfloat) width / (ruler->upper - ruler->lower);

	    x = ROUND ((ruler->position - ruler->lower) * increment) + (xthickness - bs_width) / 2 - 1;
	    if (facing_down)
		y = (height + bs_height) / 2 + ythickness;
	    else 
		y = bs_height - 1 + ythickness;

	    for (i = 0; i < bs_height; i++)
		gdk_draw_line (widget->window, gc,
			       x + i, y + (facing_down?i:-i),
			       x + bs_width - 1 - i, y + (facing_down?i:-i));
	    ruler->xsrc = x;
	    if (facing_down)
		ruler->ysrc = y;
	    else 
		ruler->ysrc = ythickness;
	}
    }
}
/* --------------------- */

/* Horizontal ruler drag event handlers */
static gint gtk_ext_hruler_drag_start(GtkWidget *widget, GdkEventButton *event)
{
    GtkExtRuler *ruler;
    gint width;
    gboolean forwarded;

    g_return_val_if_fail(GTK_IS_EXT_HRULER(widget),0);
    ruler = GTK_EXT_RULER(widget);

    forwarded = ruler->synthetic_event;
    width = widget->allocation.width;  
    if(ruler->dragging)
	return FALSE;
    ruler->dragging = TRUE;
    ruler->drag_start = event->x;
    ruler->drag_button = event->button;
    ruler->lower1 = ruler->lower;
    ruler->upper1 = ruler->upper;
    if(!forwarded) {
	gdk_pointer_grab(widget->window,FALSE,
			 GDK_POINTER_MOTION_MASK|GDK_BUTTON_RELEASE_MASK,
			 NULL,NULL,event->time);
	if(ruler->sync_flags&GTK_EXT_RULER_SYNC_ABSOLUTE && ruler->sync_ruler)
	    gtk_ext_ruler_set_range(GTK_EXT_RULER (ruler->sync_ruler),ruler->lower,ruler->upper);
	if(ruler->sync_flags&GTK_EXT_RULER_SYNC_DRAG && ruler->sync_ruler) {
	    ruler->sync_ruler->synthetic_event = TRUE;
	    gtk_ext_hruler_drag_start(GTK_WIDGET(ruler->sync_ruler),event);
	}
    } else { 
	
	/*g_message(__FUNCTION__ ": received a forwarded event. :-)"); */
    }
    ruler->synthetic_event = FALSE;
    return FALSE;
}


static gint gtk_ext_hruler_drag_end(GtkWidget *widget, GdkEventButton *event)
{
    GtkExtRuler *ruler;
    gboolean forwarded;
    g_return_val_if_fail(GTK_IS_EXT_HRULER(widget),0);
    ruler = GTK_EXT_RULER(widget);

    forwarded = ruler->synthetic_event;
    ruler->dragging = FALSE;
    if(!forwarded) {
	gdk_pointer_ungrab(event->time);
	if(ruler->sync_flags&GTK_EXT_RULER_SYNC_DRAG && ruler->sync_ruler) {
	    ruler->sync_ruler->synthetic_event = TRUE;
	    gtk_ext_hruler_drag_end(GTK_WIDGET(ruler->sync_ruler), event);
	}
	if(ruler->sync_flags&GTK_EXT_RULER_SYNC_ABSOLUTE && ruler->sync_ruler)
	    gtk_ext_ruler_set_range(GTK_EXT_RULER (ruler->sync_ruler), ruler->lower, ruler->upper);
    } else {
	ruler->synthetic_event = FALSE;
	/* g_message(__FUNCTION__ ": received a forwarded event. :-)"); */
    }
    gtk_signal_emit_by_name(GTK_OBJECT(widget),"range_changed",NULL);
    return FALSE;
}

static gint
gtk_ext_hruler_motion_notify (GtkWidget      *widget,
			    GdkEventMotion *event)
{
    GtkExtRuler *ruler;
    gint x;
    gboolean forwarded;

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_EXT_HRULER (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    ruler = GTK_EXT_RULER (widget);
    forwarded = ruler->synthetic_event;
    if (event->is_hint) 
	gdk_window_get_pointer (widget->window, &x, NULL, NULL);
    else
	x = event->x;

    ruler->position = ruler->lower + ((ruler->upper - ruler->lower) * x) / 
	widget->allocation.width;


    if (ruler->backing_store != NULL)
	gtk_ext_ruler_draw_pos (ruler);
    gtk_ext_hruler_drag_motion(widget,x);

    if(!forwarded) {
	if(ruler->sync_ruler) {
	    ruler->sync_ruler->synthetic_event = TRUE;
	    gtk_ext_hruler_motion_notify((GtkWidget*)ruler->sync_ruler,event);	
	}
    } else { 
	ruler->synthetic_event = FALSE;
    }
    return FALSE;
}


/*   */
void gtk_ext_hruler_drag_motion(GtkWidget *widget,gint event_x)
{
    gdouble dx,view_w,xinc;
    GtkExtRuler *ruler;
    gint width;
    g_return_if_fail(GTK_IS_EXT_HRULER(widget));
    ruler = GTK_EXT_RULER(widget);
    width = widget->allocation.width;  
  
    if(!ruler->dragging)
	return;
    dx = event_x - ruler->drag_start;
    view_w = ruler->upper1 - ruler->lower1;
    switch(ruler->drag_button)
    {
    case 1: /* Button 1 -> Shift mode */
	ruler->drag_start = event_x;
	xinc = dx/width*view_w;
	gtk_ext_ruler_set_range(ruler, ruler->lower - xinc, ruler->upper - xinc);
	break;
    case 3: /* Button 3 -> Scale mode */
    {				
	gdouble xi,xf,xc,scale;
	xc = ruler->lower1+ruler->drag_start/width*view_w;
	scale = exp( -(event_x - ruler->drag_start)/100 );
	xi = xc - (xc - ruler->lower1)*scale;
	xf = xc + (ruler->upper1 - xc)*scale;
	gtk_ext_ruler_set_range(ruler, xi, xf);
    }
    break;
    }
}


void gtk_ext_hruler_set_facing(GtkExtHRuler *hruler, GtkExtHRulerFacing facing)
{
    GtkExtRuler *ruler;
    g_return_if_fail(GTK_IS_EXT_HRULER(hruler));
    ruler = GTK_EXT_RULER(hruler);
    
    hruler->facing = facing;

}
