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
#include "colorscheme.h"

static const gfloat default_rgb[MAXNCOLORS][3] = {
  {0.73, 0.33, 0.83},
  {0.51, 0.44, 1.00},
  {0.00, 0.75, 1.00},
  {0.00, 0.98, 0.60},
  {0.60, 0.80, 0.20},
  {1.00, 1.00, 0.00},
  {1.00, 0.84, 0.00},
  {1.00, 0.55, 0.00},
  {1.00, 0.27, 0.00},
  {1.00, 0.08, 0.58}
};

static const gfloat bg_rgb  []    = {0, 0, 0};  /* -> bg_color */
static const gfloat accent_rgb [] = {1, 1, 1};  /* -> fg_color */

/* API */
static guint m[MAXNCOLORS][3];
guint **
getColorTable (ggobid *gg)
{
  gint k;

  for (k=0; k<MAXNCOLORS; k++) {
    m[k][0] = gg->color_table[k].red;
    m[k][1] = gg->color_table[k].green;
    m[k][2] = gg->color_table[k].blue;
  }

  return (guint **) m;
}

void
color_table_init_from_default (GdkColor *color_table,
  GdkColor *bg_color, GdkColor *accent_color)
{
  gint i, ncolors = MAXNCOLORS;
  gboolean writeable = false, best_match = true, success[MAXNCOLORS];

  for (i=0; i<ncolors; i++) {
    color_table[i].red   = (guint16) (default_rgb[i][0]*65535.0);
    color_table[i].green = (guint16) (default_rgb[i][1]*65535.0);
    color_table[i].blue  = (guint16) (default_rgb[i][2]*65535.0);
  }

  gdk_colormap_alloc_colors (gdk_colormap_get_system (), color_table, ncolors,
    writeable, best_match, success);

  /*
   * Success[i] should always be true, since I'm allowing best_match,
   * but this can't hurt.
  */
  for (i=0; i<ncolors; i++) {
    if (!success[i]) {
      color_table[i].red =   (guint16) 65535;
      color_table[i].green = (guint16) 65535;
      color_table[i].blue =  (guint16) 65535;
      if (gdk_colormap_alloc_color (gdk_colormap_get_system (),
        &color_table[i], writeable, best_match) == false)
      {
        g_printerr("Unable to allocate colors, not even white!\n");
        exit(0);
      }
    }
  }

/*
 * background color
*/
  bg_color->red   = (guint16) (bg_rgb[0]*65535.0);
  bg_color->green = (guint16) (bg_rgb[1]*65535.0);
  bg_color->blue  = (guint16) (bg_rgb[2]*65535.0);
  if (!gdk_colormap_alloc_color (gdk_colormap_get_system (),
    bg_color, writeable, best_match))
      g_printerr ("failure allocating background color\n");

/*
 * accent color:  that is, the color that's used for
 * axes and labels, and not for brushing.
*/
  accent_color->red   = (guint16) (accent_rgb[0]*65535.0);
  accent_color->green = (guint16) (accent_rgb[1]*65535.0);
  accent_color->blue  = (guint16) (accent_rgb[2]*65535.0);
  if (!gdk_colormap_alloc_color(gdk_colormap_get_system (),
    accent_color, writeable, best_match))
      g_printerr ("failure allocating accent color\n");
}

/*-- initialize the color table and a bunch of other colors as well --*/
void
color_table_init (ggobid *gg) {
  gboolean writeable = false, best_match = true;

  gg->color_table = (GdkColor *) g_malloc (MAXNCOLORS * sizeof (GdkColor));
  gg->ncolors = MAXNCOLORS;
  color_table_init_from_default (gg->color_table,
    &gg->bg_color, &gg->accent_color);

/*
 * colors that show up in the variable circle panel
*/
  gg->vcirc_manip_color.red   = (guint16) 65535;
  gg->vcirc_manip_color.green = (guint16) 0;
  gg->vcirc_manip_color.blue  = (guint16) 65535;
  if (!gdk_colormap_alloc_color (gdk_colormap_get_system (),
    &gg->vcirc_manip_color, writeable, best_match))
      g_printerr ("trouble allocating vcirc_manip_color\n");

  gg->vcirc_freeze_color.red   = (guint16) 0;
  gg->vcirc_freeze_color.green = (guint16) 64435;
  gg->vcirc_freeze_color.blue  = (guint16) 0;
  if (!gdk_colormap_alloc_color(gdk_colormap_get_system (),
    &gg->vcirc_freeze_color, writeable, best_match))
      g_printerr ("trouble allocating vcirc_freeze_color\n");
}

