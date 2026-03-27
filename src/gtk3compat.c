#include "gtk3compat.h"

#if GTK_MAJOR_VERSION >= 3

#include <math.h>

#undef gdk_cairo_create

typedef struct _GGobiRulerState {
  gdouble lower;
  gdouble upper;
  gdouble position;
  gdouble max_size;
  GtkOrientation orientation;
} GGobiRulerState;

static GQuark ggobi_ruler_state_quark (void);
static GGobiRulerState *ggobi_gtk_ruler_state_ensure (GtkWidget *widget);
static GtkWidget *ggobi_gtk_ruler_new (GtkOrientation orientation);
static gboolean ggobi_gtk_ruler_draw_cb (GtkWidget *widget, cairo_t *cr,
                                         gpointer user_data);
static gdouble ggobi_gtk_ruler_fraction (const GGobiRulerState *state,
                                         gdouble value);

static GQuark
ggobi_ruler_state_quark (void)
{
  return g_quark_from_static_string ("ggobi-gtk3-ruler-state");
}

static GGobiRulerState *
ggobi_gtk_ruler_state_ensure (GtkWidget *widget)
{
  GGobiRulerState *state;

  state = g_object_get_qdata (G_OBJECT (widget), ggobi_ruler_state_quark ());
  if (state != NULL)
    return state;

  state = g_new0 (GGobiRulerState, 1);
  state->upper = 1.0;
  state->max_size = 1.0;
  if (GTK_IS_ORIENTABLE (widget))
    state->orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (widget));
  else
    state->orientation = GTK_ORIENTATION_HORIZONTAL;

  g_object_set_qdata_full (G_OBJECT (widget), ggobi_ruler_state_quark (),
                           state, g_free);
  return state;
}

static gdouble
ggobi_gtk_ruler_fraction (const GGobiRulerState *state, gdouble value)
{
  gdouble denom;
  gdouble frac;

  denom = state->upper - state->lower;
  if (!isfinite (denom) || fabs (denom) < 1e-12)
    return 0.0;

  frac = (value - state->lower) / denom;
  if (!isfinite (frac))
    return 0.0;

  return CLAMP (frac, 0.0, 1.0);
}

static gboolean
ggobi_gtk_ruler_draw_cb (GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
  GGobiRulerState *state = ggobi_gtk_ruler_state_ensure (widget);
  GtkStyleContext *context = gtk_widget_get_style_context (widget);
  GtkStateFlags flags = gtk_style_context_get_state (context);
  GdkRGBA fg;
  gint width = gtk_widget_get_allocated_width (widget);
  gint height = gtk_widget_get_allocated_height (widget);
  gint major_ticks = 10;
  gint minor_ticks = 4;
  gint i, j;
  gdouble marker_frac;

  (void) user_data;

  gtk_render_background (context, cr, 0, 0, width, height);
  gtk_render_frame (context, cr, 0, 0, width, height);

  gtk_style_context_get_color (context, flags, &fg);
  gdk_cairo_set_source_rgba (cr, &fg);
  cairo_set_line_width (cr, 1.0);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);

  if (state->orientation == GTK_ORIENTATION_HORIZONTAL) {
    gdouble baseline = 4.5;
    gdouble usable = MAX (1, width - 1);

    cairo_move_to (cr, 0.5, baseline);
    cairo_line_to (cr, width - 0.5, baseline);

    for (i = 0; i <= major_ticks; i++) {
      gdouble x = 0.5 + usable * i / major_ticks;

      cairo_move_to (cr, x, baseline);
      cairo_line_to (cr, x, baseline + 8.0);

      if (i == major_ticks)
        continue;

      for (j = 1; j <= minor_ticks; j++) {
        gdouble mx = x + usable / major_ticks * j / (minor_ticks + 1);

        cairo_move_to (cr, mx, baseline);
        cairo_line_to (cr, mx, baseline + 4.0);
      }
    }
    cairo_stroke (cr);

    marker_frac = ggobi_gtk_ruler_fraction (state, state->position);
    cairo_move_to (cr, marker_frac * usable - 5.0, height - 0.5);
    cairo_line_to (cr, marker_frac * usable + 5.0, height - 0.5);
    cairo_line_to (cr, marker_frac * usable + 0.5, height - 6.5);
    cairo_close_path (cr);
    cairo_fill (cr);
  } else {
    gdouble baseline = width - 4.5;
    gdouble usable = MAX (1, height - 1);

    cairo_move_to (cr, baseline, 0.5);
    cairo_line_to (cr, baseline, height - 0.5);

    for (i = 0; i <= major_ticks; i++) {
      gdouble y = 0.5 + usable * i / major_ticks;

      cairo_move_to (cr, baseline, y);
      cairo_line_to (cr, baseline - 8.0, y);

      if (i == major_ticks)
        continue;

      for (j = 1; j <= minor_ticks; j++) {
        gdouble my = y + usable / major_ticks * j / (minor_ticks + 1);

        cairo_move_to (cr, baseline, my);
        cairo_line_to (cr, baseline - 4.0, my);
      }
    }
    cairo_stroke (cr);

    marker_frac = ggobi_gtk_ruler_fraction (state, state->position);
    cairo_move_to (cr, 0.5, marker_frac * usable - 5.0);
    cairo_line_to (cr, 0.5, marker_frac * usable + 5.0);
    cairo_line_to (cr, 6.5, marker_frac * usable + 0.5);
    cairo_close_path (cr);
    cairo_fill (cr);
  }

  return FALSE;
}

