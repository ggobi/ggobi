/*-- color.c --*/
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

#include <stdlib.h>

#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"
#include "colorscheme.h"

void
colorscheme_init (colorschemed * scheme)
{
  gint i;
  gboolean writeable = false;

  if (!scheme || scheme->n <= 0) {
    g_printerr ("unable to init colorscheme: ncolors=%d\n", scheme->n);
    return;
  }

  scheme->rgb = (GdkColor *) g_realloc (scheme->rgb,
                                        scheme->n * sizeof (cairo_pattern_t));

  /* this may have already been done; is there harm in doing it again? */
  for (i = 0; i < scheme->n; i++) {
    scheme->rgb[i] = cairo_pattern_create_rgb(scheme->data[i][0],
                                              scheme->data[i][1],
                                              scheme->data[i][2]);
  }

/*
 * background color
*/
  scheme->rgb_bg = cairo_pattern_create_rgb(scheme->bg[0],
                                            scheme->bg[1],
                                            scheme->bg[2]);

/*
 * color for showing hidden points and edges to preserve context
 * in a few situations:  when doing "un-hide" brushing and when showing
 * neighbors in the GraphAction plugin
*/
  {
    gfloat red, green, blue;
    if (scheme->bg[0] + scheme->bg[1] + scheme->bg[2] > 1.5) {
      red = MAX (0.0, scheme->bg[0] - .3);
      green = MAX (0.0, scheme->bg[1] - .3);
      blue = MAX (0.0, scheme->bg[2] - .3);
    }
    else {
      red = MIN (1.0, scheme->bg[0] + .3);
      green = MIN (1.0, scheme->bg[1] + .3);
      blue = MIN (1.0, scheme->bg[2] + .3);
    }
    scheme->rgb_hidden = cairo_pattern_create_rgb(red, green, blue);
  }

/*
 * accent color:  that is, the color that's used for
 * axes and labels, and not for brushing.
*/
  scheme->rgb_accent = cairo_pattern_create_rgb(scheme->accent[0],
                                                scheme->accent[1],
                                                scheme->accent[2]);
}

/*-- If gg->colorSchemes == NULL, then provide one and make it active --*/
colorschemed *
default_scheme_init ()
{
  gint i, k;
  /*
   * This section may be useful for debugging in case of
   * difficulties reading the colorscheme xml file.
   */
#ifdef SPECTRUM7
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
#endif

  static gfloat data[][3] = {
    {1.0000, 1.0000, 0.2000},
    {0.8941, 0.1020, 0.1098},
    {0.2157, 0.4941, 0.7216},
    {0.3020, 0.6863, 0.2902},
    {1.0000, 0.4980, 0.0000},
    {0.6510, 0.3373, 0.1569},
    {0.9686, 0.5059, 0.7490},
    {0.4980, 0.4980, 0.4980},
    {0.5961, 0.3059, 0.6392}
  };
  static gchar *colorNames[] = {
    "Yellow", "Orange", "Blue", "Green", "Orange", "Brown",
    "Pink", "Gray", "Purple"
  };
  static gfloat bg[] = { 0.000, 0.000, 0.000 };
  static gfloat accent[] = { 1.000, 1.000, 1.000 };
  colorschemed *scheme = (colorschemed *) g_malloc (sizeof (colorschemed));

  scheme->name = g_strdup ("Set1 9");
  scheme->description =
    g_strdup
    ("From Cindy Brewer, one of the schemes in the ColorBrewer software");
  scheme->type = qualitative;
  scheme->system = rgb;
  scheme->n = sizeof (data) / sizeof (data[0]);
  scheme->rgb = NULL;
  scheme->criticalvalue = 0;  /*-- unused --*/

  /*-- fill in the color names --*/
  scheme->colorNames = g_array_new (false, false, sizeof (gchar *));
  for (i = 0; i < scheme->n; i++)
    g_array_append_val (scheme->colorNames, colorNames[i]);

  /*-- allocate and populate data, bg, and accent --*/
  scheme->data = (gfloat **) g_malloc (scheme->n * sizeof (gfloat *));
  for (k = 0; k < scheme->n; k++) {
    scheme->data[k] = (gfloat *) g_malloc (3 * sizeof (gfloat));
    for (i = 0; i < 3; i++)
      scheme->data[k][i] = data[k][i];
  }

  scheme->bg = (gfloat *) g_malloc (3 * sizeof (gfloat));
  for (i = 0; i < 3; i++)
    scheme->bg[i] = bg[i];

  scheme->accent = (gfloat *) g_malloc (3 * sizeof (gfloat));
  for (i = 0; i < 3; i++)
    scheme->accent[i] = accent[i];

  colorscheme_init (scheme);

  return scheme;
}