void
init_plot_GC (GdkWindow *w, ggobid *gg) {
  GdkColor white, black, *bblack, *wwhite;

#ifdef USE_XML
  if(!sessionOptions->info || !sessionOptions->info->fgColor) {
     gdk_color_white (gdk_colormap_get_system (), &white);
     wwhite = &white;
  } else {
    wwhite = sessionOptions->info->fgColor;
    gg->accent_color = *wwhite;
  }
#else
   gdk_color_white (gdk_colormap_get_system (), &white);
   wwhite = &white;
#endif

#ifdef USE_XML
  if(!sessionOptions->info || !sessionOptions->info->bgColor) {
    gdk_color_black (gdk_colormap_get_system (), &black);
    bblack = &black;
  } else {
    bblack = sessionOptions->info->bgColor;
  }
#else
   gdk_color_black (gdk_colormap_get_system (), &black);
   bwhite = &black;
#endif

  gg->plot_GC = gdk_gc_new (w);
  gdk_gc_set_foreground (gg->plot_GC, wwhite);
  gdk_gc_set_background (gg->plot_GC, bblack);
  /* line_width, GdkLineStyle, GdkCapStyle, GdkJoinStyle */
  gdk_gc_set_line_attributes (gg->plot_GC,
    0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
}

void
init_var_GCs (GtkWidget *w, ggobid *gg) {
  GdkWindow *window = w->window;
  GtkStyle *style = gtk_style_copy (gtk_widget_get_style (w));
  GdkColor white, black, bg,  *bblack;

  gdk_color_white (gdk_colormap_get_system (), &white);
  gdk_color_black (gdk_colormap_get_system (), &black);

  if(!sessionOptions->info->bgColor) {
    gdk_color_black (gdk_colormap_get_system (), &black);
    bblack = &black;
  } else
    bblack = sessionOptions->info->bgColor;
/*
 * the unselected variable GCs: thin lines
*/
  gg->unselvarbg_GC = gdk_gc_new (window);
  bg = style->bg[GTK_STATE_NORMAL];
  gdk_gc_set_foreground (gg->unselvarbg_GC, &bg);

  gg->unselvarfg_GC = gdk_gc_new (window);
  gdk_gc_set_line_attributes (gg->unselvarfg_GC,
    0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  gdk_gc_set_foreground (gg->unselvarfg_GC, bblack);


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

/*-------------------------------------------------------------------*/
/*             Using colorschemed objects                            */
/*-------------------------------------------------------------------*/

void
color_table_init_from_scheme (colorschemed *scheme,
  GdkColor *color_table, GdkColor *bg_color, GdkColor *accent_color)
{
  gint i, ncolors = scheme->n;
  gboolean writeable = false, best_match = true, *success;

  success = (gboolean *) g_malloc (ncolors * sizeof (gboolean));

  for (i=0; i<ncolors; i++) {
    color_table[i].red   = scheme->rgb[i].red;
    color_table[i].green = scheme->rgb[i].green;
    color_table[i].blue  = scheme->rgb[i].blue;
  }

  gdk_colormap_alloc_colors (gdk_colormap_get_system (), color_table, ncolors,
    writeable, best_match, success);

  /*
   * Success[i] should always be true, since I'm allowing best_match,
   * but this can't hurt.
  */
  for (i=0; i<ncolors; i++) {
    if (!success[i]) {
      color_table[i].red =   (guint16) 65535;
      color_table[i].green = (guint16) 65535;
      color_table[i].blue =  (guint16) 65535;
      if (gdk_colormap_alloc_color (gdk_colormap_get_system (), &color_table[i],
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
  bg_color->red   = (guint16) (scheme->rgb_bg.red);
  bg_color->green = (guint16) (scheme->rgb_bg.green);
  bg_color->blue  = (guint16) (scheme->rgb_bg.blue);
  if (!gdk_colormap_alloc_color (gdk_colormap_get_system (),
    bg_color, writeable, best_match))
      g_printerr ("failure allocating background color\n");

/*
 * accent color:  that is, the color that's used for
 * axes and labels, and not for brushing.
*/
  accent_color->red   = (guint16) (scheme->rgb_accent.red);
  accent_color->green = (guint16) (scheme->rgb_accent.green);
  accent_color->blue  = (guint16) (scheme->rgb_accent.blue);
  if (!gdk_colormap_alloc_color (gdk_colormap_get_system (),
    accent_color, writeable, best_match))
      g_printerr ("failure allocating accent color\n");

  g_free (success);
}
