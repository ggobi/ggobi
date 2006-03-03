/* brush_bins.c */
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
 * Routines for dealing with the use of "binning" of the current_splot
 * drawing area to optimize brushing speed
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


void
assign_points_to_bins (GGobiData *d, splotd *sp, ggobid *gg)
{
  gint i, k, ih, iv;

  /*
   * Reset bin counts to zero -- but don't bother to free any space.
  */
  for (ih=0; ih<d->brush.nbins; ih++)
    for (iv=0; iv<d->brush.nbins; iv++)
      d->brush.binarray[ih][iv].nels = 0;

  for (k=0; k<d->nrows_in_plot; k++) {
    i = d->rows_in_plot.els[k];

    if (sp->screen[i].x >=0 && sp->screen[i].x <= sp->max.x &&
        sp->screen[i].y >=0 && sp->screen[i].y <= sp->max.y)
    {
      if (point_in_which_bin (sp->screen[i].x, sp->screen[i].y,
        &ih, &iv, d, sp))
      {
        /* See whether it's necessary to allocate more space for elements */
        if (d->brush.binarray[ih][iv].nels ==
          d->brush.binarray[ih][iv].nblocks * BINBLOCKSIZE)
        {
          d->brush.binarray[ih][iv].nblocks += 1;
          d->brush.binarray[ih][iv].els = (gulong *)
            g_realloc ((gpointer) d->brush.binarray[ih][iv].els,
              d->brush.binarray[ih][iv].nblocks * BINBLOCKSIZE *
              sizeof (gulong));
        }
        /*
         * brush.binarray contains the index of rows_in_plot[] rather
         * than the contents, so the assignment is k rather than i
        */
        d->brush.binarray[ih][iv].els[d->brush.binarray[ih][iv].nels] =
          (gulong) k;
        d->brush.binarray[ih][iv].nels += 1;
      }
    }
  }

}

void
get_extended_brush_corners (icoords *bin0, icoords *bin1, GGobiData *d, splotd *sp)
{
  brush_coords *brush = &sp->brush_pos;
  brush_coords *obrush = &sp->brush_pos_o;
  gint x1 = MIN (brush->x1, brush->x2);
  gint y1 = MIN (brush->y1, brush->y2);
  gint x2 = MAX (brush->x1, brush->x2);
  gint y2 = MAX (brush->y1, brush->y2);
  gint ox1, oy1, ox2, oy2;

  ox1 = MIN (obrush->x1, obrush->x2);
  oy1 = MIN (obrush->y1, obrush->y2);
  ox2 = MAX (obrush->x1, obrush->x2);
  oy2 = MAX (obrush->y1, obrush->y2);

/*
 * What bins contain the brush and the previous brush?  Allow
 * extension for safety, using BRUSH_MARGIN.
*/

  if (!point_in_which_bin (MIN (x1, ox1) - 2*BRUSH_MARGIN,
                           MIN (y1, oy1) - 2*BRUSH_MARGIN,
                           &bin0->x, &bin0->y, d, sp) )
  {
    bin0->x = MAX (bin0->x, 0);
    bin0->x = MIN (bin0->x, d->brush.nbins - 1);
    bin0->y = MAX (bin0->y, 0);
    bin0->y = MIN (bin0->y, d->brush.nbins - 1);
  }
  if (!point_in_which_bin(MAX (x2, ox2) + 2*BRUSH_MARGIN,
                          MAX (y2, oy2) + 2*BRUSH_MARGIN,
                          &bin1->x, &bin1->y, d, sp) )
  {
    bin1->x = MAX (bin1->x, 0);
    bin1->x = MIN (bin1->x, d->brush.nbins - 1);
    bin1->y = MAX (bin1->y, 0);
    bin1->y = MIN (bin1->y, d->brush.nbins - 1);
  }

  obrush->x1 = brush->x1;
  obrush->y1 = brush->y1;
  obrush->x2 = brush->x2;
  obrush->y2 = brush->y2;
}

gboolean
point_in_which_bin (gint x, gint y, gint *ih, gint *iv, GGobiData *d, splotd *sp)
{
  gboolean inwindow = true;

  *ih = (gint) ((gfloat) d->brush.nbins * (gfloat) x / (sp->max.x+1.0));
  *iv = (gint) ((gfloat) d->brush.nbins * (gfloat) y / (sp->max.y+1.0));

  if (*ih < 0 ||
      *ih > d->brush.nbins - 1 ||
      *iv < 0 ||
      *iv > d->brush.nbins - 1)
  {
    inwindow = false;
  }

  return (inwindow);
}
