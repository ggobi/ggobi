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

#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#ifdef ENABLE_CAIRO
#ifndef WIN32
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#else
  // win32 includes
#endif
#include <cairo-glitz.h>
#endif

GdkColor *
NewColor (glong red, glong green, glong blue)
{
  gboolean writeable = false, best_match = true;
  GdkColor *c = (GdkColor *) g_malloc (sizeof (GdkColor));

  c->red = red;
  c->green = green;
  c->blue = blue;

  if (gdk_colormap_alloc_color (gdk_colormap_get_system (),
                                c, writeable, best_match) == false) {
    g_printerr ("Unable to allocate color\n");
    c = NULL;
  }

  return (c);
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
draw_glyph (GdkDrawable * drawable, glyphd * gl, icoords * xypos, gint jpos,
            ggobid * gg)
{
  gushort size = gl->size + 1;

  switch (gl->type) {

  case PLUS:
    gdk_draw_line (drawable, gg->plot_GC,
                   xypos[jpos].x - size, xypos[jpos].y,
                   xypos[jpos].x + size, xypos[jpos].y);
    gdk_draw_line (drawable, gg->plot_GC,
                   xypos[jpos].x, xypos[jpos].y - size,
                   xypos[jpos].x, xypos[jpos].y + size);
    break;
  case X:
    gdk_draw_line (drawable, gg->plot_GC,
                   xypos[jpos].x - size, xypos[jpos].y - size,
                   xypos[jpos].x + size, xypos[jpos].y + size);
    gdk_draw_line (drawable, gg->plot_GC,
                   xypos[jpos].x + size, xypos[jpos].y - size,
                   xypos[jpos].x - size, xypos[jpos].y + size);
    break;
  case OR:
    gdk_draw_rectangle (drawable, gg->plot_GC, false,
                        xypos[jpos].x - size, xypos[jpos].y - size,
                        2 * size, 2 * size);
    break;
  case FR:
    gdk_draw_rectangle (drawable, gg->plot_GC, false,
                        xypos[jpos].x - size, xypos[jpos].y - size,
                        2 * size, 2 * size);
    gdk_draw_rectangle (drawable, gg->plot_GC, true,
                        xypos[jpos].x - size, xypos[jpos].y - size,
                        2 * size, 2 * size);
    break;
  case OC:
    gdk_draw_arc (drawable, gg->plot_GC, false,
                  xypos[jpos].x - size, xypos[jpos].y - size,
                  2 * size, 2 * size, 0, (gshort) 23040);
    break;
  case FC:
    gdk_draw_arc (drawable, gg->plot_GC, false,
                  xypos[jpos].x - size, xypos[jpos].y - size,
                  2 * size, 2 * size, 0, (gshort) 23040);
    gdk_draw_arc (drawable, gg->plot_GC, true,
                  xypos[jpos].x - size, xypos[jpos].y - size,
                  2 * size, 2 * size, 0, (gshort) 23040);
    break;
  case DOT_GLYPH:
    gdk_draw_point (drawable, gg->plot_GC, xypos[jpos].x, xypos[jpos].y);
    break;
  case UNKNOWN_GLYPH:
  default:
    g_printerr ("build_glyph: impossible glyph type %d\n", gl->type);
  }
}

void
mousepos_get_pressed (GtkWidget * w, GdkEventButton * event,
                      gboolean * btn1_down_p, gboolean * btn2_down_p,
                      splotd * sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  gint grab_ok;
  GdkModifierType state;

  *btn1_down_p = false;
  *btn2_down_p = false;

  gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y,
                          &state);

  grab_ok = gdk_pointer_grab (sp->da->window,
                              false,
                              (GdkEventMask) (GDK_POINTER_MOTION_MASK |
                                              GDK_BUTTON_RELEASE_MASK),
                              (GdkWindow *) NULL, (GdkCursor *) NULL,
                              event->time);

  if ((event->state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
    *btn1_down_p = true;
  else if ((event->state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
    *btn2_down_p = true;
  else if ((event->state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
    *btn2_down_p = true;

  if (*btn1_down_p)
    gg->buttondown = 1;
  else if (*btn2_down_p)
    gg->buttondown = 2;
}

void
mousepos_get_motion (GtkWidget * w, GdkEventMotion * event,
                     gboolean * btn1_down_p, gboolean * btn2_down_p,
                     splotd * sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  GdkModifierType state;

  *btn1_down_p = false;
  *btn2_down_p = false;

  /*-- that is, if using motion hints --*/
/*
  if (event->is_hint) {
*/

  gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y,
                          &state);
  if ((state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
    *btn1_down_p = true;
  else if ((state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
    *btn2_down_p = true;
  else if ((state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
    *btn2_down_p = true;

/*
  } else {

    sp->mousepos.x = (gint) event->x;
    sp->mousepos.y = (gint) event->y;
    if ((event->state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
      *btn1_down_p = true;
    else if ((event->state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
      *btn2_down_p = true;
    else if ((event->state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
      *btn2_down_p = true;
  }
*/

  if (*btn1_down_p)
    gg->buttondown = 1;
  else if (*btn2_down_p)
    gg->buttondown = 2;
}

gboolean
mouseinwindow (splotd * sp)
{
  return (0 < sp->mousepos.x && sp->mousepos.x < sp->max.x &&
          0 < sp->mousepos.y && sp->mousepos.y < sp->max.y);

}

/*--------------------------------------------------------------------*/
/*              Drawing 3D sliders                                    */
/*--------------------------------------------------------------------*/

/* (x,y) is the center of the rectangle */
void
draw_3drectangle (GtkWidget * widget, GdkDrawable * drawable,
                  gint x, gint y, gint width, gint height, ggobid * gg)
{
  GdkPoint points[7];
  gint w = width / 2;
  gint h = height / 2;

  if (gg->rectangle_GC == NULL)
    gg->rectangle_GC = gdk_gc_new (widget->window);

  /*-- draw the rectangles --*/
  gdk_gc_set_foreground (gg->rectangle_GC, &gg->mediumgray);
  gdk_draw_rectangle (drawable, gg->rectangle_GC, TRUE,
                      x - w, y - h, width, height);

  /*-- draw the dark shadows --*/
  gdk_gc_set_foreground (gg->rectangle_GC, &gg->darkgray);
  points[0].x = x - w;
  points[0].y = y + h;
  points[1].x = x + w;
  points[1].y = y + h;
  points[2].x = x + w;
  points[2].y = y - h;

  points[3].x = points[2].x - 1;
  points[3].y = points[2].y + 1;
  points[4].x = points[1].x - 1;
  points[4].y = points[1].y - 1;
  points[5].x = points[0].x + 1;
  points[5].y = points[0].y - 1;

  points[6].x = x - w;
  points[6].y = y + h;
  gdk_draw_polygon (drawable, gg->rectangle_GC, TRUE, points, 7);
  gdk_draw_line (drawable, gg->rectangle_GC,
                 x - 1, y - (h - 1), x - 1, y + (h - 2));

  /*-- draw the light shadows --*/
  gdk_gc_set_foreground (gg->rectangle_GC, &gg->lightgray);
  points[0].x = x - w;   /*-- lower left --*/
  points[0].y = y + (h - 1);
  points[1].x = x - w;   /*-- upper left --*/
  points[1].y = y - h;
  points[2].x = x + (w - 1); /*-- upper right --*/
  points[2].y = y - h;

  points[3].x = points[2].x - 1;
  points[3].y = points[2].y + 1;
  points[4].x = points[1].x + 1;
  points[4].y = points[1].y + 1;
  points[5].x = points[0].x + 1;
  points[5].y = points[0].y - 1;

  points[6].x = points[0].x;
  points[6].y = points[0].y;
  gdk_draw_polygon (drawable, gg->rectangle_GC, TRUE, points, 7);
  gdk_draw_line (drawable, gg->rectangle_GC, x, y - (h - 1), x, y + (h - 2));
}

#ifdef ENABLE_CAIRO
cairo_t *
create_cairo_glitz (GdkDrawable * drawable)
{
  cairo_surface_t *cairo_surface =
    g_object_get_data (G_OBJECT (drawable), "glitz_surface");

  if (!cairo_surface) {
    guint width, height;
    glitz_drawable_t *glitz_d = NULL;
    glitz_surface_t *glitz_s;
    glitz_format_t *format;
    GdkVisual *visual = gdk_drawable_get_visual (drawable);
    g_return_val_if_fail (visual != NULL, NULL);
    gdk_drawable_get_size (drawable, &width, &height);
#ifndef WIN32
    glitz_glx_init (NULL);
    {
      Display *dpy = GDK_DRAWABLE_XDISPLAY (drawable);
      glitz_drawable_format_t *d_format =
        glitz_glx_find_drawable_format_for_visual (dpy, DefaultScreen (dpy),
                                                   GDK_VISUAL_XVISUAL
                                                   (visual)->visualid);
      if (GDK_IS_WINDOW (drawable))
        glitz_d =
          glitz_glx_create_drawable_for_window (dpy, DefaultScreen (dpy),
                                                d_format,
                                                GDK_WINDOW_XID (GDK_WINDOW
                                                                (drawable)),
                                                width, height);
      else
        glitz_d = glitz_glx_create_pbuffer_drawable (dpy, DefaultScreen (dpy),
                                                     d_format, width, height);
    }
#else
    // windows implementation
#endif
    g_return_val_if_fail (glitz_d != NULL, NULL);
    format = glitz_find_standard_format (glitz_d, GLITZ_STANDARD_ARGB32);
    glitz_s = glitz_surface_create (glitz_d, format, width, height, 0, NULL);
    g_object_set_data_full (G_OBJECT (drawable), "glitz_surface",
                            cairo_glitz_surface_create (glitz_s),
                            (GDestroyNotify) cairo_surface_destroy);
  }
  return (cairo_create (cairo_surface));
}
#endif
