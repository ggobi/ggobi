/* brush_init.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
/*
 * Allocation and initialization routines for brushing.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include <string.h> /* for memset() */

/* external variables */

/*-------------------------------------------------------------------------*/
/*                      glyphs                                             */
/*-------------------------------------------------------------------------*/

void
br_glyph_ids_free (datad *d, ggobid *gg)
{
  g_free (d->glyph_ids);
  g_free (d->glyph_now);
  g_free (d->glyph_prev);
}

void
br_glyph_ids_alloc (datad *d, ggobid *gg)
{
  d->glyph_ids = (glyphv *) g_realloc (d->glyph_ids,
                                       d->nrows * sizeof (glyphv));
  d->glyph_now = (glyphv *) g_realloc (d->glyph_now,
                                       d->nrows * sizeof (glyphv));
  d->glyph_prev = (glyphv *) g_realloc (d->glyph_prev,
                                       d->nrows * sizeof (glyphv));
}

void
br_glyph_ids_init (datad *d, ggobid *gg)
{
  gint j;

  for (j=0; j<d->nrows; j++) {
    d->glyph_ids[j].type = d->glyph_now[j].type =
      d->glyph_prev[j].type = gg->glyph_0.type;
    d->glyph_ids[j].size = d->glyph_now[j].size =
      d->glyph_prev[j].size = gg->glyph_0.size;
  }
}

/*-------------------------------------------------------------------------*/
/*                       color                                             */
/*-------------------------------------------------------------------------*/

void
br_color_ids_free (datad *d, ggobid *gg)
{
  g_free (d->color_ids);
  g_free (d->color_now);
  g_free (d->color_prev);
}

void
br_color_ids_alloc (datad *d, ggobid *gg)
{
  gint i;

  d->color_ids = (gshort *)  g_realloc (d->color_ids,
                                        d->nrows * sizeof (gshort));
  d->color_now = (gshort *)  g_realloc (d->color_now,
                                        d->nrows * sizeof (gshort));
  d->color_prev = (gshort *) g_realloc (d->color_prev,
                                        d->nrows * sizeof (gshort));
  for (i=0; i<d->nrows; i++)
    d->color_ids[i] = d->color_now[i] = d->color_prev[i] = gg->color_0;
}

void
br_color_ids_init (datad *d, ggobid *gg)
{
  gint i;

  gg->color_id = gg->color_0;
  for (i=0; i<d->nrows; i++)
    d->color_ids[i] = d->color_now[i] = d->color_prev[i] = gg->color_0;
}

/*-------------------------------------------------------------------------*/
/*                             erasing                                     */
/*-------------------------------------------------------------------------*/

void
hidden_alloc (datad *d, ggobid *gg)
{
  if (d->hidden != NULL) g_free (d->hidden);
  if (d->hidden_now != NULL) g_free (d->hidden_now);
  if (d->hidden_prev != NULL) g_free (d->hidden_prev);

  d->hidden = (gboolean *) g_malloc (d->nrows * sizeof (gboolean));
  d->hidden_now = (gboolean *) g_malloc (d->nrows * sizeof (gboolean));
  d->hidden_prev = (gboolean *) g_malloc (d->nrows * sizeof (gboolean));
 
  memset (d->hidden, '\0', sizeof (gboolean)*d->nrows);
  memset (d->hidden_now, '\0', sizeof (gboolean)*d->nrows);
  memset (d->hidden_prev, '\0', sizeof (gboolean)*d->nrows);
}

void
hidden_init (datad *d, ggobid *gg)
{
  gint i;

  for (i=0; i<d->nrows; i++)
    d->hidden[i] = d->hidden_now[i] = d->hidden_prev[i] = false;
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
  vectorb_free (&gg->line.xed_by_brush);
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
    vectorb_realloc (&gg->line.xed_by_brush, ns);
  }
}

