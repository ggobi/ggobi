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
  g_free (d->glyph);
  g_free (d->glyph_now);
  g_free (d->glyph_prev);
}

void
br_glyph_ids_alloc (datad *d)
{
  d->glyph = (glyphv *) g_realloc (d->glyph,
                                   d->nrows * sizeof (glyphv));
  d->glyph_now = (glyphv *) g_realloc (d->glyph_now,
                                       d->nrows * sizeof (glyphv));
  d->glyph_prev = (glyphv *) g_realloc (d->glyph_prev,
                                        d->nrows * sizeof (glyphv));
}

void
br_glyph_ids_init (datad *d, ggobid *gg)
{
  gint i;

  for (i=0; i<d->nrows; i++) {
    d->glyph[i].type = d->glyph_now[i].type =
      d->glyph_prev[i].type = gg->glyph_0.type;
    d->glyph[i].size = d->glyph_now[i].size =
      d->glyph_prev[i].size = gg->glyph_0.size;
  }
}

/*-------------------------------------------------------------------------*/
/*                       color                                             */
/*-------------------------------------------------------------------------*/

void
br_color_ids_free (datad *d, ggobid *gg)
{
  vectors_free (&d->color);
  vectors_free (&d->color_now);
  vectors_free (&d->color_prev);
}

void
br_color_ids_alloc (datad *d, ggobid *gg)
{
  gint i;

  vectors_realloc (&d->color, d->nrows);
  vectors_realloc (&d->color_now, d->nrows);
  vectors_realloc (&d->color_prev, d->nrows);

  for (i=0; i<d->nrows; i++)
    d->color.els[i] = d->color_now.els[i] = d->color_prev.els[i] =
      gg->color_0;
}

void
br_color_ids_init (datad *d, ggobid *gg)
{
  gint i;

  gg->color_id = gg->color_0;
  for (i=0; i<d->nrows; i++)
    d->color.els[i] = d->color_now.els[i] = d->color_prev.els[i] =
      gg->color_0;
}

/*-------------------------------------------------------------------------*/
/*                             erasing                                     */
/*-------------------------------------------------------------------------*/

void
hidden_alloc (datad *d)
{
  vectorb_realloc (&d->hidden, d->nrows);
  vectorb_realloc (&d->hidden_now, d->nrows);
  vectorb_realloc (&d->hidden_prev, d->nrows);
 
  memset (d->hidden.els, '\0', sizeof (gboolean)*d->nrows);
  memset (d->hidden_now.els, '\0', sizeof (gboolean)*d->nrows);
  memset (d->hidden_prev.els, '\0', sizeof (gboolean)*d->nrows);
}

void
hidden_init (datad *d, ggobid *gg)
{
  gint i;

  for (i=0; i<d->nrows; i++)
    d->hidden.els[i] = d->hidden_now.els[i] = d->hidden_prev.els[i] = false;
}


/*-------------------------------------------------------------------------*/
/*                           edge color                                    */
/*-------------------------------------------------------------------------*/

void
br_edge_vectors_free (datad *d, ggobid *gg)
{
  vectorb_free (&d->edge.xed_by_brush);
}

gboolean
br_edge_vectors_check_size (gint ns, datad *d, ggobid *gg) {
  gboolean same = (d->edge.xed_by_brush.nels != ns);

  if (!same) {
    vectorb_realloc (&d->edge.xed_by_brush, ns);
  }

  return same;
}

/*-------------------------------------------------------------------------*/
/*                           the brush itself                              */
/*-------------------------------------------------------------------------*/

void
brush_pos_init (splotd *sp)
{
  sp->brush_pos.x1 = sp->brush_pos.y1 = 20;
  sp->brush_pos.x2 = sp->brush_pos.y2 = 40;

  sp->brush_pos_o.x1 = sp->brush_pos_o.y1 = 20;
  sp->brush_pos_o.x2 = sp->brush_pos_o.y2 = 40;
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

  vectorb_realloc (&d->pts_under_brush, nr);
  if (d->edge.n)
    vectorb_realloc (&d->edge.xed_by_brush, d->edge.n);

  for (i=0; i<nr; i++) {
    d->pts_under_brush.els[i] = false;
  }

  /*
   * color and glyph and their kin were allocated when
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

  vectorb_free (&d->pts_under_brush);

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
  /*-- duplicated in globals_init -- is this redundant? --*/
  gg->glyph_id.type = gg->glyph_0.type = FILLED_CIRCLE;
  gg->glyph_id.size = gg->glyph_0.size = 3;
  gg->color_id = 0;
  gg->color_0 = 4;

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

  vectorb_init_null (&d->pts_under_brush);
  brush_alloc (d, gg);
}

enum redrawStyle
brush_activate (gboolean state, displayd *display, ggobid *gg)
{
  datad *d = display->d;
  enum redrawStyle redraw_style = NONE;

  if (state)
    assign_points_to_bins (d, gg);

  else {
    /*
     * If transient brushing, restore the color of the transiently
     * brushed points to their previous color
    */
    reinit_transient_brushing (display, gg);
    redraw_style = FULL;
  }

  return redraw_style;
}
