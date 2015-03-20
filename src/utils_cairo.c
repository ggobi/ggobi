/*-- utils_cairo.c --*/
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Eclipse Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
 */

#include <gtk/gtk.h>
#include "externs.h"

void
show_buffer (cairo_t *cr, cairo_surface_t *buffer) {
  cairo_set_source_surface(cr, buffer);
  cairo_paint(cr);
}

cairo_surface_t *
create_buffer (GtkWidget *w) {
  gdk_window_create_similar_image_surface (w->window,
                                           CAIRO_FORMAT_ARGB32,
                                           w->allocation.width,
                                           w->allocation.height,
                                           0);
}

cairo_pattern_t *
NewColor (glong red, glong green, glong blue)
{
  return cairo_pattern_create_rgb(red, green, blue);
}

/*
 * The plotted glyph is actually 2*size + 1 on a side, so the
 * size progression is  5, 7, 9, 11, 13, ...     That's
 * because it seems necessary to have glyphs that have odd
 * sizes in order to make sure the point is at the center of
 * the glyph.  That may be overly fastidious for large glyphs,
 * but it's neceessary for the small ones.
 */
void
draw_glyph (cairo_t * cr, glyphd * gl, icoords * xypos, gint jpos,
            ggobid * gg)
{
  gushort size = gl->size + 1;

  switch (gl->type) {

  case PLUS:
    cairo_move_to(cr, xypos[jpos].x - size, xypos[jpos].y);
    cairo_line_to(cr, xypos[jpos].x + size, xypos[jpos].y);
    cairo_move_to(cr, xypos[jpos].x, xypos[jpos].y - size);
    cairo_line_to(cr, xypos[jpos].x, xypos[jpos].y + size);
    break;
  case X:
    cairo_move_to(cr, xypos[jpos].x - size, xypos[jpos].y - size);
    cairo_line_to(cr, xypos[jpos].x + size, xypos[jpos].y + size);
    cairo_move_to(cr, xypos[jpos].x + size, xypos[jpos].y - size);
    cairo_line_to(cr, xypos[jpos].x - size, xypos[jpos].y + size);
    break;
  case OR:
  case FR:
    cairo_rectangle(cr,
                    xypos[jpos].x - size, xypos[jpos].y - size,
                    2 * size, 2 * size);
    break;
  case OC:
  case FC:
    cairo_arc (cr, xypos[jpos].x, xypos[jpos].y, size, 0, 2 * M_PI);
    break;
  case DOT_GLYPH:
    cairo_rectangle(cr, xypos[jpos].x - 0.5, xypos[jpos].y - 0.5, 1., 1.)
      break;
  case UNKNOWN_GLYPH:
  default:
    g_printerr ("build_glyph: impossible glyph type %d\n", gl->type);
  }

  if (gl->type == FC || gl->type == FR) {
    cairo_fill(cr);
  }
  cairo_stroke(cr);
}

void draw_polygon(cairo_t *cr, GdkPoint *points, int npoints) {
  if (npoints == 0)
    return;
  cairo_move_to(cr, points[0].x, points[0].y);
  for (int i = 1; i < npoints; i++) {
    cairo_line_to(cr, points[i].x, points[i].y);
  }
  cairo_close_path(cr);
}

void draw_segments(cairo_t *cr, GdkSegment *segments, int nsegments) {
  for (int i = 0; i < nsegments; i++) {
    cairo_move_to(cr, segments[i].x1, segments[i].y1);
    cairo_line_to(cr, segments[i].x2, segments[i].y2);
  }
}


/*--------------------------------------------------------------------*/
/*              Drawing 3D sliders                                    */
/*--------------------------------------------------------------------*/

/* (x,y) is the center of the rectangle */
void
draw_3drectangle (GtkWidget * widget, cairo_t * cr,
                  gint x, gint y, gint width, gint height, ggobid * gg)
{
  GdkPoint points[7];
  gint w = width / 2;
  gint h = height / 2;

  /*-- draw the rectangles --*/
  cairo_set_source(cr, gg->mediumgray);
  cairo_rect(cr, x - w, y - h, width, height);
  cairo_fill(cr);
  
  /*-- draw the dark shadows --*/
  cairo_set_source (cr, gg->darkgray);
  cairo_translate(cr, x, y);
  cairo_move_to(cr, -w, h);
  cairo_line_to(cr, w, h);
  cairo_line_to(cr, w, -h);
  cairo_line_to(cr, w - 1, -h + 1);
  cairo_line_to(cr, w - 1, h - 1);
  cairo_line_to(cr, -w + 1, h - 1);
  cairo_close_path(cr);
  cairo_move_to(cr, -1, -h + 1);
  cairo_line_to(cr, -1, h - 2);
  cairo_fill(cr);
  cairo_stroke(cr);
  
  /*-- draw the light shadows --*/
  cairo_set_source(cr, gg->lightgray);
  cairo_move_to(cr, -w, h - 1);
  cairo_line_to(cr, -w, -h);
  cairo_line_to(cr, w - 1, -h);
  cairo_line_to(cr, w - 2, -h + 1);
  cairo_line_to(cr, -w + 1, -h + 1);
  cairo_line_to(cr, -w + 1, h - 2);
  cairo_close_path(cr);
  cairo_move_to(cr, 0, h + 1);
  cairo_line_to(cr, 0, h - 2);
  cairo_fill(cr);
  cairo_stroke(cr);

  cairo_translate(cr, -x, -y);
}
