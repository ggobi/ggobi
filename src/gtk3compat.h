#ifndef GGOBI_GTK3COMPAT_H
#define GGOBI_GTK3COMPAT_H

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms-compat.h>
#include <pango/pangocairo.h>
#include <cairo.h>

#if GTK_MAJOR_VERSION >= 3

typedef struct _GGobiDrawStyle {
  GdkColor foreground;
  GdkColor background;
  gint line_width;
  gint line_style;
  gint cap_style;
  gint join_style;
  gint dashes_offset;
  gchar dashes[8];
  gint ndashes;
} GGobiDrawStyle;

#ifndef GDK_LINE_SOLID
#define GDK_LINE_SOLID 0
#define GDK_LINE_ON_OFF_DASH 1
#define GDK_LINE_DOUBLE_DASH 2
#endif

#ifndef GDK_CAP_NOT_LAST
#define GDK_CAP_NOT_LAST 0
#define GDK_CAP_BUTT 1
#define GDK_CAP_ROUND 2
#define GDK_CAP_PROJECTING 3
#endif

#ifndef GDK_JOIN_MITER
#define GDK_JOIN_MITER 0
#define GDK_JOIN_ROUND 1
#define GDK_JOIN_BEVEL 2
#endif

typedef struct _GdkSegment {
  gint x1;
  gint y1;
  gint x2;
  gint y2;
} GdkSegment;

typedef struct _GGobiGdkRegion {
  GdkPoint *points;
  gint npoints;
} GdkRegion;

#ifndef GDK_WINDING_RULE
#define GDK_WINDING_RULE 1
#endif

typedef struct _GGobiDrawTarget {
  gboolean is_window;
  GdkWindow *window;
  cairo_surface_t *surface;
  gint width;
  gint height;
} GGobiDrawTarget;

typedef GGobiDrawTarget GGobiSurfaceBuffer;

cairo_t *ggobi_draw_target_cairo_create (gpointer target);
void ggobi_draw_style_init (GGobiDrawStyle *style);
void ggobi_draw_style_apply (cairo_t *cr, const GGobiDrawStyle *style);
void ggobi_draw_style_set_foreground (GGobiDrawStyle *style,
                                      const GdkColor *color);
void ggobi_draw_style_set_background (GGobiDrawStyle *style,
                                      const GdkColor *color);
void ggobi_draw_style_set_line_attributes (GGobiDrawStyle *style,
                                           gint line_width, gint line_style,
                                           gint cap_style, gint join_style);
void ggobi_draw_style_set_dashes (GGobiDrawStyle *style, gint dash_offset,
                                  const gchar *dash_list, gint n);
void ggobi_cairo_set_source_gdk_color (cairo_t *cr, const GdkColor *color);
void ggobi_cairo_draw_rectangle (cairo_t *cr, gboolean filled,
                                 gint x, gint y, gint width, gint height);
void ggobi_cairo_draw_line (cairo_t *cr, gint x1, gint y1, gint x2, gint y2);
void ggobi_cairo_draw_arc (cairo_t *cr, gboolean filled,
                           gint x, gint y, gint width, gint height,
                           gint angle1, gint angle2);
void ggobi_cairo_draw_polygon (cairo_t *cr, gboolean filled,
                               GdkPoint *points, gint npoints);
void ggobi_cairo_draw_lines (cairo_t *cr, GdkPoint *points, gint npoints);
void ggobi_cairo_draw_segments (cairo_t *cr, GdkSegment *segs, gint nsegs);
void ggobi_cairo_draw_layout (cairo_t *cr, PangoLayout *layout,
                              gint x, gint y);
void ggobi_draw_target_draw_rectangle (GGobiDrawTarget *target,
                                       GGobiDrawStyle *style,
                                       gboolean filled,
                                       gint x, gint y,
                                       gint width, gint height);
void ggobi_draw_target_draw_line (GGobiDrawTarget *target,
                                  GGobiDrawStyle *style,
                                  gint x1, gint y1, gint x2, gint y2);
