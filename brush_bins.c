/* brush_bins.c */
/*
 * Routines for dealing with the use of "binning" of the current_splot
 * drawing area to optimize brushing speed
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


void
assign_points_to_bins ()
{
  splotd *sp = gg.current_splot;
  gint i, k, ih, iv;

  /*
   * Reset bin counts to zero -- but don't bother to free any space.
  */
  for (ih=0; ih<gg.br_nbins; ih++)
    for (iv=0; iv<gg.br_nbins; iv++)
      gg.br_binarray[ih][iv].nels = 0;

  for (k=0; k<gg.nrows_in_plot; k++) {
    i = gg.rows_in_plot[k];

    if (sp->screen[i].x >=0 && sp->screen[i].x <= sp->max.x &&
        sp->screen[i].y >=0 && sp->screen[i].y <= sp->max.y)
    {
      if (point_in_which_bin (sp->screen[i].x, sp->screen[i].y, &ih, &iv))
      {
        /* See whether it's necessary to allocate more space for elements */
        if (gg.br_binarray[ih][iv].nels == gg.br_binarray[ih][iv].nblocks *
                                           BINBLOCKSIZE)
        {
          gg.br_binarray[ih][iv].nblocks += 1;
          gg.br_binarray[ih][iv].els = (gulong *)
            g_realloc ((gpointer) gg.br_binarray[ih][iv].els,
              gg.br_binarray[ih][iv].nblocks * BINBLOCKSIZE * sizeof (gulong));
        }
        /*
         * br_binarray contains the
         * index of rows_in_plot[] rather than the contents, so
         * here the assignment is k rather than i
        */
        gg.br_binarray[ih][iv].els[gg.br_binarray[ih][iv].nels] = (gulong) k;
        gg.br_binarray[ih][iv].nels += 1;
      }
    }
  }
}

void
get_extended_brush_corners (icoords *bin0, icoords *bin1)
{
  static brush_coords obrush;
  static gboolean initd = false;
  gint x1 = MIN (gg.app.brush_pos.x1, gg.app.brush_pos.x2);
  gint y1 = MIN (gg.app.brush_pos.y1, gg.app.brush_pos.y2);
  gint x2 = MAX (gg.app.brush_pos.x1, gg.app.brush_pos.x2);
  gint y2 = MAX (gg.app.brush_pos.y1, gg.app.brush_pos.y2);
  gint ox1, oy1, ox2, oy2;

  if (!initd)
  {
    /* from initial values */
    obrush.x1 = obrush.y1 = 20;
    obrush.x2 = obrush.y2 = 40;
    initd = true;
  }

  ox1 = MIN (obrush.x1, obrush.x2);
  oy1 = MIN (obrush.y1, obrush.y2);
  ox2 = MAX (obrush.x1, obrush.x2);
  oy2 = MAX (obrush.y1, obrush.y2);

/*
 * What bins contain the brush and the previous brush?  Allow
 * extension for safety, using BRUSH_MARGIN.
*/

  if (!point_in_which_bin (MIN (x1, ox1) - 2*BRUSH_MARGIN,
                           MIN (y1, oy1) - 2*BRUSH_MARGIN,
                           &bin0->x, &bin0->y) )
  {
    bin0->x = MAX (bin0->x, 0);
    bin0->x = MIN (bin0->x, gg.br_nbins - 1);
    bin0->y = MAX (bin0->y, 0);
    bin0->y = MIN (bin0->y, gg.br_nbins - 1);
  }
  if (!point_in_which_bin(MAX (x2, ox2) + 2*BRUSH_MARGIN,
                          MAX (y2, oy2) + 2*BRUSH_MARGIN,
                          &bin1->x, &bin1->y) )
  {
    bin1->x = MAX (bin1->x, 0);
    bin1->x = MIN (bin1->x, gg.br_nbins - 1);
    bin1->y = MAX (bin1->y, 0);
    bin1->y = MIN (bin1->y, gg.br_nbins - 1);
  }

  obrush.x1 = gg.app.brush_pos.x1;
  obrush.y1 = gg.app.brush_pos.y1;
  obrush.x2 = gg.app.brush_pos.x2;
  obrush.y2 = gg.app.brush_pos.y2;
}

gboolean
point_in_which_bin (gint x, gint y, gint *ih, gint *iv)
{
  gboolean inwindow = true;
  splotd *sp = gg.current_splot;

  *ih = (gint) ((gfloat) gg.br_nbins * (gfloat) x / (sp->max.x+1.0));
  *iv = (gint) ((gfloat) gg.br_nbins * (gfloat) y / (sp->max.y+1.0));

  if (*ih < 0 || *ih > gg.br_nbins - 1 || *iv < 0 || *iv > gg.br_nbins - 1)
    inwindow = false;

  return (inwindow);
}
