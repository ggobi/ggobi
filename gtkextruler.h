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


#ifndef __GTK_EXT_RULER_H__
#define __GTK_EXT_RULER_H__

#include <stdio.h>
#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
    
#define GTK_EXT_RULER(obj)          GTK_CHECK_CAST (obj, gtk_ext_ruler_get_type (), GtkExtRuler)
#define GTK_EXT_RULER_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gtk_ext_ruler_get_type (), GtkExtRulerClass)
#define GTK_IS_EXT_RULER(obj)       GTK_CHECK_TYPE (obj, gtk_ext_ruler_get_type ())

    
typedef struct _GtkExtRuler        GtkExtRuler;
typedef struct _GtkExtRulerClass   GtkExtRulerClass;
    
typedef enum {GTK_EXT_RULER_LINEAR, GTK_EXT_RULER_LOG} GtkExtRulerMode; 

typedef enum { /* Some of these flags are not implemented yet  */
    GTK_EXT_RULER_SYNC_ABSOLUTE=1<<0,
    GTK_EXT_RULER_SYNC_DELAYED=1<<1,
    GTK_EXT_RULER_SYNC_DRAG=1<<2
} GtkExtRulerSyncFlags;

struct _GtkExtRuler
{
    GtkWidget widget;
	    
    GdkPixmap *backing_store;
    GdkGC *non_gr_exp_gc;
    /*GtkRulerMetric *metric;*/
    gint xsrc, ysrc;
    gint slider_size;
    
    /* The upper limit of the ruler */
    gdouble lower;
    /* The lower limit of the ruler */
    gdouble upper;
    /* The position of the mark on the ruler */
    gdouble position;
    /* Tick marks information -- this information could be useful to
       the application */
    /* (tick marks in pixel units) */
    gfloat tick_start,tick_div,tick_subdiv,tick_limit;
    /* (tick marks in ruler units) */
    gdouble rtick_start,rtick_div,rtick_subdiv,rtick_limit;
    GtkExtRulerMode mode;
    gdouble lower1,upper1,drag_start;
    gboolean dragging:1;
    guint drag_button;
    GtkExtRuler *sync_ruler;
    GtkExtRulerSyncFlags sync_flags;
    gboolean synthetic_event:1;
};

struct _GtkExtRulerClass
{
    GtkWidgetClass parent_class;

    void (* draw_ticks) (GtkExtRuler *ruler);
    void (* draw_pos)   (GtkExtRuler *ruler);
    void (* range_changed) (GtkExtRuler *ruler);
    gdouble log29[8];		/* log10(2..9) */
};   

GtkType gtk_ext_ruler_get_type       (void);
void    gtk_ext_ruler_set_mode       (GtkExtRuler          *ruler,
				      GtkExtRulerMode       mode);
void    gtk_ext_ruler_set_range      (GtkExtRuler          *ruler,
				      gdouble               lower,
				      gdouble               upper);
void    gtk_ext_ruler_draw_ticks     (GtkExtRuler          *ruler);
void    gtk_ext_ruler_draw_pos       (GtkExtRuler          *ruler);
void    gtk_ext_ruler_set_sync_ruler (GtkExtRuler          *ruler,
				      GtkExtRuler          *sync_ruler);
void    gtk_ext_ruler_set_sync_flags (GtkExtRuler          *ruler,
				      GtkExtRulerSyncFlags  sync_flags);

/* private */
void    gtk_ext_ruler_calc_scale     (GtkExtRuler          *ruler,
				      gchar                 direction);




#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_EXT_RULER_H__ */







