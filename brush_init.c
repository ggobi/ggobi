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
br_glyph_ids_free (ggobid *gg)
{
  g_free (gg->glyph_ids);
  g_free (gg->glyph_now);
  g_free (gg->glyph_prev);
}

void
br_glyph_ids_alloc (ggobid *gg)
{
  gg->glyph_ids = (glyphv *) g_realloc (gg->glyph_ids,
                                       gg->nrows * sizeof (glyphv));
  gg->glyph_now = (glyphv *) g_realloc (gg->glyph_now,
                                       gg->nrows * sizeof (glyphv));
  gg->glyph_prev = (glyphv *) g_realloc (gg->glyph_prev,
                                       gg->nrows * sizeof (glyphv));
}

void
br_glyph_ids_init (ggobid *gg)
{
  gint j;

  for (j=0; j<gg->nrows; j++) {
    gg->glyph_ids[j].type = gg->glyph_now[j].type =
      gg->glyph_prev[j].type = gg->glyph_0.type;
    gg->glyph_ids[j].size = gg->glyph_now[j].size =
      gg->glyph_prev[j].size = gg->glyph_0.size;
  }
}

/*-------------------------------------------------------------------------*/
/*                       color                                             */
/*-------------------------------------------------------------------------*/

void
br_color_ids_free (ggobid *gg)
{
  g_free (gg->color_ids);
  g_free (gg->color_now);
  g_free (gg->color_prev);
}

void
br_color_ids_alloc (ggobid *gg)
{
  gint i;

  gg->color_ids = (gshort *)  g_realloc (gg->color_ids,
                                         gg->nrows * sizeof (gshort));
  gg->color_now = (gshort *)  g_realloc (gg->color_now,
                                         gg->nrows * sizeof (gshort));
  gg->color_prev = (gshort *) g_realloc (gg->color_prev,
                                         gg->nrows * sizeof (gshort));
  for (i=0; i<gg->nrows; i++)
    gg->color_ids[i] = gg->color_now[i] = gg->color_prev[i] = gg->color_0;
}

void
br_color_ids_init (ggobid *gg)
{
  gint i;

  gg->color_id = gg->color_0;
  for (i=0; i<gg->nrows; i++)
    gg->color_ids[i] = gg->color_now[i] = gg->color_prev[i] = gg->color_0;
}

/*-------------------------------------------------------------------------*/
/*                             erasing                                     */
/*-------------------------------------------------------------------------*/

void
hidden_alloc (ggobid *gg)
{
  if (gg->hidden != NULL) g_free (gg->hidden);
  if (gg->hidden_now != NULL) g_free (gg->hidden_now);
  if (gg->hidden_prev != NULL) g_free (gg->hidden_prev);

  gg->hidden = (gboolean *) g_malloc (gg->nrows * sizeof (gboolean));
  gg->hidden_now = (gboolean *) g_malloc (gg->nrows * sizeof (gboolean));
  gg->hidden_prev = (gboolean *) g_malloc (gg->nrows * sizeof (gboolean));
}
void
hidden_init (ggobid *gg)
{
  gint i;

  for (i=0; i<gg->nrows; i++)
    gg->hidden[i] = gg->hidden_now[i] = gg->hidden_prev[i] = false;
}


/*-------------------------------------------------------------------------*/
/*                           line color                                    */
/*-------------------------------------------------------------------------*/

void
br_line_vectors_free (ggobid *gg)
{
  vectors_free (&gg->line.color);
  vectors_free (&gg->line.color_now);
  vectors_free (&gg->line.color_prev);
  vectorb_free (&gg->line.hidden);
  vectorb_free (&gg->line.hidden_now);
  vectorb_free (&gg->line.hidden_prev);

  g_free ((gpointer) gg->line.xed_by_brush);
}

void
br_line_vectors_check_size (gint ns, ggobid *gg) {
  /*-- assume these vectors are always of the same size --*/
  if (gg->line.color.nels != ns) {
    vectors_realloc (&gg->line.color, ns);
    vectors_realloc (&gg->line.color_now, ns);
    vectors_realloc (&gg->line.color_prev, ns);
    vectorb_realloc (&gg->line.hidden, ns);
    vectorb_realloc (&gg->line.hidden_now, ns);
    vectorb_realloc (&gg->line.hidden_prev, ns);
  }
}

