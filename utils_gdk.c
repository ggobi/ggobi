/*-- utils_gdk.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#include <gtk/gtk.h>
#include <vars.h>
#include "externs.h"

GdkColor *
NewColor (glong red, glong green, glong blue) {
  gboolean writeable = false, best_match = true;
  GdkColor *c = (GdkColor *) g_malloc (sizeof (GdkColor));

  c->red = red;
  c->green = green;
  c->blue = blue;

/*  if (gdk_colormap_alloc_color(gdk_colormap_get_system (),*/
  if (gdk_colormap_alloc_color(gdk_rgb_get_cmap (),
    c, writeable, best_match) == false)
  {
    g_printerr("Unable to allocate color\n");
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
draw_glyph (GdkDrawable *drawable, glyphv *gl, icoords *xypos, gint jpos, ggobid *gg)
{
  gushort size = gl->size + 1;

  switch (gl->type) {

    case PLUS_GLYPH:
      gdk_draw_line (drawable, gg->plot_GC,
        xypos[jpos].x - size, xypos[jpos].y,
        xypos[jpos].x + size, xypos[jpos].y);
      gdk_draw_line (drawable, gg->plot_GC,
        xypos[jpos].x, xypos[jpos].y - size,
        xypos[jpos].x, xypos[jpos].y + size);
      break;
    case X_GLYPH:
      gdk_draw_line (drawable, gg->plot_GC,
        xypos[jpos].x - size, xypos[jpos].y - size,
        xypos[jpos].x + size, xypos[jpos].y + size);
      gdk_draw_line (drawable, gg->plot_GC,
        xypos[jpos].x + size, xypos[jpos].y - size,
        xypos[jpos].x - size, xypos[jpos].y + size);
      break;
    case OPEN_RECTANGLE:
      gdk_draw_rectangle (drawable, gg->plot_GC, false,
        xypos[jpos].x - size, xypos[jpos].y - size,
        2*size, 2*size);
      break;
    case FILLED_RECTANGLE:
      gdk_draw_rectangle (drawable, gg->plot_GC, false,
        xypos[jpos].x - size, xypos[jpos].y - size,
        2*size, 2*size);
      gdk_draw_rectangle (drawable, gg->plot_GC, true,
        xypos[jpos].x - size, xypos[jpos].y - size,
        2*size, 2*size);
      break;
    case OPEN_CIRCLE:
      gdk_draw_arc (drawable, gg->plot_GC, false,
        xypos[jpos].x - size, xypos[jpos].y - size,
        2*size, 2*size, 0, (gshort) 23040);
      break;
    case FILLED_CIRCLE:
      gdk_draw_arc (drawable, gg->plot_GC, false,
        xypos[jpos].x - size, xypos[jpos].y - size,
        2*size, 2*size, 0, (gshort) 23040);
      gdk_draw_arc (drawable, gg->plot_GC, true,
        xypos[jpos].x - size, xypos[jpos].y - size,
        2*size, 2*size, 0, (gshort) 23040);
      break;
    case POINT_GLYPH:
      gdk_draw_point (drawable, gg->plot_GC, xypos[jpos].x, xypos[jpos].y);
      break;
    default:
      g_printerr ("build_glyph: impossible glyph type %d\n", gl->type);
  }
}

void
mousepos_get_pressed (GtkWidget *w, GdkEventButton *event,
                      gboolean *btn1_down_p, gboolean *btn2_down_p, splotd *sp)
{
  *btn1_down_p = false;
  *btn2_down_p = false;

  sp->mousepos.x = (gint) event->x;
  sp->mousepos.y = (gint) event->y;
  if ((event->state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
    *btn1_down_p = true;
  else if ((event->state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
    *btn2_down_p = true;
  else if ((event->state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
    *btn2_down_p = true;
}

void
mousepos_get_motion (GtkWidget *w, GdkEventMotion *event,
                     gboolean *btn1_down_p, gboolean *btn2_down_p, splotd *sp)
{
  GdkModifierType state;

  *btn1_down_p = false;
  *btn2_down_p = false;

  /*-- that is, if using motion hints --*/
  if (event->is_hint) {

    gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y,
      &state);
    if ((state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
      *btn1_down_p = true;
    else if ((state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
      *btn2_down_p = true;
    else if ((state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
      *btn2_down_p = true;

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
}

gboolean
mouseinwindow (splotd *sp) {
  return (0 < sp->mousepos.x && sp->mousepos.x < sp->max.x &&
          0 < sp->mousepos.y && sp->mousepos.y < sp->max.y) ;

}
