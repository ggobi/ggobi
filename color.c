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

void
colorscheme_init (colorschemed *scheme)
{
  gint i;
  gboolean writeable = false, best_match = true, *success;

  if (!scheme || scheme->n <= 0) {
    g_printerr ("unable to init colorscheme: ncolors=%d\n", scheme->n);
    return;
  }

  success = (gboolean *) g_malloc (scheme->n * sizeof (gboolean));

  scheme->rgb = (GdkColor *) g_realloc (scheme->rgb,
    scheme->n * sizeof(GdkColor));

  /* this may have already been done; is there harm in doing it again? */
  for (i=0; i<scheme->n; i++) {
    scheme->rgb[i].red =   (guint16) (scheme->data[i][0]*65535.0);
    scheme->rgb[i].green = (guint16) (scheme->data[i][1]*65535.0);
    scheme->rgb[i].blue =  (guint16) (scheme->data[i][2]*65535.0);
  }

  gdk_colormap_alloc_colors (gdk_colormap_get_system (),
    scheme->rgb, scheme->n, writeable, best_match, success);

  /*
   * Success[i] should always be true, since I'm allowing best_match,
   * but this can't hurt.
  */
  for (i=0; i<scheme->n; i++) {
    if (!success[i]) {
      scheme->rgb[i].red =   (guint16) 65535;
      scheme->rgb[i].green = (guint16) 65535;
      scheme->rgb[i].blue =  (guint16) 65535;
      if (gdk_colormap_alloc_color (gdk_colormap_get_system (),
        &scheme->rgb[i], writeable, best_match) == false)
      {
        g_printerr("Unable to allocate colors, not even white!\n");
        exit(0);
      }
    }
  }

/*
 * background color
*/
  scheme->rgb_bg.red   = (guint16) (scheme->bg[0]*65535.0);
  scheme->rgb_bg.green = (guint16) (scheme->bg[1]*65535.0);
  scheme->rgb_bg.blue  = (guint16) (scheme->bg[2]*65535.0);
  if (!gdk_colormap_alloc_color (gdk_colormap_get_system (),
    &scheme->rgb_bg, writeable, best_match))
      g_printerr ("failure allocating background color\n");

/*
 * accent color:  that is, the color that's used for
 * axes and labels, and not for brushing.
*/
  scheme->rgb_accent.red   = (guint16) (scheme->accent[0]*65535.0);
  scheme->rgb_accent.green = (guint16) (scheme->accent[1]*65535.0);
  scheme->rgb_accent.blue  = (guint16) (scheme->accent[2]*65535.0);
  if (!gdk_colormap_alloc_color (gdk_colormap_get_system (),
    &scheme->rgb_accent, writeable, best_match))
      g_printerr ("failure allocating background color\n");

  g_free (success);
}

/*-- If gg->colorSchemes == NULL, then provide one and make it active --*/
colorschemed *
default_scheme_init ()
{
  gint i, k;
  static gfloat data[7][3] = {
    {0.890, 0.196, 0.122},
    {1.000, 0.549, 0.000},
    {0.988, 0.839, 0.051},
    {0.988, 0.988, 0.000},
    {0.604, 0.824, 0.294},
    {0.235, 0.690, 0.616},
    {0.149, 0.482, 0.671}
  };
  static gchar *colorNames[7] = {
   "Red", "Orange", "Gold", "Yellow", "Green Yellow", "Teal", "Blue"
  };
  static gfloat bg[] = { 0.000, 0.000, 0.000 };
  static gfloat accent[] = { 1.000, 1.000, 1.000 };
  colorschemed *scheme = (colorschemed *) g_malloc (sizeof (colorschemed));

  scheme->name = g_strdup ("Spectrum 7");
  scheme->description = g_strdup ("From Cindy Brewer, a spectrum");
  scheme->type = spectral;
  scheme->system = rgb;
  scheme->n = 7;
  scheme->rgb = NULL;
  scheme->criticalvalue = 0;  /*-- unused --*/

  /*-- fill in the color names --*/
  scheme->colorNames = g_array_new (false, false, sizeof (gchar *));
  for (i=0; i<scheme->n; i++)
    g_array_append_val (scheme->colorNames, colorNames[i]);

  /*-- allocate and populate data, bg, and accent --*/
  scheme->data = (gfloat **) g_malloc (scheme->n * sizeof(gfloat *));
  for (k=0; k<scheme->n; k++) {
    scheme->data[k] = (gfloat *) g_malloc (3 * sizeof(gfloat));
    for (i=0; i<3; i++)
      scheme->data[k][i] = data[k][i];
  }

  scheme->bg = (gfloat *) g_malloc (3 * sizeof(gfloat));
  for (i=0; i<3; i++)
    scheme->bg[i] = bg[i];

  scheme->accent = (gfloat *) g_malloc (3 * sizeof(gfloat));
  for (i=0; i<3; i++)
    scheme->accent[i] = accent[i];

  colorscheme_init (scheme);

  return scheme;
}

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
  colorschemed *scheme = gg->activeColorScheme;

  for (k=0; k<MAXNCOLORS; k++) {
    m[k][0] = scheme->rgb[k].red;
    m[k][1] = scheme->rgb[k].green;
    m[k][2] = scheme->rgb[k].blue;
  }

  return (guint **) m;
}


/*-- initialize the tour manip colors --*/
void
tour_manip_colors_init (ggobid *gg) {
  gboolean writeable = false, best_match = true;

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
  colorschemed *scheme = gg->activeColorScheme;

  gg->plot_GC = gdk_gc_new (w);
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  gdk_gc_set_background (gg->plot_GC, &scheme->rgb_bg);
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

/*
#ifdef USE_XML
  if(!sessionOptions->info->bgColor) {
    gdk_color_black (gdk_colormap_get_system (), &black);
    bblack = &black;
  } else
    bblack = sessionOptions->info->bgColor;
#else
*/
   gdk_color_black (gdk_colormap_get_system (), &black);
   bblack = &black;
/*
#endif
*/

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

  g_free (style);
}