void
br_line_color_init (ggobid *gg)
{
  gint j;
  gshort *color, *color_now, *color_prev;
  gboolean *hidden, *hidden_now, *hidden_prev;

  br_line_vectors_check_size (gg->nsegments, gg);

  color = gg->line.color.data;
  color_now = gg->line.color_now.data;
  color_prev = gg->line.color_prev.data;
  hidden = gg->line.hidden.data;
  hidden_now = gg->line.hidden_now.data;
  hidden_prev = gg->line.hidden_prev.data;

  for (j=0; j<gg->nsegments; j++) {
    color[j] = color_now[j] = color_prev[j] = gg->color_0;
    hidden[j] = hidden_now[j] = hidden_prev[j] = false;
  }
}

/*-------------------------------------------------------------------------*/
/*                           the brush itself                              */
/*-------------------------------------------------------------------------*/

void
brush_pos_init (ggobid *gg)
{
  gg->brush.brush_pos.x1 = gg->brush.brush_pos.y1 = 20;
  gg->brush.brush_pos.x2 = gg->brush.brush_pos.y2 = 40;
}

/*----------------------------------------------------------------------*/
/*                          general                                     */
/*----------------------------------------------------------------------*/

void
brush_alloc (ggobid *gg)
/*
 * Dynamically allocate arrays.
*/
{
  guint nr = (guint) gg->nrows;
  gint i, iv, ih;
  gboolean initd = false;

  gg->brush.nbins = BRUSH_NBINS;

  gg->included = (gboolean *) g_realloc (gg->included, nr * sizeof (gboolean));
  gg->pts_under_brush = (gboolean *) g_realloc (gg->pts_under_brush,
                                               nr * sizeof (gboolean));

  for (i=0; i<nr; i++) {
    gg->included[i] = true;
    gg->pts_under_brush[i] = false;
  }

  /*
   * color_ids and glyph_ids and their kin were allocated when
   * the data was read in.
  */

  if (!initd) {
    /* binning the plot window; no need to realloc these */
    gg->brush.binarray = (bin_struct **)
      g_malloc (gg->brush.nbins * sizeof (bin_struct *));
    for (ih=0; ih<gg->brush.nbins; ih++) {
      gg->brush.binarray[ih] = (bin_struct *)
        g_malloc (gg->brush.nbins * sizeof (bin_struct));

      for (iv=0; iv<gg->brush.nbins; iv++) {
        gg->brush.binarray[ih][iv].nels = 0;
        gg->brush.binarray[ih][iv].nblocks = 1;
        gg->brush.binarray[ih][iv].els = (gulong *)
          g_malloc (BINBLOCKSIZE * sizeof (gulong));
      }
    }
    initd = true;
  }
}

void
brush_free (ggobid *gg)
/*
 * Dynamically free arrays.
*/
{
  int j,k;

  br_glyph_ids_free (gg);
  br_color_ids_free (gg);

  g_free ((gpointer) gg->pts_under_brush);

  for (k=0; k<gg->brush.nbins; k++) {
    for (j=0; j<gg->brush.nbins; j++)
      g_free ((gpointer) gg->brush.binarray[k][j].els);
    g_free ((gpointer) gg->brush.binarray[k]);
  }
  g_free ((gpointer) gg->brush.binarray);
}

void
brush_init (ggobid *gg)
{
  static gboolean firsttime = true;

  gg->glyph_id.type = gg->glyph_0.type = FILLED_CIRCLE;
  gg->glyph_id.size = gg->glyph_0.size = 3;
  gg->color_id = gg->color_0 = 0;

  if (firsttime) {
    brush_pos_init (gg);

    /*
     * Used in binning the plot window
    */
    gg->brush.nbins = BRUSH_NBINS;

    /*
     * These are initialized so that the first merge_brushbins()
     * call will behave reasonably.
    */
    gg->brush.bin0.x = gg->brush.bin1.x = BRUSH_NBINS;
    gg->brush.bin0.y = gg->brush.bin1.y = BRUSH_NBINS;

    brush_alloc (gg);

    firsttime = false;

  } else {
  }
}

void
brush_activate (gboolean state, ggobid *gg)
{
  if (state)
    assign_points_to_bins (gg);
}
