/* p1d.c */

#include <gtk/gtk.h>
#include "vars.h"

/* external functions */
extern gint do_ash1d (gfloat *, gint, gint, gint, gfloat *, gfloat *, gfloat *);
extern void textur (gfloat *, gfloat *, gint, gint, gfloat, gint);
/* */

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
p1d_spread_var (displayd *disp, splotd *sp, gint jvar) {
/*
 * Set up the next dot plot.
*/
  gint i;
  gfloat del = 1.;
  gint option = 1, stages = 3;
  gfloat min, max;
  cpaneld *cpanel = &disp->cpanel;

  gfloat *yy = (gfloat *) g_malloc (xg.nrows_in_plot * sizeof (gfloat));
  for (i=0; i<xg.nrows_in_plot; i++)
    yy[i] = xg.tform2[ xg.rows_in_plot[i] ][jvar];

/*
 * This shouldn't be here.  In xgobi, it was allocated when
 * the mode was selected and freed when it was deselected
*/
  if (sp->p1d_data == null)
    sp->p1d_data = (gfloat *) g_malloc (xg.nrows * sizeof (gfloat));
/* */

  switch (cpanel->p1d_type) {
    case DOTPLOT:
      sp->p1d_lim.min = FORGETITAXIS_MIN ;
      sp->p1d_lim.max = FORGETITAXIS_MAX ;
      for (i=0; i<xg.nrows_in_plot; i++)
        sp->p1d_data[i] = 50;
      break;   

    case TEXTURE:
      sp->p1d_lim.min = FORGETITAXIS_MIN ;
      sp->p1d_lim.max = FORGETITAXIS_MAX ;
      textur (yy, sp->p1d_data, xg.nrows_in_plot, option, del, stages);
      break;

    case ASH:
      do_ash1d (yy, xg.nrows_in_plot,
               cpanel->nbins, cpanel->nASHes,
               sp->p1d_data, &min, &max);
      sp->p1d_lim.min = min;
      sp->p1d_lim.max = max;
      break;   
  }
  g_free ((gpointer) yy);

}

void
p1d_reproject (splotd *sp)
{
/*
 * Project the y variable down from the ncols-dimensional world_data[]
 * to the 2-dimensional array planar[]; get the x variable directly
 * from p1d_data[].
*/
  gint i, k, jvar = 0;
  gfloat rdiff = sp->p1d_lim.max - sp->p1d_lim.min;
  gfloat ftmp;
  gfloat precis = PRECISION1;
  displayd *disp = (displayd *) sp->displayptr;

  if (sp == NULL)
    return;

  jvar = sp->p1dvar;

  /* First generate sp->p1d_data */
  p1d_spread_var (disp, sp, jvar);

  rdiff = sp->p1d_lim.max - sp->p1d_lim.min;
  /* Then project it */
  for (i=0; i<xg.nrows_in_plot; i++) {
    k = xg.rows_in_plot[i];

    /*
     * Use p1d_data[i] not [k] because p1d_data[]
     * is of length xg.nrows_in_plot rather than xg.nrows.
    */
    ftmp = -1.0 + 2.0*(sp->p1d_data[i] - sp->p1d_lim.min)/rdiff;

    /*
     * Now here's a charming kludge:  since we'd rather have the
     * jitter perpendicular to the axes, subtract it from the
     * selected variable and swap it onto the jitter variable.
     * November 1999 -- dfs
    */

    if (disp->p1d_orientation == VERTICAL) {
      sp->planar[k].x = (long) (precis * ftmp) + xg.jitter_data[k][jvar];
      sp->planar[k].y = xg.world_data[k][jvar] - xg.jitter_data[k][jvar];
    } else {
      sp->planar[k].x = xg.world_data[k][jvar] - xg.jitter_data[k][jvar];
      sp->planar[k].y = (long) (precis * ftmp) + xg.jitter_data[k][jvar];
    }

  }
}

