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
 * Modified by Deborah Swayne, dfs@research.att.com, to
 * use myrint rather than rint, since it seemed that I couldn't
 * rely on rint being present in all environments.
*/


#include <string.h>  /*-- dfs, linux gcc is complaining --*/
#include <math.h>
#include <gtk/gtksignal.h>
#include "gtkextruler.h"

static void gtk_ext_ruler_class_init    (GtkExtRulerClass  *klass);
static void gtk_ext_ruler_init          (GtkExtRuler       *ruler);
static void gtk_ext_ruler_realize       (GtkWidget      *widget);
static void gtk_ext_ruler_unrealize     (GtkWidget      *widget);
static void gtk_ext_ruler_size_allocate (GtkWidget      *widget,
					 GtkAllocation  *allocation);
static gint gtk_ext_ruler_expose        (GtkWidget      *widget,
					 GdkEventExpose *event);
static void gtk_ext_ruler_make_pixmap   (GtkExtRuler       *ruler);


#ifdef __cplusplus
extern "C" {
#endif

#ifdef Darwin
#define myrint rint
#else
extern gdouble myrint(gdouble);
#endif

#ifdef __cplusplus
}
#endif


static GtkWidgetClass *parent_class;
enum { RANGE_CHANGED , LAST_SIGNAL };    
static guint ext_ruler_signals[LAST_SIGNAL] = { 0 };

guint
gtk_ext_ruler_get_type (void)
{
    static guint ruler_type = 0;
    
    if (!ruler_type)
    {
	static const GtkTypeInfo ruler_info =
	{
    /* Name is GtkRuler instead of GtkExtRuler because of styles and themes */
	    "GtkRuler",
	    sizeof (GtkExtRuler), 
	    sizeof (GtkExtRulerClass),
	    (GtkClassInitFunc) gtk_ext_ruler_class_init,
	    (GtkObjectInitFunc) gtk_ext_ruler_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};
	
	ruler_type = gtk_type_unique (gtk_widget_get_type (), &ruler_info);
    }
    
    return ruler_type;
}

static gint
gtk_ext_ruler_enter_notify (GtkWidget *widget,GdkEventCrossing *event)
{
    GtkExtRuler *ruler;
    g_return_val_if_fail(GTK_IS_EXT_RULER(widget),0);
    ruler = (GtkExtRuler*)widget;
    gtk_widget_set_state(widget,GTK_STATE_PRELIGHT);
    gtk_ext_ruler_draw_ticks(ruler);
    gtk_widget_queue_draw(widget);
    /*g_print(__FUNCTION__ "\n");*/
    return TRUE;
}

static gint gtk_ext_ruler_leave_notify(GtkWidget *widget,GdkEventCrossing *event)
{
    GtkExtRuler *ruler;
    g_return_val_if_fail(GTK_IS_EXT_RULER(widget),0);
    ruler = (GtkExtRuler*)widget;
    gtk_widget_set_state(widget,GTK_STATE_NORMAL);
    gtk_ext_ruler_draw_ticks(ruler);
    gtk_widget_queue_draw(widget);
    /*g_print(__FUNCTION__ "\n");*/
    return TRUE;
}


static void
gtk_ext_ruler_class_init(GtkExtRulerClass *klass)
{
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;
    gint i;
    
    object_class = (GtkObjectClass*) klass;
    widget_class = (GtkWidgetClass*) klass;
    
    parent_class = (GtkWidgetClass *) gtk_type_class (gtk_widget_get_type ());
    
    widget_class->realize = gtk_ext_ruler_realize;
    widget_class->unrealize = gtk_ext_ruler_unrealize;
    widget_class->size_allocate = gtk_ext_ruler_size_allocate;
    widget_class->expose_event = gtk_ext_ruler_expose;
    widget_class->enter_notify_event = gtk_ext_ruler_enter_notify;
    widget_class->leave_notify_event = gtk_ext_ruler_leave_notify;
    
    klass->draw_ticks = NULL;
    klass->draw_pos = NULL;
    klass->range_changed = NULL;
    
    for(i=2; i<10; i++)
	klass->log29[i-2] = log10((gdouble)i);
    
    ext_ruler_signals[RANGE_CHANGED] = 
	gtk_signal_new( "range_changed",
			(GtkSignalRunType) (GTK_RUN_ACTION|GTK_RUN_NO_RECURSE),
			object_class->type,
			GTK_SIGNAL_OFFSET (GtkExtRulerClass,range_changed),
			gtk_marshal_NONE__NONE,
			GTK_TYPE_NONE, 0);
    gtk_object_class_add_signals (object_class, ext_ruler_signals, LAST_SIGNAL);
}