static GtkWidget *
ggobi_gtk_ruler_new (GtkOrientation orientation)
{
  GtkWidget *widget = gtk_drawing_area_new ();

  gtk_widget_add_events (widget,
                         GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK);
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    gtk_widget_set_size_request (widget, -1, 24);
  else
    gtk_widget_set_size_request (widget, 24, -1);

  ggobi_gtk_ruler_state_ensure (widget)->orientation = orientation;
  g_signal_connect (G_OBJECT (widget), "draw",
                    G_CALLBACK (ggobi_gtk_ruler_draw_cb), NULL);
  return widget;
}

static gboolean
ggobi_point_in_polygon (const GdkPoint *points, gint npoints, gint x, gint y)
{
  gboolean inside = FALSE;
  gint i;
  gint j;

  for (i = 0, j = npoints - 1; i < npoints; j = i++) {
    gboolean intersects;

    intersects =
      ((points[i].y > y) != (points[j].y > y)) &&
      (x < (points[j].x - points[i].x) * (y - points[i].y) /
       (gdouble) (points[j].y - points[i].y) + points[i].x);
    if (intersects)
      inside = !inside;
  }

  return inside;
}

static void
ggobi_gdk_color_to_rgba (const GdkColor *color, GdkRGBA *rgba)
{
  rgba->red = color->red / 65535.0;
  rgba->green = color->green / 65535.0;
  rgba->blue = color->blue / 65535.0;
  rgba->alpha = 1.0;
}

static cairo_t *
ggobi_draw_target_begin (GGobiDrawTarget *target)
{
  if (target == NULL)
    return NULL;

  if (target->is_window)
    return gdk_cairo_create (target->window);

  if (target->surface)
    return cairo_create (target->surface);

  return NULL;
}

static void
ggobi_gc_apply (cairo_t *cr, const GGobiDrawStyle *style)
{
  GdkRGBA rgba;
  cairo_line_cap_t cap = CAIRO_LINE_CAP_ROUND;
  cairo_line_join_t join = CAIRO_LINE_JOIN_ROUND;

  if (cr == NULL || style == NULL)
    return;

  ggobi_gdk_color_to_rgba (&style->foreground, &rgba);
  gdk_cairo_set_source_rgba (cr, &rgba);
  cairo_set_line_width (cr, MAX (1, style->line_width));

  switch (style->cap_style) {
  case GDK_CAP_BUTT:
    cap = CAIRO_LINE_CAP_BUTT;
    break;
  case GDK_CAP_PROJECTING:
    cap = CAIRO_LINE_CAP_SQUARE;
    break;
  case GDK_CAP_NOT_LAST:
  case GDK_CAP_ROUND:
  default:
    cap = CAIRO_LINE_CAP_ROUND;
    break;
  }
  cairo_set_line_cap (cr, cap);

  switch (style->join_style) {
  case GDK_JOIN_MITER:
    join = CAIRO_LINE_JOIN_MITER;
    break;
  case GDK_JOIN_BEVEL:
    join = CAIRO_LINE_JOIN_BEVEL;
    break;
  case GDK_JOIN_ROUND:
  default:
    join = CAIRO_LINE_JOIN_ROUND;
    break;
  }
  cairo_set_line_join (cr, join);

  if (style->line_style == GDK_LINE_ON_OFF_DASH && style->ndashes > 0) {
    double dashes[8];
    gint i;

    for (i = 0; i < style->ndashes; i++)
      dashes[i] = MAX (1, (gint) style->dashes[i]);
    cairo_set_dash (cr, dashes, style->ndashes, style->dashes_offset);
  } else {
    cairo_set_dash (cr, NULL, 0, 0.0);
  }
}

