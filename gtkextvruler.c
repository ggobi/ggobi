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


#include <math.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "gtkextvruler.h"

extern gdouble myrint (gdouble);

#define RULER_WIDTH           14
#define MINIMUM_INCR          5
#define MAXIMUM_SUBDIVIDE     5
#define MAXIMUM_SCALES        10

#define ROUND(x) ((int) ((x) + 0.5))


static void gtk_ext_vruler_class_init    (GtkExtVRulerClass *klass);
static void gtk_ext_vruler_init          (GtkExtVRuler      *vruler);
static gint gtk_ext_vruler_motion_notify (GtkWidget         *widget,
					  GdkEventMotion    *event);
static void gtk_ext_vruler_draw_ticks    (GtkExtRuler       *ruler);
static void gtk_ext_vruler_draw_pos      (GtkExtRuler       *ruler);
static gint gtk_ext_vruler_drag_start    (GtkWidget         *widget,
					  GdkEventButton    *event);
static gint gtk_ext_vruler_drag_end      (GtkWidget         *widget,
					  GdkEventButton    *event);
void        gtk_ext_vruler_drag_motion   (GtkWidget         *widget,
					  GdkEventMotion    *event);



guint
gtk_ext_vruler_get_type (void)
{
    static guint vruler_type = 0;
    
    if (!vruler_type)
    {
	static const GtkTypeInfo vruler_info =
	{
	    "GtkExtVRuler",
	    sizeof (GtkExtVRuler),
	    sizeof (GtkExtVRulerClass),
	    (GtkClassInitFunc) gtk_ext_vruler_class_init,
	    (GtkObjectInitFunc) gtk_ext_vruler_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};
	
	vruler_type = gtk_type_unique (gtk_ext_ruler_get_type (), &vruler_info);
    }
    
    return vruler_type;
}

static void
gtk_ext_vruler_class_init (GtkExtVRulerClass *klass)
{
    GtkWidgetClass *widget_class;
    GtkExtRulerClass *ruler_class;

    widget_class = (GtkWidgetClass*) klass;
    ruler_class = (GtkExtRulerClass*) klass;

    widget_class->motion_notify_event = gtk_ext_vruler_motion_notify;

    widget_class->button_press_event = gtk_ext_vruler_drag_start;
    widget_class->button_release_event = gtk_ext_vruler_drag_end;

    ruler_class->draw_ticks = gtk_ext_vruler_draw_ticks;
    ruler_class->draw_pos = gtk_ext_vruler_draw_pos;
}

static void
gtk_ext_vruler_init (GtkExtVRuler *vruler)
{
    GtkWidget *widget;

    widget = GTK_WIDGET (vruler);
    widget->requisition.width = widget->style->klass->xthickness * 2 + RULER_WIDTH;
    widget->requisition.height = widget->style->klass->ythickness * 2 + 1;
}

GtkWidget*
gtk_ext_vruler_new (void)
{
    return GTK_WIDGET (gtk_type_new (gtk_ext_vruler_get_type ()));
}


