#include <gtk/gtk.h>
#include <vars.h>
#include "externs.h"

GdkColor *
NewColor (glong red, glong green, glong blue) {
  GdkColor *c = (GdkColor *) g_malloc (sizeof (GdkColor));
  c->red = red;
  c->green = green;
  c->blue = blue;
  gdk_color_alloc (gdk_colormap_get_system (), c);

  return (c);
}

void
draw_glyph (GdkDrawable *drawable, glyphv *gl, icoords *xypos, gint jpos)
{
  gushort size;

  switch (gl->type) {

    case PLUS_GLYPH:
      size = gl->size + 2 ;
      gdk_draw_line (drawable, plot_GC,
        xypos[jpos].x - size, xypos[jpos].y,
        xypos[jpos].x + size, xypos[jpos].y);
      gdk_draw_line (drawable, plot_GC,
        xypos[jpos].x, xypos[jpos].y - size,
        xypos[jpos].x, xypos[jpos].y + size);
      break;
    case X_GLYPH:
      size = gl->size;
      gdk_draw_line (drawable, plot_GC,
        xypos[jpos].x - size, xypos[jpos].y - size,
        xypos[jpos].x + size, xypos[jpos].y + size);
      gdk_draw_line (drawable, plot_GC,
        xypos[jpos].x + size, xypos[jpos].y - size,
        xypos[jpos].x - size, xypos[jpos].y + size);
      break;
    case OPEN_RECTANGLE:
      size = 2*size;
      gdk_draw_rectangle (drawable, plot_GC, false,
        xypos[jpos].x - size/2 + 1, xypos[jpos].y - size/2 + 1,
        size, size);
      break;
    case FILLED_RECTANGLE:
      size = size+2;
      gdk_draw_rectangle (drawable, plot_GC, true,
        xypos[jpos].x - size/2 + 1, xypos[jpos].y - size/2 + 1,
        size, size);
      break;
    case OPEN_CIRCLE:
      size = gl->size * 3;
      gdk_draw_arc (drawable, plot_GC, false,
        xypos[jpos].x - size/2, xypos[jpos].y - size/2,
        size, size, 0, (gshort) 23040);
      break;
    case FILLED_CIRCLE:
      size = gl->size * 3;
      gdk_draw_arc (drawable, plot_GC, false,
        xypos[jpos].x - size/2, xypos[jpos].y - size/2,
        size, size, 0, (gshort) 23040);
      gdk_draw_arc (drawable, plot_GC, true,
        xypos[jpos].x - size/2, xypos[jpos].y - size/2,
        size, size, 0, (gshort) 23040);
      break;
    case POINT_GLYPH:
      gdk_draw_point (drawable, plot_GC, xypos[jpos].x, xypos[jpos].y);
      break;
    default:
      g_printerr ("build_glyph: impossible glyph type %d\n", gl->type);
  }
}

void
mousepos_get (GtkWidget *w, GdkEventMotion *event,
              gboolean *btn1_down_p, gboolean *btn2_down_p)
{
  *btn1_down_p = false;
  *btn2_down_p = false;

  if (event->is_hint) {  /*-- that is, if using motion hints --*/

    GdkModifierType state;
    /*-- mousepos is global --*/
    gdk_window_get_pointer (w->window, &mousepos.x, &mousepos.y, &state);
    if ((state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
      *btn1_down_p = true;
    else if ((state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
      *btn2_down_p = true;
    else if ((state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
      *btn2_down_p = true;

  } else {
    /*-- mousepos is global --*/
    mousepos.x = event->x;
    mousepos.y = event->y;
    if ((event->state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
      *btn1_down_p = true;
    else if ((event->state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
      *btn2_down_p = true;
    else if ((event->state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
      *btn2_down_p = true;
  }
}