GtkTooltips *
ggobi_gtk_tooltips_new (void)
{
  GtkTooltips *tips = g_new0 (GtkTooltips, 1);

  tips->enabled = TRUE;
  return tips;
}

void
ggobi_gtk_tooltips_set_tip (GtkWidget *widget, const gchar *tip_text)
{
  gtk_widget_set_tooltip_text (widget, tip_text);
}

void
ggobi_gtk_tooltips_enable (GtkTooltips *tips)
{
  if (tips != NULL)
    tips->enabled = TRUE;
}

void
ggobi_gtk_tooltips_disable (GtkTooltips *tips)
{
  if (tips != NULL)
    tips->enabled = FALSE;
}

cairo_t *
ggobi_draw_target_cairo_create (gpointer target)
{
  if (target == NULL)
    return NULL;

  if (GDK_IS_WINDOW (target))
    return gdk_cairo_create (GDK_WINDOW (target));

  return ggobi_draw_target_begin ((GGobiDrawTarget *) target);
}

cairo_t *
ggobi_gdk_cairo_create (gpointer target)
{
  return ggobi_draw_target_cairo_create (target);
}

void
ggobi_cairo_apply_gc (cairo_t *cr, GdkGC *gc)
{
  ggobi_gc_apply (cr, gc);
}

void
ggobi_draw_style_init (GGobiDrawStyle *style)
{
  if (style == NULL)
    return;

  memset (style, 0, sizeof (*style));
  style->background.red = 65535;
  style->background.green = 65535;
  style->background.blue = 65535;
  style->line_width = 1;
  style->line_style = GDK_LINE_SOLID;
  style->cap_style = GDK_CAP_ROUND;
  style->join_style = GDK_JOIN_ROUND;
}

void
ggobi_draw_style_apply (cairo_t *cr, const GGobiDrawStyle *style)
{
  ggobi_gc_apply (cr, style);
}

void
ggobi_draw_style_set_foreground (GGobiDrawStyle *style, const GdkColor *color)
{
  if (style && color)
    style->foreground = *color;
}

void
ggobi_draw_style_set_background (GGobiDrawStyle *style, const GdkColor *color)
{
  if (style && color)
    style->background = *color;
}

void
ggobi_draw_style_set_line_attributes (GGobiDrawStyle *style, gint line_width,
                                      gint line_style, gint cap_style,
                                      gint join_style)
{
  if (style == NULL)
    return;

  style->line_width = line_width;
  style->line_style = line_style;
  style->cap_style = cap_style;
  style->join_style = join_style;
}

void
ggobi_draw_style_set_dashes (GGobiDrawStyle *style, gint dash_offset,
                             const gchar *dash_list, gint n)
{
  if (style == NULL)
    return;

  style->dashes_offset = dash_offset;
  style->ndashes = MIN (n, (gint) G_N_ELEMENTS (style->dashes));
  if (dash_list && style->ndashes > 0)
    memcpy (style->dashes, dash_list, style->ndashes);
}

void
ggobi_draw_style_get_values (const GGobiDrawStyle *style, GdkGCValues *values)
{
  if (style == NULL || values == NULL)
    return;

  memset (values, 0, sizeof (*values));
  values->foreground = style->foreground;
  values->background = style->background;
  values->line_width = style->line_width;
  values->line_style = style->line_style;
  values->cap_style = style->cap_style;
  values->join_style = style->join_style;
}

void
ggobi_cairo_set_source_gdk_color (cairo_t *cr, const GdkColor *color)
{
  GdkRGBA rgba;

  if (cr == NULL || color == NULL)
    return;

  ggobi_gdk_color_to_rgba (color, &rgba);
  gdk_cairo_set_source_rgba (cr, &rgba);
}

