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

#ifndef __GTK_EXT_HRULER_H__
#define __GTK_EXT_HRULER_H__


#include <gdk/gdk.h>
#include "gtkextruler.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_EXT_HRULER(obj)          GTK_CHECK_CAST (obj, gtk_ext_hruler_get_type (), GtkExtHRuler)
#define GTK_EXT_HRULER_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gtk_ext_hruler_get_type (), GtkExtHRulerClass)
#define GTK_IS_EXT_HRULER(obj)       GTK_CHECK_TYPE (obj, gtk_ext_hruler_get_type ())


typedef struct _GtkExtHRuler       GtkExtHRuler;
typedef struct _GtkExtHRulerClass  GtkExtHRulerClass;
    
typedef enum {
    GTK_EXT_HRULER_FACING_UP, GTK_EXT_HRULER_FACING_DOWN
} GtkExtHRulerFacing;

struct _GtkExtHRuler
{
    GtkExtRuler ruler;
    GtkExtHRulerFacing facing;
};
    
struct _GtkExtHRulerClass
{
    GtkExtRulerClass parent_class;
};


guint      gtk_ext_hruler_get_type   (void);
GtkWidget* gtk_ext_hruler_new        (void);
void       gtk_ext_hruler_set_facing (GtkExtHRuler       *hruler,
				      GtkExtHRulerFacing  facing);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_EXT_HRULER_H__ */
 

