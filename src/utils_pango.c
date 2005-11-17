/*-- utils_gdk.c --*/
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

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"
#include <string.h>

/** Lays out the text using the given layout. If 'rect' is non-NULL it contains
	the logical text extents. */
void
layout_text(PangoLayout *layout, const gchar *text, PangoRectangle *rect)
{	
	if (text) {
		pango_layout_set_text(layout, text, -1);
		if (rect)
			pango_layout_get_pixel_extents(layout, NULL, rect);
	}
}
void
underline_text(PangoLayout *layout)
{
	PangoAttrList *list;
	PangoAttribute *attr;
	
	attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
	attr->start_index = 0;
	attr->end_index = strlen(pango_layout_get_text(layout));
	
	list = pango_attr_list_new();
	pango_attr_list_insert(list, attr);
	
	pango_layout_set_attributes(layout, list);
	
	pango_attr_list_unref(list);
}