void
ggobi_cairo_draw_rectangle (cairo_t *cr, gboolean filled,
                            gint x, gint y, gint width, gint height)
{
  if (cr == NULL)
    return;

  cairo_rectangle (cr, x, y, width, height);
  if (filled)
    cairo_fill (cr);
  else
    cairo_stroke (cr);
}

void
ggobi_cairo_draw_line (cairo_t *cr, gint x1, gint y1, gint x2, gint y2)
{
  if (cr == NULL)
    return;

  cairo_move_to (cr, x1, y1);
  cairo_line_to (cr, x2, y2);
  cairo_stroke (cr);
}

void
ggobi_cairo_draw_arc (cairo_t *cr, gboolean filled,
                      gint x, gint y, gint width, gint height,
                      gint angle1, gint angle2)
{
  gdouble cx, cy, rx, ry;
  gdouble start, end;

  if (cr == NULL)
    return;

  cx = x + width / 2.0;
  cy = y + height / 2.0;
  rx = MAX (1, width) / 2.0;
  ry = MAX (1, height) / 2.0;
  start = angle1 / (64.0 * 180.0) * G_PI;
  end = (angle1 + angle2) / (64.0 * 180.0) * G_PI;

  cairo_save (cr);
  cairo_translate (cr, cx, cy);
  cairo_scale (cr, rx, ry);
  cairo_arc (cr, 0.0, 0.0, 1.0, start, end);
  cairo_restore (cr);

  if (filled)
    cairo_fill (cr);
  else
    cairo_stroke (cr);
}

void
ggobi_cairo_draw_polygon (cairo_t *cr, gboolean filled,
                          GdkPoint *points, gint npoints)
{
  gint i;

  if (cr == NULL || points == NULL || npoints <= 0)
    return;

  cairo_move_to (cr, points[0].x, points[0].y);
  for (i = 1; i < npoints; i++)
    cairo_line_to (cr, points[i].x, points[i].y);
  cairo_close_path (cr);

  if (filled)
    cairo_fill (cr);
  else
    cairo_stroke (cr);
}

void
ggobi_cairo_draw_lines (cairo_t *cr, GdkPoint *points, gint npoints)
{
  gint i;

  if (cr == NULL || points == NULL || npoints <= 0)
    return;

  cairo_move_to (cr, points[0].x, points[0].y);
  for (i = 1; i < npoints; i++)
    cairo_line_to (cr, points[i].x, points[i].y);
  cairo_stroke (cr);
}

void
ggobi_cairo_draw_segments (cairo_t *cr, GdkSegment *segs, gint nsegs)
{
  gint i;

  if (cr == NULL || segs == NULL || nsegs <= 0)
    return;

  for (i = 0; i < nsegs; i++) {
    cairo_move_to (cr, segs[i].x1, segs[i].y1);
    cairo_line_to (cr, segs[i].x2, segs[i].y2);
  }
  cairo_stroke (cr);
}

void
ggobi_cairo_draw_layout (cairo_t *cr, PangoLayout *layout, gint x, gint y)
{
  if (cr == NULL || layout == NULL)
    return;

  cairo_move_to (cr, x, y);
  pango_cairo_show_layout (cr, layout);
}

GdkGC *
gdk_gc_new (GdkWindow *window)
{
  GdkGC *gc = g_new0 (GdkGC, 1);

  (void) window;
  ggobi_draw_style_init (gc);
  return gc;
}

void
gdk_gc_destroy (GdkGC *gc)
{
  g_free (gc);
}

void
gdk_gc_set_foreground (GdkGC *gc, const GdkColor *color)
{
  ggobi_draw_style_set_foreground (gc, color);
}

void
gdk_gc_set_background (GdkGC *gc, const GdkColor *color)
{
  ggobi_draw_style_set_background (gc, color);
}

void
gdk_gc_set_line_attributes (GdkGC *gc, gint line_width, gint line_style,
                            gint cap_style, gint join_style)
{
  ggobi_draw_style_set_line_attributes (gc, line_width, line_style,
                                        cap_style, join_style);
}

void
gdk_gc_set_dashes (GdkGC *gc, gint dash_offset, const gchar *dash_list, gint n)
{
  ggobi_draw_style_set_dashes (gc, dash_offset, dash_list, n);
}