static gint
gtk_ext_vruler_motion_notify (GtkWidget      *widget,
			    GdkEventMotion *event)
{
    GtkExtRuler *ruler;
    gint y;

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_EXT_VRULER (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    ruler = GTK_EXT_RULER (widget);

    if (event->is_hint)
	gdk_window_get_pointer (widget->window, NULL, &y, NULL);
    else
	y = event->y;

    ruler->position = ruler->lower + ((ruler->upper - ruler->lower) * y) / widget->allocation.height;

    /*  Make sure the ruler has been allocated already  */
    if (ruler->backing_store != NULL)
	gtk_ext_ruler_draw_pos (ruler);

    if(ruler->dragging)
	gtk_ext_vruler_drag_motion(widget,event);

    return FALSE;
}

static void
gtk_ext_vruler_draw_ticks (GtkExtRuler *ruler)
{
    GtkWidget *widget;
    GdkGC *gc, *bg_gc;
    GdkFont *font;
    gint j;
    gint width, height;
    gint xthickness;
    gint ythickness;
    gint length;
    gchar unit_str[32];
    gchar digit_str[2] = { '\0', '\0' };
    gint digit_height;
    

    GtkExtRulerClass *klass;
    gdouble ry0,ry,ryl,ryf,rdiv;
    gfloat y0,y,y1,yf,suby,div,subdiv;
    gint wy,t;


    g_return_if_fail (ruler != NULL);
    g_return_if_fail (GTK_IS_EXT_VRULER (ruler));

   /* if (!GTK_WIDGET_DRAWABLE (ruler)) 
	return;*/

    widget = GTK_WIDGET (ruler);
    klass = GTK_EXT_RULER_CLASS(GTK_OBJECT(ruler)->klass);

    gc = widget->style->fg_gc[widget->state];
    bg_gc = widget->style->bg_gc[widget->state];
    font = widget->style->font;
    xthickness = widget->style->klass->xthickness;
    ythickness = widget->style->klass->ythickness;
    digit_height = font->ascent; 

    width = widget->allocation.height;
    height = widget->allocation.width - ythickness * 2;

    gtk_paint_box (widget->style, ruler->backing_store,
		   widget->state, GTK_SHADOW_OUT, 
		   NULL, widget, "vruler",
		   0, 0, 
		   widget->allocation.width, widget->allocation.height);

    gdk_draw_line (ruler->backing_store, gc,
		   height + xthickness,
		   ythickness,
		   height + xthickness,
		   widget->allocation.height - ythickness);
   
    gtk_ext_ruler_calc_scale(ruler,'v');
     
    y0 = ruler->tick_start;
    div = ruler->tick_div;
    subdiv = ruler->tick_subdiv;
    yf = ruler->tick_limit+div;

    ry0 = ruler->rtick_start;
    rdiv = ruler->rtick_div;
    ryf = ruler->rtick_limit;

    /* drawing starts here */
    switch(ruler->mode) 
    {
    case GTK_EXT_RULER_LINEAR:
	for(y=y0,ry=ry0,ryl=ry0; y<yf; y+=div,ry+=rdiv) {
	    wy = width - myrint(y);
	    length = 10;
	    gdk_draw_line (ruler->backing_store, gc,
			   height + xthickness - length, wy,
			   height + xthickness, wy);
	    /* draw label */
            /*
             * is this number very small compared to the previous
             * number? If so, it's probably just 0.0
            */
	    if(fabs(ry/ryl)<=1e-10)
		sprintf (unit_str, "%G", 0.0);
	    else
		sprintf (unit_str, "%G", ry);
	    ryl = ry;
	    for (j = 0; j < (int) strlen (unit_str); j++)
	    {
		digit_str[0] = unit_str[j];
		gdk_draw_string (ruler->backing_store, font, gc,
				 xthickness + 1,
				 wy + digit_height * (j + 1) + 1,
				 digit_str);
	    }
	  
	    for(suby=y; suby<y+div; suby+=subdiv) 
	    {
		length=5;
		wy = width - myrint(suby);
		gdk_draw_line (ruler->backing_store, gc,
			       height + xthickness - length, wy,
			       height + xthickness, wy);
		if(subdiv>=6) 
		{
		    wy -= subdiv/2;
		    length = 3;
		    gdk_draw_line (ruler->backing_store, gc,
				   height + xthickness - length, wy,
				   height + xthickness, wy);
		}
	    }
	}		
	break;
    case GTK_EXT_RULER_LOG:
	if(div<10) break; 
	
	for(y=y0,ry=ry0; y<yf; y+=div,ry++) {
	    wy = width - myrint(y);
	    length = 10;
	    gdk_draw_line (ruler->backing_store, gc,
			   height + xthickness - length, wy,
			   height + xthickness, wy);
	    /* draw label */
	    if(ry<=3 && ry >=-2)
		sprintf (unit_str, "%G", pow(10,ry));
	    else
		sprintf (unit_str, "1E%i", (int)ry);
	    /* sprintf (unit_str, "%G", pow(10,ry)); */
	    for (j = 0; j < (int) strlen (unit_str); j++)
	    {
		digit_str[0] = unit_str[j];
		gdk_draw_string (ruler->backing_store, font, gc,
				 xthickness + 1,
				 wy + digit_height * (j + 1) + 1,
				 digit_str);
	    }

	    for(t=2 ; t<=9 ; t++) {
		y1 = y + div*klass->log29[t-2];
		if(y1>yf) break;
		if(y1>=0) {
		    length = 3;
		    wy = width - myrint(y1);
		    gdk_draw_line (ruler->backing_store, gc,
				   height + xthickness - length, wy,
				   height + xthickness, wy);	    
		}
	    }
	}
	break;
    }
}


static void
gtk_ext_vruler_draw_pos (GtkExtRuler *ruler)
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

    g_return_if_fail (ruler != NULL);
    g_return_if_fail (GTK_IS_EXT_VRULER (ruler));

    if (GTK_WIDGET_DRAWABLE (ruler))
    {
	widget = GTK_WIDGET (ruler);

	gc = widget->style->fg_gc[widget->state];
	xthickness = widget->style->klass->xthickness;
	ythickness = widget->style->klass->ythickness;
	width = widget->allocation.width - xthickness * 2;
	height = widget->allocation.height;

	bs_height = width / 2;
	bs_height |= 1;  /* make sure it's odd */
	bs_width = bs_height / 2 + 1;

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

	    increment = (gfloat) height / (ruler->upper - ruler->lower);

	    x = (width + bs_width) / 2 + xthickness;
	    y = ROUND ((ruler->position - ruler->lower) * increment) + (ythickness - bs_height) / 2 - 1;
	  
	    for (i = 0; i < bs_width; i++)
		gdk_draw_line (widget->window, gc,
			       x + i, y + i,
			       x + i, y + bs_height - 1 - i);
	  
	    ruler->xsrc = x;
	    ruler->ysrc = y;
	}
    }
}


