/* p1d.c */
/*
 * The 1d plots use the values of world_data (or missing_world_data)
 * for the variable of interest, but they use tform2 (or missing)
 * to do the spreading calculations, which are done in floating point.
 *
 * At the very end, to deal with jittering, they remove the jitter
 * from the selected variable and add it to the spread direction.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*
 * min and max for 'forget it' dotplot's texturing axis;
 * they're defined so as to locate the plot in the center of
 * the window, with the texturing values returned on a range
 * of [0,100].
 *
 * The min and max are calculated on the fly for the ash.
*/
#define FORGETITAXIS_MIN -100.
#define FORGETITAXIS_MAX 200.

void
p1d_spread_var (displayd *display, gfloat *yy, splotd *sp) {
/*
 * Set up the next dot plot.
*/
  gint i;
  gfloat del = 1.;
  gint option = 1, stages = 3;
  gfloat min, max;
  cpaneld *cpanel = &display->cpanel;

  switch (cpanel->p1d_type) {
    case TEXTURE:
      sp->p1d_lim.min = FORGETITAXIS_MIN ;
      sp->p1d_lim.max = FORGETITAXIS_MAX ;

      textur (yy, sp->p1d_data, gg.nrows_in_plot, option, del, stages);
      break;

    case ASH:
      do_ash1d (yy, gg.nrows_in_plot,
               cpanel->nbins, cpanel->nASHes,
               sp->p1d_data, &min, &max);
      sp->p1d_lim.min = min;
      sp->p1d_lim.max = max;
      break;   

    case DOTPLOT:
      sp->p1d_lim.min = FORGETITAXIS_MIN ;
      sp->p1d_lim.max = FORGETITAXIS_MAX ;
      for (i=0; i<gg.nrows_in_plot; i++)
        sp->p1d_data[i] = 50;  /*-- halfway between _MIN and _MAX --*/
      break;   
  }
}

void
p1d_reproject (splotd *sp, glong **world_data)
{
/*
 * Project the y variable down from the ncols-dimensional world_data[]
 * to the 2-dimensional array planar[]; get the x variable directly
 * from p1d_data[].
*/
  gint i, m, jvar = 0;
  gfloat rdiff, ftmp;
  gfloat precis = PRECISION1;
  displayd *display = (displayd *) sp->displayptr;
  gfloat *yy;

  if (sp == NULL)
    return;

  yy = (gfloat *) g_malloc (gg.nrows_in_plot * sizeof (gfloat));
  jvar = sp->p1dvar;

  if (display->missing_p) {
    for (i=0; i<gg.nrows_in_plot; i++)
      yy[i] = (gfloat) gg.missing.data[gg.rows_in_plot[i]][jvar];
  } else {
    for (i=0; i<gg.nrows_in_plot; i++)
      yy[i] = gg.tform2.data[gg.rows_in_plot[i]][jvar];
  }

  p1d_spread_var (display, yy, sp);

  /* Then project it */
  rdiff = sp->p1d_lim.max - sp->p1d_lim.min;
  for (i=0; i<gg.nrows_in_plot; i++) {
    m = gg.rows_in_plot[i];

    /*
     * Use p1d_data[i] not [m] because p1d_data[] is populated
     * only up to gg.nrows_in_plot
    */
    ftmp = -1.0 + 2.0*(sp->p1d_data[i] - sp->p1d_lim.min)/rdiff;

    if (display->p1d_orientation == VERTICAL) {
      sp->planar[m].x = (glong) (precis * ftmp);
      sp->planar[m].y = world_data[m][jvar];
    } else {
      sp->planar[m].x = world_data[m][jvar];
      sp->planar[m].y = (glong) (precis * ftmp);
    }

    /*
     * Since we want the jitter perpendicular to the axis, subtract
     * the jitter from the selected variable and add it to the
     * jitter or spread variable.  Leave missings alone, since we
     * probably want jitter in both directions.
    */
    if (!display->missing_p) {
      if (display->p1d_orientation == VERTICAL) {
        sp->planar[m].x += gg.jitter.data[m][jvar];
        sp->planar[m].y -= gg.jitter.data[m][jvar];
      } else {
        sp->planar[m].x -= gg.jitter.data[m][jvar];
        sp->planar[m].y += gg.jitter.data[m][jvar];
      }
    }
  }

  g_free ((gpointer) yy);
}

gboolean
p1d_varsel (splotd *sp, gint jvar, gint *jvar_prev, gint button)
{
  gboolean redraw = true;
  displayd *display = (displayd *) sp->displayptr;
  gint orientation = display->p1d_orientation;

  display->p1d_orientation = (button == 1) ? HORIZONTAL : VERTICAL;

  redraw = (orientation != display->p1d_orientation) || (jvar != sp->p1dvar);

  *jvar_prev = sp->p1dvar;
  sp->p1dvar = jvar;

  if (orientation != display->p1d_orientation)
    scatterplot_show_rulers (display, P1PLOT);

  return redraw;
}