void
gdk_gc_get_values (GdkGC *gc, GdkGCValues *values)
{
  ggobi_draw_style_get_values (gc, values);
}

GdkColormap *
gdk_gc_get_colormap (GdkGC *gc)
{
  (void) gc;
  return NULL;
}

GGobiSurfaceBuffer *
ggobi_surface_buffer_new (gpointer parent, gint width, gint height, gint depth)
{
  GGobiSurfaceBuffer *buffer = g_new0 (GGobiSurfaceBuffer, 1);

  (void) parent;
  (void) depth;
  buffer->is_window = FALSE;
  buffer->width = width;
  buffer->height = height;
  buffer->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                MAX (1, width),
                                                MAX (1, height));
  return buffer;
}

void
ggobi_surface_buffer_free (GGobiSurfaceBuffer *buffer)
{
  if (buffer == NULL)
    return;

  if (buffer->surface)
    cairo_surface_destroy (buffer->surface);
  g_free (buffer);
}

void
ggobi_draw_target_get_size (GGobiDrawTarget *target, gint *width, gint *height)
{
  if (target == NULL)
    return;

  if (target->is_window && target->window) {
    if (width || height)
      gdk_window_get_geometry (target->window, NULL, NULL, width, height);
    return;
  }

  if (width)
    *width = target->width;
  if (height)
    *height = target->height;
}

GdkVisual *
ggobi_draw_target_get_visual (GGobiDrawTarget *target)
{
  if (target && target->is_window && target->window)
    return gdk_window_get_visual (target->window);
  return NULL;
}

GdkPixmap *
gdk_pixmap_new (gpointer parent, gint width, gint height, gint depth)
{
  return ggobi_surface_buffer_new (parent, width, height, depth);
}

void
gdk_pixmap_unref (GdkPixmap *pixmap)
{
  ggobi_surface_buffer_free (pixmap);
}

void
gdk_drawable_get_size (GdkDrawable *drawable, gint *width, gint *height)
{
  ggobi_draw_target_get_size (drawable, width, height);
}

GdkVisual *
gdk_drawable_get_visual (GdkDrawable *drawable)
{
  return ggobi_draw_target_get_visual (drawable);
}

void
gdk_draw_rectangle (GdkDrawable *drawable, GdkGC *gc, gboolean filled,
                    gint x, gint y, gint width, gint height)
{
  cairo_t *cr = ggobi_draw_target_begin (drawable);

  ggobi_gc_apply (cr, gc);
  cairo_rectangle (cr, x, y, width, height);
  if (filled)
    cairo_fill_preserve (cr);
  cairo_stroke (cr);
  cairo_destroy (cr);
}

void
gdk_draw_line (GdkDrawable *drawable, GdkGC *gc,
               gint x1, gint y1, gint x2, gint y2)
{
  cairo_t *cr = ggobi_draw_target_begin (drawable);

  ggobi_gc_apply (cr, gc);
  cairo_move_to (cr, x1, y1);
  cairo_line_to (cr, x2, y2);
  cairo_stroke (cr);
  cairo_destroy (cr);
}

void
gdk_draw_arc (GdkDrawable *drawable, GdkGC *gc, gboolean filled,
              gint x, gint y, gint width, gint height,
              gint angle1, gint angle2)
{
  cairo_t *cr = ggobi_draw_target_begin (drawable);

  ggobi_gc_apply (cr, gc);
  cairo_save (cr);
  cairo_translate (cr, x + width / 2.0, y + height / 2.0);
  cairo_scale (cr, width / 2.0, height / 2.0);
  cairo_arc_negative (cr, 0.0, 0.0, 1.0,
                      (360.0 - (angle1 / 64.0)) * M_PI / 180.0,
                      (360.0 - (angle2 / 64.0)) * M_PI / 180.0);
  if (filled)
    cairo_fill_preserve (cr);
  cairo_stroke (cr);
  cairo_restore (cr);
  cairo_destroy (cr);
}