void ggobi_draw_target_draw_arc (GGobiDrawTarget *target,
                                 GGobiDrawStyle *style,
                                 gboolean filled,
                                 gint x, gint y, gint width, gint height,
                                 gint angle1, gint angle2);
void ggobi_draw_target_draw_polygon (GGobiDrawTarget *target,
                                     GGobiDrawStyle *style,
                                     gboolean filled,
                                     GdkPoint *points, gint npoints);
void ggobi_draw_target_draw_points (GGobiDrawTarget *target,
                                    GGobiDrawStyle *style,
                                    GdkPoint *points, gint npoints);
void ggobi_draw_target_draw_segments (GGobiDrawTarget *target,
                                      GGobiDrawStyle *style,
                                      GdkSegment *segs, gint nsegs);
void ggobi_draw_target_draw_lines (GGobiDrawTarget *target,
                                   GGobiDrawStyle *style,
                                   GdkPoint *points, gint npoints);
void ggobi_draw_target_draw_point (GGobiDrawTarget *target,
                                   GGobiDrawStyle *style,
                                   gint x, gint y);
void ggobi_draw_target_draw_layout (GGobiDrawTarget *target,
                                    GGobiDrawStyle *style,
                                    gint x, gint y, PangoLayout *layout);
void ggobi_draw_target_draw_string (GGobiDrawTarget *target,
                                    gpointer font,
                                    GGobiDrawStyle *style,
                                    gint x, gint y, const gchar *text);
void ggobi_draw_target_blit_surface (GGobiDrawTarget *target,
                                     GGobiDrawStyle *style,
                                     GGobiSurfaceBuffer *src,
                                     gint xsrc, gint ysrc,
                                     gint xdest, gint ydest,
                                     gint width, gint height);

GGobiSurfaceBuffer *ggobi_surface_buffer_new (gpointer parent, gint width,
                                              gint height, gint depth);
void ggobi_surface_buffer_free (GGobiSurfaceBuffer *buffer);
void ggobi_draw_target_get_size (GGobiDrawTarget *target, gint *width,
                                 gint *height);
GdkVisual *ggobi_draw_target_get_visual (GGobiDrawTarget *target);
GGobiSurfaceBuffer *ggobi_surface_buffer_from_xpm_data (gpointer drawable,
                                                        gpointer mask,
                                                        gpointer transparent_color,
                                                        gchar **data);
GtkWidget *ggobi_gtk_image_new_from_surface_buffer (GGobiSurfaceBuffer *buffer,
                                                    gpointer mask);
GdkRegion *gdk_region_polygon (const GdkPoint *points, gint npoints,
                               gint fill_rule);
gboolean gdk_region_point_in (const GdkRegion *region, gint x, gint y);
void gdk_region_destroy (GdkRegion *region);
gboolean ggobi_pointer_grab (GtkWidget *widget, GdkEvent *event,
                             GdkEventMask event_mask);
void ggobi_pointer_ungrab (GtkWidget *widget);

GtkWidget *ggobi_gtk_hruler_new (void);
GtkWidget *ggobi_gtk_vruler_new (void);
void ggobi_gtk_ruler_set_range (GtkWidget *widget, gdouble lower,
                                gdouble upper, gdouble position,
                                gdouble max_size);
void ggobi_gtk_ruler_get_range (GtkWidget *widget, gdouble *lower,
                                gdouble *upper, gdouble *position,
                                gdouble *max_size);
void ggobi_gtk_ruler_set_position (GtkWidget *widget, gdouble position);
gboolean ggobi_gtk_ruler_is_horizontal (GtkWidget *widget);

GList *ggobi_gtk_table_children (GtkWidget *table);
void ggobi_gtk_table_get_attachments (GtkWidget *table, GtkWidget *child,
                                      guint *left_attach, guint *right_attach,
                                      guint *top_attach, guint *bottom_attach);
void ggobi_gtk_table_set_attachments (GtkWidget *table, GtkWidget *child,
                                      guint left_attach, guint right_attach,
                                      guint top_attach, guint bottom_attach);

#define gtk_hruler_new() ggobi_gtk_hruler_new ()
#define gtk_vruler_new() ggobi_gtk_vruler_new ()

#endif

#endif
