/* brush_init.c */
/*
 * Allocation and initialization routines for brushing.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/* external variables */

/*-------------------------------------------------------------------------*/
/*                      glyphs                                             */
/*-------------------------------------------------------------------------*/

void
br_glyph_ids_free ()
{
  g_free (xg.glyph_ids);
  g_free (xg.glyph_now);
  g_free (xg.glyph_prev);
}

void
br_glyph_ids_alloc ()
{
  xg.glyph_ids = (glyphv *) g_realloc (xg.glyph_ids,
                                       xg.nrows * sizeof (glyphv));
  xg.glyph_now = (glyphv *) g_realloc (xg.glyph_now,
                                       xg.nrows * sizeof (glyphv));
  xg.glyph_prev = (glyphv *) g_realloc (xg.glyph_prev,
                                       xg.nrows * sizeof (glyphv));
}

void
br_glyph_ids_init ()
{
  gint j;

  for (j=0; j<xg.nrows; j++) {
    xg.glyph_ids[j].type = xg.glyph_now[j].type =
      xg.glyph_prev[j].type = xg.glyph_0.type;
    xg.glyph_ids[j].size = xg.glyph_now[j].size =
      xg.glyph_prev[j].size = xg.glyph_0.size;
  }
}

/*-------------------------------------------------------------------------*/
/*                       color                                             */
/*-------------------------------------------------------------------------*/

void
br_color_ids_free ()
{
  g_free (xg.color_ids);
  g_free (xg.color_now);
  g_free (xg.color_prev);
}

void
br_color_ids_alloc ()
{
  gint i;

  xg.color_ids = (gushort *)  g_realloc (xg.color_ids,
                                         xg.nrows * sizeof (gushort));
  xg.color_now = (gushort *)  g_realloc (xg.color_now,
                                         xg.nrows * sizeof (gushort));
  xg.color_prev = (gushort *) g_realloc (xg.color_prev,
                                         xg.nrows * sizeof (gushort));
  for (i=0; i<xg.nrows; i++)
    xg.color_ids[i] = xg.color_now[i] = xg.color_prev[i] = xg.color_0;
}

void
br_color_ids_init ()
{
  gint i;

  xg.color_id = xg.color_0;
  for (i=0; i<xg.nrows; i++)
    xg.color_ids[i] = xg.color_now[i] = xg.color_prev[i] = xg.color_0;
}

/*-------------------------------------------------------------------------*/
/*                             erasing                                     */
/*-------------------------------------------------------------------------*/

void
erase_alloc (void)
{
  if (xg.erased != NULL) g_free (xg.erased);
  if (xg.erased_now != NULL) g_free (xg.erased_now);
  if (xg.erased_prev != NULL) g_free (xg.erased_prev);

  xg.erased = (gboolean *) g_malloc (xg.nrows * sizeof (gboolean));
  xg.erased_now = (gboolean *) g_malloc (xg.nrows * sizeof (gboolean));
  xg.erased_prev = (gboolean *) g_malloc (xg.nrows * sizeof (gboolean));
}
void
erase_init (void)
{
  gint i;

  for (i=0; i<xg.nrows; i++)
    xg.erased[i] = xg.erased_now[i] = xg.erased_prev[i] = false;
}


/*-------------------------------------------------------------------------*/
/*                           line color                                    */
/*-------------------------------------------------------------------------*/


void
br_line_color_ids_alloc ()
{
  gint ns = xg.nsegments; 

  xg.line_color_ids = (gushort *) g_realloc ((gpointer) xg.line_color_ids,
    ns * sizeof (gushort));
  xg.line_color_now = (gushort *) g_realloc ((gpointer) xg.line_color_now,
    ns * sizeof (gushort));
  xg.line_color_prev = (gushort *) g_realloc ((gpointer) xg.line_color_prev,
    ns * sizeof (gushort));
  xg.xed_by_new_brush = (gushort *) g_realloc ((gpointer) xg.xed_by_new_brush,
    ns * sizeof (gushort));
}