void
gdk_draw_polygon (GdkDrawable *drawable, GdkGC *gc, gboolean filled,
                  GdkPoint *points, gint npoints)
{
  gint i;
  cairo_t *cr;

  if (points == NULL || npoints <= 0)
    return;

  cr = ggobi_draw_target_begin (drawable);
  ggobi_gc_apply (cr, gc);
  cairo_move_to (cr, points[0].x, points[0].y);
  for (i = 1; i < npoints; i++)
    cairo_line_to (cr, points[i].x, points[i].y);
  cairo_close_path (cr);
  if (filled)
    cairo_fill_preserve (cr);
  cairo_stroke (cr);
  cairo_destroy (cr);
}

void
gdk_draw_points (GdkDrawable *drawable, GdkGC *gc,
                 GdkPoint *points, gint npoints)
{
  gint i;
  cairo_t *cr = ggobi_draw_target_begin (drawable);

  ggobi_gc_apply (cr, gc);
  for (i = 0; i < npoints; i++)
    cairo_rectangle (cr, points[i].x, points[i].y, 1.0, 1.0);
  cairo_fill (cr);
  cairo_destroy (cr);
}

void
gdk_draw_segments (GdkDrawable *drawable, GdkGC *gc,
                   GdkSegment *segs, gint nsegs)
{
  gint i;
  cairo_t *cr = ggobi_draw_target_begin (drawable);

  ggobi_gc_apply (cr, gc);
  for (i = 0; i < nsegs; i++) {
    cairo_move_to (cr, segs[i].x1, segs[i].y1);
    cairo_line_to (cr, segs[i].x2, segs[i].y2);
  }
  cairo_stroke (cr);
  cairo_destroy (cr);
}

void
gdk_draw_lines (GdkDrawable *drawable, GdkGC *gc,
                GdkPoint *points, gint npoints)
{
  gint i;
  cairo_t *cr;

  if (points == NULL || npoints <= 0)
    return;

  cr = ggobi_draw_target_begin (drawable);
  ggobi_gc_apply (cr, gc);
  cairo_move_to (cr, points[0].x, points[0].y);
  for (i = 1; i < npoints; i++)
    cairo_line_to (cr, points[i].x, points[i].y);
  cairo_stroke (cr);
  cairo_destroy (cr);
}

void
gdk_draw_point (GdkDrawable *drawable, GdkGC *gc, gint x, gint y)
{
  cairo_t *cr = ggobi_draw_target_begin (drawable);

  ggobi_gc_apply (cr, gc);
  cairo_rectangle (cr, x, y, 1.0, 1.0);
  cairo_fill (cr);
  cairo_destroy (cr);
}

void
gdk_draw_layout (GdkDrawable *drawable, GdkGC *gc,
                 gint x, gint y, PangoLayout *layout)
{
  cairo_t *cr = ggobi_draw_target_begin (drawable);

  ggobi_gc_apply (cr, gc);
  cairo_move_to (cr, x, y);
  pango_cairo_show_layout (cr, layout);
  cairo_destroy (cr);
}

void
gdk_draw_string (GdkDrawable *drawable, gpointer font, GdkGC *gc,
                 gint x, gint y, const gchar *text)
{
  cairo_t *cr = ggobi_draw_target_begin (drawable);

  (void) font;
  ggobi_gc_apply (cr, gc);
  cairo_move_to (cr, x, y);
  cairo_show_text (cr, text);
  cairo_destroy (cr);
}

void
gdk_draw_pixmap (GdkDrawable *drawable, GdkGC *gc, GdkPixmap *src,
                 gint xsrc, gint ysrc, gint xdest, gint ydest,
                 gint width, gint height)
{
  cairo_t *cr = ggobi_draw_target_begin (drawable);

  if (cr == NULL)
    return;

  (void) gc;
  if (src && src->surface) {
    gint src_width;
    gint src_height;

    src_width = src->width;
    src_height = src->height;
    if (width < 0)
      width = src_width - xsrc;
    if (height < 0)
      height = src_height - ysrc;
    width = MIN (width, src_width - xsrc);
    height = MIN (height, src_height - ysrc);

    if (width > 0 && height > 0) {
      cairo_save (cr);
      cairo_rectangle (cr, xdest, ydest, width, height);
      cairo_clip (cr);
      cairo_set_source_surface (cr, src->surface, xdest - xsrc, ydest - ysrc);
      cairo_paint (cr);
      cairo_restore (cr);
    }
  }
  cairo_destroy (cr);
}

