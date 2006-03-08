/* brush_init.c */
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
br_glyph_ids_free (GGobiData * d)
{
  vectorg_free (&d->glyph);
  vectorg_free (&d->glyph_now);
  vectorg_free (&d->glyph_prev);
}

void
br_glyph_ids_alloc (GGobiData * d)
{
  vectorg_alloc (&d->glyph, d->nrows);
  vectorg_alloc (&d->glyph_now, d->nrows);
  vectorg_alloc (&d->glyph_prev, d->nrows);
}

void
br_glyph_ids_init (GGobiData * d)
{
  gint i;

  g_assert (d->glyph.nels == d->nrows);

  for (i = 0; i < d->nrows; i++) {
    d->glyph.els[i].type = d->glyph_now.els[i].type =
      d->glyph_prev.els[i].type = FC;
    d->glyph.els[i].size = d->glyph_now.els[i].size =
      d->glyph_prev.els[i].size = 1;
  }
}

/*-- reallocates and initializes to the current glyph type and size --*/
void
br_glyph_ids_add (GGobiData * d, ggobid * gg)
{
  gint i, nprev = d->glyph.nels;

  vectorg_realloc (&d->glyph, d->nrows);
  vectorg_realloc (&d->glyph_now, d->nrows);
  vectorg_realloc (&d->glyph_prev, d->nrows);

  for (i = nprev; i < d->nrows; i++) {
    d->glyph.els[i].type = d->glyph_now.els[i].type =
      d->glyph_prev.els[i].type = gg->glyph_id.type;
    d->glyph.els[i].size = d->glyph_now.els[i].size =
      d->glyph_prev.els[i].size = gg->glyph_id.size;
  }
}


/*-------------------------------------------------------------------------*/
/*                       color                                             */
/*-------------------------------------------------------------------------*/

void
br_color_ids_free (GGobiData * d)
{
  vectors_free (&d->color);
  vectors_free (&d->color_now);
  vectors_free (&d->color_prev);
}

void
br_color_ids_alloc (GGobiData * d)
{
  vectors_realloc (&d->color, d->nrows);
  vectors_realloc (&d->color_now, d->nrows);
  vectors_realloc (&d->color_prev, d->nrows);
}

void
br_color_ids_init (GGobiData * d)
{
  gint i;

  g_assert (d->color.nels == d->nrows);

  for (i = 0; i < d->nrows; i++)
    d->color.els[i] = d->color_now.els[i] = d->color_prev.els[i] = 0;
}

/*-- reallocate and initialize colors --*/
void
br_color_ids_add (GGobiData * d, ggobid * gg)
{
  gint i, nprev = d->color.nels;

  vectors_realloc (&d->color, d->nrows);
  vectors_realloc (&d->color_now, d->nrows);
  vectors_realloc (&d->color_prev, d->nrows);

  /* initialize */
  for (i = nprev; i < d->nrows; i++)
    d->color.els[i] = d->color_now.els[i] = d->color_prev.els[i] =
      gg->color_id;
}

/*-------------------------------------------------------------------------*/
/*                             erasing                                     */
/*-------------------------------------------------------------------------*/

void
br_hidden_alloc (GGobiData * d)
{
  gint i, nprev = d->hidden.nels;

  vectorb_realloc (&d->hidden, d->nrows);
  vectorb_realloc (&d->hidden_now, d->nrows);
  vectorb_realloc (&d->hidden_prev, d->nrows);

  /* initialize to not hidden */
  for (i = nprev; i < d->nrows; i++)
    d->hidden.els[i] = d->hidden_now.els[i] = d->hidden_prev.els[i] = 0;
}

void
br_hidden_init (GGobiData * d)
{
  gint i;

  g_assert (d->hidden.nels == d->nrows);

  for (i = 0; i < d->nrows; i++)
    d->hidden.els[i] = d->hidden_now.els[i] = d->hidden_prev.els[i] = false;
}


/*-------------------------------------------------------------------------*/
/*                           edge color                                    */
/*-------------------------------------------------------------------------*/

void
br_edge_vectors_free (GGobiData * d, ggobid * gg)
{
  vectorb_free (&d->edge.xed_by_brush);
}

gboolean
br_edge_vectors_check_size (gint ns, GGobiData * d, ggobid * gg)
{
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
brush_pos_init (splotd * sp)
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
brush_alloc (GGobiData * d, ggobid * gg)
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

  for (i = 0; i < nr; i++) {
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
    for (ih = 0; ih < d->brush.nbins; ih++) {
      d->brush.binarray[ih] = (bin_struct *)
        g_malloc (d->brush.nbins * sizeof (bin_struct));

      for (iv = 0; iv < d->brush.nbins; iv++) {
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
brush_free (GGobiData * d, ggobid * gg)
/*
 * Dynamically free arrays.
*/
{
  int j, k;

  br_glyph_ids_free (d);
  br_color_ids_free (d);

  vectorb_free (&d->pts_under_brush);

  for (k = 0; k < d->brush.nbins; k++) {
    for (j = 0; j < d->brush.nbins; j++)
      g_free ((gpointer) d->brush.binarray[k][j].els);
    g_free ((gpointer) d->brush.binarray[k]);
  }
  g_free ((gpointer) d->brush.binarray);
}

void
brush_init (GGobiData * d, ggobid * gg)
{
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

RedrawStyle
brush_activate (gboolean state, displayd * display, splotd * sp, ggobid * gg)
{
  GGobiData *d = display->d;
  RedrawStyle redraw_style = NONE;

  if (sp != gg->current_splot)
    return redraw_style;

  if (GGOBI_IS_EXTENDED_SPLOT (sp)) {
    void (*f) (GGobiData *, splotd *, ggobid *);
    GGobiExtendedSPlotClass *klass;
    klass = GGOBI_EXTENDED_SPLOT_GET_CLASS (sp);
    if (state) {
      f = klass->splot_assign_points_to_bins;
      if (f) {
        f (d, sp, gg);          // need to exclude area plots
      }
    }
  }

  return redraw_style;
}
