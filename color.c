/*-- color.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#include <stdlib.h>

#ifdef USE_STRINGS_H		    
#include <strings.h>
#endif

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

static const gfloat default_rgb[NCOLORS][3] = {
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

static const gfloat bg_rgb  []    = {0, 0, 0};  /* -> bg_color */
static const gfloat accent_rgb [] = {1, 1, 1};  /* -> fg_color */

/* API */
static guint m[NCOLORS][3];
guint **
getColorTable (ggobid *gg)
{
  gint k;

  for (k=0; k<NCOLORS; k++) {
    m[k][0] = gg->default_color_table[k].red;
    m[k][1] = gg->default_color_table[k].green;
    m[k][2] = gg->default_color_table[k].blue;
  }

  return (guint **) m;
}


/*-- initialize the color table and a bunch of other colors as well --*/
void
color_table_init (ggobid *gg) {
  gint i;
  GdkColormap *cmap = gdk_colormap_get_system ();
  gboolean writeable = false, best_match = true, success[NCOLORS];

  gg->default_color_table = (GdkColor *) g_malloc (NCOLORS * sizeof (GdkColor));

  for (i=0; i<NCOLORS; i++) {
    gg->default_color_table[i].red   = (guint16) (default_rgb[i][0]*65535.0);
    gg->default_color_table[i].green = (guint16) (default_rgb[i][1]*65535.0);
    gg->default_color_table[i].blue  = (guint16) (default_rgb[i][2]*65535.0);
  }

  gdk_colormap_alloc_colors (cmap, gg->default_color_table, NCOLORS,
    writeable, best_match, success);

  gg->ncolors = NCOLORS;  /* add foreground and background once I know them */

  /*
   * Success[i] should always be true, since I'm allowing best_match,
   * but this can't hurt.
  */
  for (i=0; i<NCOLORS; i++) {
    if (!success[i]) {
      gg->default_color_table[i].red =   (guint16) 65535;
      gg->default_color_table[i].green = (guint16) 65535;
      gg->default_color_table[i].blue =  (guint16) 65535;
      if (gdk_colormap_alloc_color (cmap, &gg->default_color_table[i],
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
  gg->bg_color.red   = (guint16) (bg_rgb[0]*65535.0);
  gg->bg_color.green = (guint16) (bg_rgb[1]*65535.0);
  gg->bg_color.blue  = (guint16) (bg_rgb[2]*65535.0);
  if (!gdk_colormap_alloc_color(cmap, &gg->bg_color, writeable, best_match))
    g_printerr ("failure allocating background color\n");

/*
 * accent color:  that is, the color that's used for
 * axes and labels, and not for brushing.
*/
  gg->accent_color.red   = (guint16) (accent_rgb[0]*65535.0);
  gg->accent_color.green = (guint16) (accent_rgb[1]*65535.0);
  gg->accent_color.blue  = (guint16) (accent_rgb[2]*65535.0);
  if (!gdk_colormap_alloc_color(cmap, &gg->accent_color, writeable, best_match))
    g_printerr ("failure allocating accent color\n");

/*
 * colors that show up in the variable circle panel
*/
  gg->vcirc_manip_color.red   = (guint16) 65535;
  gg->vcirc_manip_color.green = (guint16) 0;
  gg->vcirc_manip_color.blue  = (guint16) 65535;
  if (!gdk_colormap_alloc_color (cmap, &gg->vcirc_manip_color,
    writeable, best_match))
      g_printerr ("trouble allocating vcirc_manip_color\n");

  gg->vcirc_freeze_color.red   = (guint16) 0;
  gg->vcirc_freeze_color.green = (guint16) 64435;
  gg->vcirc_freeze_color.blue  = (guint16) 0;
  if (!gdk_colormap_alloc_color(cmap, &gg->vcirc_freeze_color,
    writeable, best_match))
      g_printerr ("trouble allocating vcirc_freeze_color\n");
}

void
init_plot_GC (GdkWindow *w, ggobid *gg) {
  GdkColor white, black;
  gdk_color_white (gdk_colormap_get_system (), &white);
  gdk_color_black (gdk_colormap_get_system (), &black);

  gg->plot_GC = gdk_gc_new (w);
  gdk_gc_set_foreground (gg->plot_GC, &white);
  gdk_gc_set_background (gg->plot_GC, &black);
  /* line_width, GdkLineStyle, GdkCapStyle, GdkJoinStyle */
  gdk_gc_set_line_attributes (gg->plot_GC,
    0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
}

void
init_var_GCs (GtkWidget *w, ggobid *gg) {
  GdkWindow *window = w->window;
  GtkStyle *style = gtk_style_copy (gtk_widget_get_style (w));
  GdkColor white, black, bg;

  gdk_color_white (gdk_colormap_get_system (), &white);
  gdk_color_black (gdk_colormap_get_system (), &black);

/*
 * the unselected variable GCs: thin lines
*/
  gg->unselvarbg_GC = gdk_gc_new (window);
  bg = style->bg[GTK_STATE_NORMAL];
  gdk_gc_set_foreground (gg->unselvarbg_GC, &bg);

  gg->unselvarfg_GC = gdk_gc_new (window);
  gdk_gc_set_line_attributes (gg->unselvarfg_GC,
    0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  gdk_gc_set_foreground (gg->unselvarfg_GC, &black);


/*
 * the selected variable GC: thick lines
*/
  gg->selvarfg_GC = gdk_gc_new (window);
  gdk_gc_set_line_attributes (gg->selvarfg_GC,
    2, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  gdk_gc_set_foreground (gg->selvarfg_GC, &black);

  gg->selvarbg_GC = gdk_gc_new (window);
  gdk_gc_set_foreground (gg->selvarbg_GC, &white);

/*
 * the manip variable GCs: thin purple lines
*/
  gg->manipvarfg_GC = gdk_gc_new (window);
  gdk_gc_set_line_attributes (gg->manipvarfg_GC,
    0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  gdk_gc_set_foreground (gg->manipvarfg_GC, &gg->vcirc_manip_color);

}