void
gdk_draw_drawable (GdkDrawable *drawable, GdkGC *gc, GdkDrawable *src,
                   gint xsrc, gint ysrc, gint xdest, gint ydest,
                   gint width, gint height)
{
  gdk_draw_pixmap (drawable, gc, (GdkPixmap *) src, xsrc, ysrc,
                   xdest, ydest, width, height);
}

GdkRegion *
gdk_region_polygon (const GdkPoint *points, gint npoints, gint fill_rule)
{
  GdkRegion *region;

  (void) fill_rule;
  if (points == NULL || npoints <= 0)
    return NULL;

  region = g_new0 (GdkRegion, 1);
  region->points = g_new (GdkPoint, npoints);
  memcpy (region->points, points, sizeof (GdkPoint) * npoints);
  region->npoints = npoints;
  return region;
}

gboolean
gdk_region_point_in (const GdkRegion *region, gint x, gint y)
{
  if (region == NULL || region->points == NULL || region->npoints < 3)
    return FALSE;

  return ggobi_point_in_polygon (region->points, region->npoints, x, y);
}

void
gdk_region_destroy (GdkRegion *region)
{
  if (region == NULL)
    return;

  g_free (region->points);
  g_free (region);
}

GGobiSurfaceBuffer *
ggobi_surface_buffer_from_xpm_data (gpointer drawable,
                                    GdkColormap *colormap,
                                    gpointer mask,
                                    gpointer transparent_color,
                                    gchar **data)
{
  GdkPixbuf *pixbuf;
  GGobiSurfaceBuffer *buffer;
  cairo_t *cr;

  (void) drawable;
  (void) colormap;
  (void) mask;
  (void) transparent_color;

  pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) data);
  buffer = ggobi_surface_buffer_new (NULL,
                                     gdk_pixbuf_get_width (pixbuf),
                                     gdk_pixbuf_get_height (pixbuf),
                                     -1);
  cr = cairo_create (buffer->surface);
  gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
  cairo_paint (cr);
  cairo_destroy (cr);
  g_object_unref (pixbuf);

  return buffer;
}

GtkWidget *
ggobi_gtk_image_new_from_surface_buffer (GGobiSurfaceBuffer *buffer,
                                         gpointer mask)
{
  (void) mask;
  return gtk_image_new_from_surface (buffer->surface);
}

GdkPixmap *
gdk_pixmap_colormap_create_from_xpm_d (gpointer drawable,
                                       GdkColormap *colormap,
                                       gpointer mask,
                                       gpointer transparent_color,
                                       gchar **data)
{
  return ggobi_surface_buffer_from_xpm_data (drawable, colormap, mask,
                                             transparent_color, data);
}

GtkWidget *
ggobi_gtk_image_new_from_pixmap (GdkPixmap *pixmap, gpointer mask)
{
  return ggobi_gtk_image_new_from_surface_buffer (pixmap, mask);
}

gboolean
ggobi_pointer_grab (GtkWidget *widget, GdkEvent *event, GdkEventMask event_mask)
{
  GdkWindow *window;
  GdkSeat *seat;

  (void) event_mask;

  if (widget == NULL)
    return FALSE;

  window = gtk_widget_get_window (widget);
  if (window == NULL)
    return FALSE;

  seat = gdk_display_get_default_seat (gtk_widget_get_display (widget));
  if (seat == NULL)
    return FALSE;

  return gdk_seat_grab (seat, window, GDK_SEAT_CAPABILITY_POINTER,
                        FALSE, NULL, event, NULL, NULL) == GDK_GRAB_SUCCESS;
}

void
ggobi_pointer_ungrab (GtkWidget *widget)
{
  GdkSeat *seat;

  if (widget == NULL)
    return;

  seat = gdk_display_get_default_seat (gtk_widget_get_display (widget));
  if (seat != NULL)
    gdk_seat_ungrab (seat);
}

GtkWidget *
ggobi_gtk_hruler_new (void)
{
  return ggobi_gtk_ruler_new (GTK_ORIENTATION_HORIZONTAL);
}

GtkWidget *
ggobi_gtk_vruler_new (void)
{
  return ggobi_gtk_ruler_new (GTK_ORIENTATION_VERTICAL);
}