static void
gtk_ext_ruler_init (GtkExtRuler *ruler)
{
    ruler->backing_store = NULL;
    ruler->non_gr_exp_gc = NULL;
    ruler->xsrc = 0;
    ruler->ysrc = 0;
    ruler->slider_size = 0;
    ruler->lower = 0;
    ruler->upper = 1;
    ruler->position = 0;
    ruler->dragging = 0;
    ruler->sync_ruler = NULL;
    ruler->sync_flags = GTK_EXT_RULER_SYNC_DRAG;
    ruler->synthetic_event = FALSE;
    gtk_ext_ruler_set_mode (ruler, GTK_EXT_RULER_LINEAR);
}

void
gtk_ext_ruler_set_mode (GtkExtRuler      *ruler,
			GtkExtRulerMode  mode)
{
    g_return_if_fail (ruler != NULL);
    g_return_if_fail (GTK_IS_EXT_RULER (ruler));
  
    ruler->mode = mode;
  
    if (GTK_WIDGET_DRAWABLE (ruler)) {
	gtk_widget_queue_draw (GTK_WIDGET (ruler));
    }
}

void
gtk_ext_ruler_set_range (GtkExtRuler *ruler,
			 gdouble    lower,
			 gdouble    upper)
{
    g_return_if_fail (ruler != NULL);
    g_return_if_fail (GTK_IS_EXT_RULER (ruler));

    ruler->lower = lower;
    ruler->upper = upper;

    if (GTK_WIDGET_DRAWABLE (ruler)) {
	gtk_ext_ruler_draw_ticks (ruler);
	gtk_widget_queue_draw (GTK_WIDGET (ruler));
    }
}

void
gtk_ext_ruler_draw_ticks (GtkExtRuler *ruler)
{
    g_return_if_fail (ruler != NULL);
    g_return_if_fail (GTK_IS_EXT_RULER (ruler));

    if (GTK_EXT_RULER_CLASS (GTK_OBJECT (ruler)->klass)->draw_ticks)
	(* GTK_EXT_RULER_CLASS (GTK_OBJECT (ruler)->klass)->draw_ticks) (ruler);
}

void
gtk_ext_ruler_draw_pos (GtkExtRuler *ruler)
{
    g_return_if_fail (ruler != NULL);
    g_return_if_fail (GTK_IS_EXT_RULER (ruler));

    if (GTK_EXT_RULER_CLASS (GTK_OBJECT (ruler)->klass)->draw_pos)
	(* GTK_EXT_RULER_CLASS (GTK_OBJECT (ruler)->klass)->draw_pos) (ruler);
}


static void
gtk_ext_ruler_realize (GtkWidget *widget)
{
    GtkExtRuler *ruler;
    GdkWindowAttr attributes;
    gint attributes_mask;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_EXT_RULER (widget));

    ruler = GTK_EXT_RULER (widget);
    GTK_WIDGET_SET_FLAGS (ruler, GTK_REALIZED);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.event_mask = gtk_widget_get_events (widget);
    attributes.event_mask |= 
	( GDK_EXPOSURE_MASK |
	  GDK_POINTER_MOTION_MASK |
	  GDK_POINTER_MOTION_HINT_MASK |
	  GDK_BUTTON_PRESS_MASK |
	  GDK_BUTTON_RELEASE_MASK |
	  GDK_ENTER_NOTIFY_MASK |
	  GDK_LEAVE_NOTIFY_MASK );
  
    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
    gdk_window_set_user_data (widget->window, ruler);

    widget->style = gtk_style_attach (widget->style, widget->window);
    gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);

    gtk_ext_ruler_make_pixmap (ruler);
    gtk_ext_ruler_draw_ticks(ruler);
}