/* Vertical ruler drag event handlers */
static gint gtk_ext_vruler_drag_start(GtkWidget *widget, GdkEventButton *event)
{
    GtkExtRuler *ruler;
    gint width;
    g_return_val_if_fail(GTK_IS_EXT_VRULER(widget),0);
    ruler = GTK_EXT_RULER(widget);
    width = widget->allocation.height;  
    if(ruler->dragging)
	return FALSE;
    ruler->dragging = TRUE;
    ruler->drag_start = (width - event->y);
    ruler->drag_button = event->button;
    ruler->lower1 = ruler->lower;
    ruler->upper1 = ruler->upper;
    gdk_pointer_grab(widget->window,FALSE,
		     GDK_POINTER_MOTION_MASK|GDK_BUTTON_RELEASE_MASK,
		     NULL,NULL,event->time);
    return FALSE;
}

/*   */
static gint gtk_ext_vruler_drag_end(GtkWidget *widget, GdkEventButton *event)
{
    GtkExtRuler *ruler;

    gdk_pointer_ungrab(event->time);
    g_return_val_if_fail(GTK_IS_EXT_VRULER(widget),0);
    ruler = GTK_EXT_RULER(widget);

    ruler->dragging = FALSE;
    gtk_signal_emit_by_name(GTK_OBJECT(widget),"range_changed",NULL);
    return FALSE;
}

/*   */
void gtk_ext_vruler_drag_motion(GtkWidget *widget,GdkEventMotion *event)
{
    gdouble dx,view_w,xinc;
    GtkExtRuler *ruler;
    gint width;
    g_return_if_fail(GTK_IS_EXT_VRULER(widget));
    ruler = GTK_EXT_RULER(widget);
    width = widget->allocation.height;  
    
    dx = (width - event->y) - ruler->drag_start;
    view_w = ruler->upper1 - ruler->lower1;
    switch(ruler->drag_button)
    {
    case 1: /* Button 1 -> Shift mode */
	ruler->drag_start = (width - event->y);
	xinc = dx/width*view_w;
	gtk_ext_ruler_set_range(ruler, ruler->lower - xinc, ruler->upper - xinc);
	break;
    case 3: /* Button 3 -> Scale mode */
    {				
	gdouble xi,xf,xc,scale;
	xc = ruler->lower1+ruler->drag_start/width*view_w;
	scale = exp( -((width - event->y) - ruler->drag_start)/100 );
	xi = xc - (xc - ruler->lower1)*scale;
	xf = xc + (ruler->upper1 - xc)*scale;
	gtk_ext_ruler_set_range(ruler, xi, xf);
    }
    break;
    }
}


