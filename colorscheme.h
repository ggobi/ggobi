#ifndef GGOBI_COLOR_SCHEME_H
#define GGOBI_COLOR_SCHEME_H

#include <gdk/gdk.h>

typedef enum {diverging, sequential, spectral, qualitative,
  UNKNOWN_COLOR_TYPE} colorscaletype;
typedef enum {rgb, hsv, cmy, cmyk, UNKNOWN_COLOR_SYSTEM} colorsystem;

typedef struct {
  gchar *name;
  gchar *description;
  colorscaletype type;
  colorsystem system;   /*-- system used in the xml description --*/
  gint criticalvalue;   /*-- if diverging, where's the center? --*/

  gfloat system_min, system_max;  /*-- min and max for system; relevant for rgb --*/

  gint n;               /*-- n <= MAXNCOLORS --*/
  gfloat **data;        /*-- the data in the colortable, in its original system and dimensions --*/
  GdkColor *rgb;         /*-- the data converted to rgb, nx3 --*/
  GArray *colorNames;   /*-- in case we have them --*/

  gfloat *bg;           /*-- high-contrast background color, rgb --*/
  GdkColor rgb_bg;     /*-- high-contrast background color, rgb --*/

  gfloat *accent;       /*-- high-contrast accent color, rgb --*/
  GdkColor rgb_accent; /*-- high-contrast accent color, rgb --*/
} colorschemed;

colorschemed *read_colorscheme(char *fileName, GList **);
colorschemed *findColorSchemeByName(GList *schemes, const gchar *name);

#ifdef USE_XML
gint getColor(xmlNodePtr node, xmlDocPtr doc, gfloat **original, GdkColor *col, gfloat min, gfloat max);
#endif

#endif
