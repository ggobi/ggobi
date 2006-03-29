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
/*                           edge color                                    */
/*-------------------------------------------------------------------------*/

void
br_edge_vectors_free (GGobiData * d)
{
  vectorb_free (&d->edge.xed_by_brush);
}

gboolean
br_edge_vectors_check_size (gint ns, GGobiData * d)
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
brush_free (GGobiData * d)
/*
 * Dynamically free arrays.
*/
{
  int j, k;

  for (k = 0; k < d->brush.nbins; k++) {
    for (j = 0; j < d->brush.nbins; j++)
      g_free ((gpointer) d->brush.binarray[k][j].els);
    g_free ((gpointer) d->brush.binarray[k]);
  }
  g_free ((gpointer) d->brush.binarray);
}


RedrawStyle
brush_activate (gboolean state, displayd * display, splotd * sp)
{
  GGobiData *d = display->d;
  RedrawStyle redraw_style = NONE;

  if (sp != d->gg->current_splot)
    return redraw_style;

  if (GGOBI_IS_EXTENDED_SPLOT (sp)) {
    void (*f) (GGobiData *, splotd *, ggobid *);
    GGobiExtendedSPlotClass *klass;
    klass = GGOBI_EXTENDED_SPLOT_GET_CLASS (sp);
    if (state) {
      f = klass->splot_assign_points_to_bins;
      if (f) {
        f (d, sp, d->gg);          // need to exclude area plots
      }
    }
  }

  return redraw_style;
}