void
br_line_color_ids_free ()
{
  g_free ((gpointer) xg.line_color_ids);
  g_free ((gpointer) xg.line_color_now);
  g_free ((gpointer) xg.line_color_prev);
  g_free ((gpointer) xg.xed_by_new_brush);
}

void
br_line_color_ids_init ()
{
  gint j;

  for (j=0; j<xg.nsegments; j++) {
    xg.line_color_ids[j] = xg.line_color_now[j] = xg.line_color_prev[j] =
      xg.color_0;
  }
}

/*-------------------------------------------------------------------------*/
/*                           the brush itself                              */
/*-------------------------------------------------------------------------*/

void
brush_pos_init ()
{
  brush_pos.x1 = brush_pos.y1 = 20;
  brush_pos.x2 = brush_pos.y2 = 40;
}

/*----------------------------------------------------------------------*/
/*                          general                                     */
/*----------------------------------------------------------------------*/

void
brush_alloc ()
/*
 * Dynamically allocate arrays.
*/
{
  guint nr = (guint) xg.nrows;
  gint i, iv, ih;
  gboolean initd = false;

  xg.br_nbins = BRUSH_NBINS;

  xg.included = (gboolean *) g_realloc (xg.included, nr * sizeof (gboolean));
  xg.under_new_brush = (gboolean *) g_realloc (xg.under_new_brush,
                                               nr * sizeof (gboolean));

  for (i=0; i<nr; i++) {
    xg.included[i] = true;
    xg.under_new_brush[i] = false;
  }

  /*
   * color_ids and glyph_ids and their kin were allocated when
   * the data was read in.
  */

  if (!initd) {
    /* binning the plot window; no need to realloc these */
    xg.br_binarray = (bin_struct **)
      g_malloc (xg.br_nbins * sizeof (bin_struct *));
    for (ih=0; ih<xg.br_nbins; ih++) {
      xg.br_binarray[ih] = (bin_struct *)
        g_malloc (xg.br_nbins * sizeof (bin_struct));

      for (iv=0; iv<xg.br_nbins; iv++) {
        xg.br_binarray[ih][iv].nels = 0;
        xg.br_binarray[ih][iv].nblocks = 1;
        xg.br_binarray[ih][iv].els = (gulong *)
          g_malloc (BINBLOCKSIZE * sizeof (gulong));
      }
    }
    initd = true;
  }
}

void
brush_free ()
/*
 * Dynamically free arrays.
*/
{
  int j,k;

  br_glyph_ids_free ();
  br_color_ids_free ();

  g_free ((gpointer) xg.under_new_brush);

  for (k=0; k<xg.br_nbins; k++) {
    for (j=0; j<xg.br_nbins; j++)
      g_free ((gpointer) xg.br_binarray[k][j].els);
    g_free ((gpointer) xg.br_binarray[k]);
  }
  g_free ((gpointer) xg.br_binarray);
}

void
brush_init ()
{
  static gboolean firsttime = true;

  xg.glyph_id.type = xg.glyph_0.type = FILLED_CIRCLE;
  xg.glyph_id.size = xg.glyph_0.size = 3;
  xg.color_id = xg.color_0 = 0;

  if (firsttime) {
    brush_pos_init ();

    /*
     * Used in binning the plot window
    */
    xg.br_nbins = BRUSH_NBINS;

    /*
     * These are initialized so that the first merge_brushbins()
     * call will behave reasonably.
    */
    xg.bin0.x = xg.bin1.x = BRUSH_NBINS;
    xg.bin0.y = xg.bin1.y = BRUSH_NBINS;

    brush_alloc ();

    firsttime = false;

  } else {
  }
}

void
brush_activate (gboolean state)
{
  if (state)
    assign_points_to_bins ();
}