void
br_line_color_init (ggobid *gg)
{
  gint j;
  gshort *color, *color_now, *color_prev;
  gboolean *hidden, *hidden_now, *hidden_prev;

  br_line_vectors_check_size (gg->nedges, gg);

  color = gg->line.color.vals;
  color_now = gg->line.color_now.vals;
  color_prev = gg->line.color_prev.vals;
  hidden = gg->line.hidden.vals;
  hidden_now = gg->line.hidden_now.vals;
  hidden_prev = gg->line.hidden_prev.vals;

  for (j=0; j<gg->nedges; j++) {
    color[j] = color_now[j] = color_prev[j] = gg->color_0;
    hidden[j] = hidden_now[j] = hidden_prev[j] = false;
  }
}

/*-------------------------------------------------------------------------*/
/*                           the brush itself                              */
/*-------------------------------------------------------------------------*/

void
brush_pos_init (datad *d)
{
  d->brush_pos.x1 = d->brush_pos.y1 = 20;
  d->brush_pos.x2 = d->brush_pos.y2 = 40;
}

/*----------------------------------------------------------------------*/
/*                          general                                     */
/*----------------------------------------------------------------------*/

void
brush_alloc (datad *d, ggobid *gg)
/*
 * Dynamically allocate arrays.
*/
{
  guint nr = (guint) d->nrows, i;
  gint iv, ih;
  gboolean initd = false;

  d->brush.nbins = BRUSH_NBINS;

  d->included = (gboolean *) g_realloc (d->included, nr * sizeof (gboolean));
  d->pts_under_brush = (gboolean *) g_realloc (d->pts_under_brush,
                                               nr * sizeof (gboolean));

  for (i=0; i<nr; i++) {
    d->included[i] = true;
    d->pts_under_brush[i] = false;
  }

  /*
   * color_ids and glyph_ids and their kin were allocated when
   * the data was read in.
  */

  if (!initd) {
    /* binning the plot window; no need to realloc these */
    d->brush.binarray = (bin_struct **)
      g_malloc (d->brush.nbins * sizeof (bin_struct *));
    for (ih=0; ih<d->brush.nbins; ih++) {
      d->brush.binarray[ih] = (bin_struct *)
        g_malloc (d->brush.nbins * sizeof (bin_struct));

      for (iv=0; iv<d->brush.nbins; iv++) {
        d->brush.binarray[ih][iv].nels = 0;
        d->brush.binarray[ih][iv].nblocks = 1;
        d->brush.binarray[ih][iv].els = (gulong *)
          g_malloc (BINBLOCKSIZE * sizeof (gulong));
      }
    }
    initd = true;
  }
}

void
brush_free (datad *d, ggobid *gg)
/*
 * Dynamically free arrays.
*/
{
  int j,k;

  br_glyph_ids_free (d, gg);
  br_color_ids_free (d, gg);

  g_free ((gpointer) d->pts_under_brush);

  for (k=0; k<d->brush.nbins; k++) {
    for (j=0; j<d->brush.nbins; j++)
      g_free ((gpointer) d->brush.binarray[k][j].els);
    g_free ((gpointer) d->brush.binarray[k]);
  }
  g_free ((gpointer) d->brush.binarray);
}

void
brush_init (datad *d, ggobid *gg)
{
  gg->glyph_id.type = gg->glyph_0.type = FILLED_CIRCLE;
  gg->glyph_id.size = gg->glyph_0.size = 3;
  gg->color_id = gg->color_0 = 0;

  brush_pos_init (d);

  /*
   * Used in binning the plot window
  */
  d->brush.nbins = BRUSH_NBINS;

  /*
   * These are initialized so that the first merge_brushbins()
   * call will behave reasonably.
  */
  d->brush.bin0.x = d->brush.bin1.x = BRUSH_NBINS;
  d->brush.bin0.y = d->brush.bin1.y = BRUSH_NBINS;

  brush_alloc (d, gg);
}

void
brush_activate (gboolean state, datad *d, ggobid *gg)
{
  if (state)
    assign_points_to_bins (d, gg);
}
