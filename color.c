#include <stdlib.h>
#include <strings.h>
#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

static gfloat default_rgb[NCOLORS][3] = {
  {1.00, 0.08, 0.58},
  {1.00, 0.27, 0.00},
  {1.00, 0.55, 0.00},
  {1.00, 0.84, 0.00},
  {1.00, 1.00, 0.00},
  {0.00, 0.75, 1.00},
  {0.51, 0.44, 1.00},
  {0.60, 0.80, 0.20},
  {0.00, 0.98, 0.60},
  {0.73, 0.33, 0.83}
};

static gfloat bg_rgb[]     = {0, 0, 0};  /* -> bg_color */
static gfloat accent_rgb[] = {1, 1, 1};  /* -> fg_color */

void
color_table_init () {
  gint i;
  GdkColormap *cmap = gdk_colormap_get_system ();
  gboolean writeable = false, best_match = true, success[NCOLORS];

  xg.default_color_table = (GdkColor *) g_malloc (NCOLORS * sizeof (GdkColor));

  for (i=0; i<NCOLORS; i++) {
    xg.default_color_table[i].red   = (guint16) (default_rgb[i][0]*65535.0);
    xg.default_color_table[i].green = (guint16) (default_rgb[i][1]*65535.0);
    xg.default_color_table[i].blue  = (guint16) (default_rgb[i][2]*65535.0);
  }

  gdk_colormap_alloc_colors (cmap, xg.default_color_table, NCOLORS,
    writeable, best_match, success);

  xg.ncolors = NCOLORS;  /* add foreground and background once I know them */

  /*
   * Success[i] should always be true, since I'm allowing best_match,
   * but this can't hurt.
  */
  for (i=0; i<NCOLORS; i++) {
    if (!success[i]) {
      xg.default_color_table[i].red =   (guint16) 65535;
      xg.default_color_table[i].green = (guint16) 65535;
      xg.default_color_table[i].blue =  (guint16) 65535;
      if (gdk_colormap_alloc_color (cmap, &xg.default_color_table[i],
        writeable, best_match) == false)
      {
        g_printerr("Unable to allocate colors, not even white!\n");
        exit(0);
      }
    }
  }

/*
 * background color
*/
  xg.bg_color.red   = (guint16) (bg_rgb[0]*65535.0);
  xg.bg_color.green = (guint16) (bg_rgb[1]*65535.0);
  xg.bg_color.blue  = (guint16) (bg_rgb[2]*65535.0);
  gdk_colormap_alloc_color(cmap, &xg.bg_color, writeable, best_match);

/*
 * accent color:  that is, the color that's used for
 * axes and labels, and not for brushing.
*/
  xg.accent_color.red   = (guint16) (accent_rgb[0]*65535.0);
  xg.accent_color.green = (guint16) (accent_rgb[1]*65535.0);
  xg.accent_color.blue  = (guint16) (accent_rgb[2]*65535.0);
  gdk_colormap_alloc_color(cmap, &xg.accent_color, writeable, best_match);
}

void
init_plot_GC (GdkWindow *w) {
  GdkColor white, black;
  gdk_color_white (gdk_colormap_get_system (), &white);
  gdk_color_black (gdk_colormap_get_system (), &black);

  plot_GC = gdk_gc_new (w);
  gdk_gc_set_foreground (plot_GC, &white);
  gdk_gc_set_background (plot_GC, &black);
  /* line_width, GdkLineStyle, GdkCapStyle, GdkJoinStyle */
  gdk_gc_set_line_attributes (plot_GC,
    0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
}

void
init_var_GCs (GtkWidget *w) {
  GdkWindow *window = w->window;
  GtkStyle *style = gtk_style_copy (gtk_widget_get_style (w));
  GdkColor white, black, bg;

  gdk_color_white (gdk_colormap_get_system (), &white);
  gdk_color_black (gdk_colormap_get_system (), &black);

/*
 * the unselected variable GCs: thin lines
*/
  unselvarbg_GC = gdk_gc_new (window);
  bg = style->bg[GTK_STATE_NORMAL];
  gdk_gc_set_foreground (unselvarbg_GC, &bg);

  unselvarfg_GC = gdk_gc_new (window);
  gdk_gc_set_line_attributes(unselvarfg_GC,
    0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  gdk_gc_set_foreground (unselvarfg_GC, &black);


/*
 * the selected variable GC: thick lines
*/
  selvarfg_GC = gdk_gc_new (window);
  gdk_gc_set_line_attributes(selvarfg_GC,
    2, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  gdk_gc_set_foreground (selvarfg_GC, &black);

  selvarbg_GC = gdk_gc_new (window);
  gdk_gc_set_foreground (selvarbg_GC, &white);
}