static void
gtk_ext_ruler_unrealize (GtkWidget *widget)
{
    GtkExtRuler *ruler;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_EXT_RULER (widget));

    ruler = GTK_EXT_RULER (widget);

    if (ruler->backing_store)
	gdk_pixmap_unref (ruler->backing_store);
    if (ruler->non_gr_exp_gc)
	gdk_gc_destroy (ruler->non_gr_exp_gc);

    ruler->backing_store = NULL;
    ruler->non_gr_exp_gc = NULL;

    if (GTK_WIDGET_CLASS (parent_class)->unrealize)
	(* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}

static void
gtk_ext_ruler_size_allocate (GtkWidget     *widget,
			     GtkAllocation *allocation)
{
    GtkExtRuler *ruler;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_EXT_RULER (widget));

    ruler = GTK_EXT_RULER (widget);
    widget->allocation = *allocation;

    if (GTK_WIDGET_REALIZED (widget))
    {
	gdk_window_move_resize (widget->window,
				allocation->x, allocation->y,
				allocation->width, allocation->height);

	gtk_ext_ruler_make_pixmap (ruler);
	gtk_ext_ruler_draw_ticks(ruler);
    }
}

static gint
gtk_ext_ruler_expose (GtkWidget      *widget,
		      GdkEventExpose *event)
{
    GtkExtRuler *ruler;

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_EXT_RULER (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    if (GTK_WIDGET_DRAWABLE (widget))
    {
	ruler = GTK_EXT_RULER (widget);
	gdk_draw_pixmap (widget->window,
			 ruler->non_gr_exp_gc,
			 ruler->backing_store,
			 0, 0, 0, 0,
			 widget->allocation.width,
			 widget->allocation.height);
	gtk_ext_ruler_draw_pos (ruler);
    }

    return FALSE;
}

static void
gtk_ext_ruler_make_pixmap (GtkExtRuler *ruler)
{
    GtkWidget *widget;
    gint width;
    gint height;

    widget = GTK_WIDGET (ruler);

    if (ruler->backing_store)
    {
	gdk_window_get_size (ruler->backing_store, &width, &height);
	if ((width == widget->allocation.width) &&
	    (height == widget->allocation.height))
	    return;

	gdk_pixmap_unref (ruler->backing_store);
    }

    ruler->backing_store = gdk_pixmap_new (widget->window,
					   widget->allocation.width,
					   widget->allocation.height,
					   -1);

    ruler->xsrc = 0;
    ruler->ysrc = 0;

    if (!ruler->non_gr_exp_gc)
    {
	ruler->non_gr_exp_gc = gdk_gc_new (widget->window);
	gdk_gc_set_exposures (ruler->non_gr_exp_gc, FALSE);
    }
}



void
gtk_ext_ruler_calc_scale (GtkExtRuler *ruler, gchar direction)
{
    GtkWidget *widget;
    GdkGC *gc, *bg_gc;
    GdkFont *font;
    gint width, height, xthickness, ythickness, size;
    gchar unit_str[32];
    gint digit_height;
    gint text_width,text_width1,text_width2; 
    gdouble tbe,div,subdiv,x0,xi,xf,dx,k;
    gfloat wsubdiv;
    gint ndiv;

    /*-- dfs: subdiv is never assigned a value, but its value is used --*/

    /*g_return_if_fail (ruler != NULL);*/
    /*g_return_if_fail (GTK_IS_MYHRULER (ruler));*/

    widget = GTK_WIDGET (ruler);

    gc = widget->style->fg_gc[GTK_STATE_NORMAL];
    bg_gc = widget->style->bg_gc[GTK_STATE_NORMAL];
    font = widget->style->font;

    xthickness = widget->style->klass->xthickness;
    ythickness = widget->style->klass->ythickness;
    digit_height = font->ascent; /* assume descent == 0 ? */

    if(direction=='h') {
	width = widget->allocation.width;
	height = widget->allocation.height - ythickness * 2;
	size = width;
    } else if(direction=='v') {
	width = widget->allocation.height;
	height = widget->allocation.width - ythickness * 2;
	size = width;
    } else
	g_warning("Unknown direction '%c' (this is a bug in gtk_ext_ruler_calc_scale()",direction);

    xf = ruler->upper;
    xi = ruler->lower;
    dx = xf - xi;
    g_return_if_fail(dx > 0);

    switch(ruler->mode) 
    {
	gint n;
    case GTK_EXT_RULER_LINEAR:
	ndiv = 10;
	for(n=5; n; n--) { /* iterate 5 times to find optimum division size */
	    /* div: length of each division */
	    tbe=log10(dx/ndiv);   /* looking for approx. 'ndiv' divisions in a length 'dx' */
	    div=pow(10,myrint(tbe));	/* div: power of 10 closest to dx/ndiv */
	    if(fabs(div/2-dx/ndiv) < fabs(div-dx/ndiv)) /* test if div/2 is closer to dx/ndiv */
		div/=2;
	    else if(fabs(div*2-dx/ndiv) < fabs(div-dx/ndiv))
		div*=2;			/* test if div*2 is closer to dx/ndiv */
	    x0=div*ceil(xi / div) - div;
	    sprintf (unit_str, "  %G ", x0+div);
	    text_width1 = strlen (unit_str) * digit_height + 1;
	    sprintf (unit_str, "  %G ", x0+ndiv*div);
	    text_width2 = strlen (unit_str) * digit_height + 1;
	    text_width = MAX(text_width1,text_width2);
	    if(n > 1) 
               ndiv = (gint) myrint(size / text_width);
	}
	wsubdiv = subdiv/dx*size; 
      
	ruler->rtick_start = x0;
	ruler->rtick_div = div;
	ruler->rtick_subdiv = div/5;
	ruler->rtick_limit = ruler->upper;
	k = size/dx;
	ruler->tick_start = (x0-xi)*k;
	ruler->tick_div = div*k;
	ruler->tick_subdiv = ruler->rtick_subdiv*k;
	ruler->tick_limit = size;
	break;
    case GTK_EXT_RULER_LOG:
	x0 = ceil(xi)-1;
	div = 1.0;
	ruler->rtick_start = x0;
	ruler->rtick_div = div;
	ruler->rtick_subdiv = 0;
	ruler->rtick_limit = ruler->upper;
	k = size/dx;
	ruler->tick_start = (x0-xi)*k;
	ruler->tick_div = /* div* */k;
	ruler->tick_subdiv = 0;
	ruler->tick_limit = size;
	break;
    }
}

void gtk_ext_ruler_sync_update(GtkExtRuler *ruler)
{
    g_return_if_fail(GTK_IS_EXT_RULER(ruler));
    if(!ruler->sync_ruler) return;

}

void gtk_ext_ruler_set_sync_ruler(GtkExtRuler *ruler,GtkExtRuler *sync_ruler)
{
    g_return_if_fail(GTK_IS_EXT_RULER(ruler));
    if(sync_ruler) {
	g_return_if_fail(GTK_IS_EXT_RULER(sync_ruler));
	/* Can't sync hruler with vruler (TODO) */
	g_return_if_fail(GTK_OBJECT_TYPE(sync_ruler) == GTK_OBJECT_TYPE (ruler)); 
	ruler->sync_ruler = sync_ruler;
	if(ruler->sync_flags&GTK_EXT_RULER_SYNC_ABSOLUTE)
	    gtk_ext_ruler_set_range(ruler->sync_ruler,ruler->lower,ruler->upper);
    }
    else
	ruler->sync_ruler = NULL;
}

void gtk_ext_ruler_set_sync_flags(GtkExtRuler *ruler, GtkExtRulerSyncFlags sync_flags)
{
    g_return_if_fail(GTK_IS_EXT_RULER(ruler));
    ruler->sync_flags = sync_flags;
}

