/* colorscheme.h */
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

#ifndef GGOBI_COLOR_SCHEME_H
#define GGOBI_COLOR_SCHEME_H

#include <gdk/gdk.h>

typedef enum { diverging, sequential, spectral, qualitative,
  UNKNOWN_COLOR_TYPE
} colorscaletype;
typedef enum { rgb, hsv, cmy, cmyk, UNKNOWN_COLOR_SYSTEM } colorsystem;

typedef struct {
  gchar *name;
  gchar *description;
  colorscaletype type;
  colorsystem system;   /*-- system used in the xml description --*/
  gint criticalvalue;   /*-- if diverging, where's the center? --*/

  gint n;               /*-- n <= MAXNCOLORS --*/
  gdouble **data;        /*-- the data in the colortable, in its
                             original system and dimensions --*/
  GdkColor *rgb;         /*-- the data converted to rgb, of length n --*/
  GArray *colorNames;   /*-- in case we have them --*/

  gdouble *bg;           /*-- high-contrast background color, rgb --*/
  GdkColor rgb_bg;     /*-- high-contrast background color, rgb --*/

  GdkColor rgb_hidden;  /*-- for hidden points and edges, close to bg --*/

  gdouble *accent;       /*-- high-contrast accent color, rgb --*/
  GdkColor rgb_accent; /*-- high-contrast accent color, rgb --*/
} colorschemed;

colorschemed *findColorSchemeByName(GList * schemes, const gchar * name);

gint getColor(xmlNodePtr node, xmlDocPtr doc, gdouble ** original,
              GdkColor * col);

colorschemed *read_colorscheme(char *fileName, GList **);

void colorscheme_init(colorschemed * scheme);
#endif