/* API */
static guint m[MAXNCOLORS][3];
guint **
getColorTable (ggobid * gg)
{
  gint k;
  colorschemed *scheme = gg->activeColorScheme;

  for (k = 0; k < MAXNCOLORS; k++) {
    cairo_pattern_get_rgb(scheme->rgb[k], m[k], m[k]+1, m[k]+2);
  }

  return (guint **) m;
}


/*-- initialize the tour manip colors and the shades of gray --*/
void
special_colors_init (ggobid * gg)
{
  gboolean writeable = false, best_match = true;

/*
 * colors that show up in the variable circle panel
*/
  gg->vcirc_manip_color = cairo_pattern_create_rgb(1.0, 0.0, 1.0);
  gg->vcirc_freeze_color = cairo_pattern_create_rgb(0.0, 1.0, 0.0);
  
  gg->darkgray = cairo_pattern_create_rgb(0.3, 0.3, 0.3);
  gg->mediumgray = cairo_pattern_create_rgb(0.5, 0.5, 0.5);
  gg->lightgray = cairo_pattern_create_rgb(0.7, 0.7, 0.7);
}

gushort  /*-- returns the maximum color id --*/
datad_colors_used_get (gint * ncolors_used, gushort * colors_used,
                       GGobiData * d, ggobid * gg)
{
  gboolean new_color;
  gint i, k, m, n;
  gushort colorid, maxcolorid = 0;
  gushort *tmp;

  if (d == NULL || d->nrows == 0)
    /**/ return -1;

  g_assert (d->color.nels == d->nrows);

  n = 0;  /*-- *ncolors_used --*/

  /*
   * Loop once through d->color[], collecting the colors currently
   * in use into the colors_used[] vector.
   */
  for (i = 0; i < d->nrows_in_plot; i++) {
    m = d->rows_in_plot.els[i];
    if (d->hidden_now.els[m]) {  /*-- if it's hidden, we don't care --*/
      new_color = false;
    }
    else {
      new_color = true;
      for (k = 0; k < n; k++) {
        if (colors_used[k] == d->color_now.els[m]) {
          new_color = false;
          break;
        }
      }
    }
    if (new_color) {
      colorid = d->color_now.els[m];
      colors_used[n] = colorid;
      maxcolorid = MAX (colorid, maxcolorid);
      (n)++;
    }
  }

  /* Reorder the values in colors_used so that they are along
   * the order used in the color scheme.  Hey, that's simply
   * a matter of putting them in numerical order.  ... reverse
   * numerical order works better.
  */

  qsort ((void *) colors_used, n, sizeof (gushort), scompare);
  tmp = (gushort *) g_malloc(n * sizeof(gushort));
  for (i=0; i<n; i++) 
    tmp[n-i-1] = colors_used[i];
  for (i=0; i<n; i++) 
    colors_used[i] = tmp[i];
  g_free(tmp);



  /*
   * Make sure that the current brushing color is
   * last in the list, so that it is drawn on top of
   * the pile of points.
   */
  for (k = 0; k < (n - 1); k++) {
    if (colors_used[k] == gg->color_id) {
      colors_used[k] = colors_used[n - 1];
      colors_used[n - 1] = gg->color_id;
      break;
    }
  }

  /* insurance -- eg if using mono drawing on a color screen */
  if (n == 0) {
    n = 1;
    colors_used[0] = d->color_now.els[0];
  }

  *ncolors_used = n;
  return (maxcolorid);
}