void
ggobi_gtk_ruler_set_range (GtkWidget *widget, gdouble lower,
                           gdouble upper, gdouble position,
                           gdouble max_size)
{
  GGobiRulerState *state;

  if (widget == NULL)
    return;

  state = ggobi_gtk_ruler_state_ensure (widget);
  state->lower = lower;
  state->upper = upper;
  state->position = position;
  state->max_size = max_size;

  if (!isfinite (lower) || !isfinite (upper) || !isfinite (position))
    return;

  state->position = CLAMP (position, MIN (lower, upper), MAX (lower, upper));
  gtk_widget_queue_draw (widget);
}

void
ggobi_gtk_ruler_get_range (GtkWidget *widget, gdouble *lower,
                           gdouble *upper, gdouble *position,
                           gdouble *max_size)
{
  GGobiRulerState *state;

  if (widget == NULL)
    return;

  state = ggobi_gtk_ruler_state_ensure (widget);
  if (lower)
    *lower = state->lower;
  if (upper)
    *upper = state->upper;
  if (position)
    *position = state->position;
  if (max_size)
    *max_size = state->max_size;
}

void
ggobi_gtk_ruler_set_position (GtkWidget *widget, gdouble position)
{
  GGobiRulerState *state;

  if (widget == NULL)
    return;

  state = ggobi_gtk_ruler_state_ensure (widget);
  state->position = position;
  if (!isfinite (position) || !isfinite (state->lower) ||
      !isfinite (state->upper))
    return;
  gtk_widget_queue_draw (widget);
}

gboolean
ggobi_gtk_ruler_is_horizontal (GtkWidget *widget)
{
  if (widget == NULL)
    return TRUE;

  return ggobi_gtk_ruler_state_ensure (widget)->orientation ==
    GTK_ORIENTATION_HORIZONTAL;
}

GdkColormap *
gdk_colormap_get_system (void)
{
  return NULL;
}

gboolean
gdk_colormap_alloc_color (GdkColormap *colormap, GdkColor *color,
                          gboolean writeable, gboolean best_match)
{
  (void) colormap;
  (void) color;
  (void) writeable;
  (void) best_match;
  return TRUE;
}

void
gdk_colormap_alloc_colors (GdkColormap *colormap, GdkColor *colors,
                           gint ncolors, gboolean writeable,
                           gboolean best_match, gboolean *success)
{
  gint i;

  (void) colormap;
  (void) colors;
  (void) writeable;
  (void) best_match;
  if (success) {
    for (i = 0; i < ncolors; i++)
      success[i] = TRUE;
  }
}

void
gdk_colormap_query_color (GdkColormap *colormap, gulong pixel, GdkColor *result)
{
  (void) colormap;
  if (result)
    result->pixel = pixel;
}

gboolean
gdk_color_alloc (GdkColormap *colormap, GdkColor *color)
{
  (void) colormap;
  (void) color;
  return TRUE;
}

void
gdk_color_white (GdkColormap *colormap, GdkColor *color)
{
  (void) colormap;
  if (color) {
    color->red = 65535;
    color->green = 65535;
    color->blue = 65535;
  }
}

void
gdk_color_black (GdkColormap *colormap, GdkColor *color)
{
  (void) colormap;
  if (color) {
    color->red = 0;
    color->green = 0;
    color->blue = 0;
  }
}

GdkColormap *
gtk_widget_get_colormap (GtkWidget *widget)
{
  (void) widget;
  return NULL;
}

GList *
ggobi_gtk_table_children (GtkWidget *table)
{
  return gtk_container_get_children (GTK_CONTAINER (table));
}

void
ggobi_gtk_table_get_attachments (GtkWidget *table, GtkWidget *child,
                                 guint *left_attach, guint *right_attach,
                                 guint *top_attach, guint *bottom_attach)
{
  gtk_container_child_get (GTK_CONTAINER (table), child,
                           "left-attach", left_attach,
                           "right-attach", right_attach,
                           "top-attach", top_attach,
                           "bottom-attach", bottom_attach,
                           NULL);
}

void
ggobi_gtk_table_set_attachments (GtkWidget *table, GtkWidget *child,
                                 guint left_attach, guint right_attach,
                                 guint top_attach, guint bottom_attach)
{
  gtk_container_child_set (GTK_CONTAINER (table), child,
                           "left-attach", left_attach,
                           "right-attach", right_attach,
                           "top-attach", top_attach,
                           "bottom-attach", bottom_attach,
                           NULL);
}

#endif
