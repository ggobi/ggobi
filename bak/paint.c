/* paint.c */

#include <stdio.h>

#include <gtk/gtk.h>
#include "vars.h"

/*static gulong npts_under_brush = 0;*/

void
alloc_glyph_ids()
{
  if (xg.glyph_ids != NULL)  g_free(xg.glyph_ids);
  if (xg.glyph_now != NULL)  g_free(xg.glyph_now);
  if (xg.glyph_prev != NULL) g_free(xg.glyph_prev);

  xg.glyph_ids = (glyphv *)  g_malloc(xg.nrows * sizeof(glyphv));
  xg.glyph_now = (glyphv *)  g_malloc(xg.nrows * sizeof(glyphv));
  xg.glyph_prev = (glyphv *) g_malloc(xg.nrows * sizeof(glyphv));
}

void
init_glyph_ids()
{
  gint j;

  for (j=0; j<xg.nrows; j++) {
    xg.glyph_ids[j].type = xg.glyph_now[j].type =
      xg.glyph_prev[j].type = xg.glyph_0.type;
    xg.glyph_ids[j].size = xg.glyph_now[j].size =
      xg.glyph_prev[j].size = xg.glyph_0.size;
  }
}

void
find_glyph_type_and_size(gint gid, glyphv *glyph)
{
  glyph->type = ( (gid-1) / (gint) NGLYPHSIZES ) + 1 ;
  glyph->size = ( (gid-1) % (gint) NGLYPHSIZES ) + 1 ;
}

void
alloc_color_ids()
{
  if (xg.color_ids != NULL)  g_free(xg.color_ids);
  if (xg.color_now != NULL)  g_free(xg.color_now);
  if (xg.color_prev != NULL) g_free(xg.color_prev);

  xg.color_ids = (gushort *)  g_malloc0(xg.nrows * sizeof(gulong));
  xg.color_now = (gushort *)  g_malloc0(xg.nrows * sizeof(gulong));
  xg.color_prev = (gushort *) g_malloc0(xg.nrows * sizeof(gulong));
}

void
init_color_ids()
{
  gint j;

  xg.color_id = xg.color_0;
  for (j=0; j<xg.nrows; j++) {
    xg.color_ids[j] = xg.color_now[j] =
      xg.color_prev[j] = xg.color_0;
  }
}
