/*-- pipeline.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static gfloat mean_largest_dist (gfloat **, gint *, gint, gfloat *, gfloat *, datad *, ggobid *);
static gfloat median_largest_dist (gfloat **, gint *, gint, gfloat *, gfloat *, datad *, ggobid *);

/* ------------ Dynamic allocation, freeing section --------- */

void
pipeline_arrays_free (datad *d, ggobid *gg)
/*
 * Dynamically free arrays used in data pipeline.
*/
{
  arrayf_free (&d->tform, 0, 0);

  arrayl_free (&d->world, 0, 0);
  arrayl_free (&d->jitdata, 0, 0);

  g_free ((gpointer) d->rows_in_plot);
  g_free ((gpointer) d->sampled);
}

void
pipeline_arrays_alloc (datad *d, ggobid *gg)
/*
 * Dynamically allocate arrays.
*/
{
  gint nc = d->ncols, nr = d->nrows;

  if (d->tform.vals != NULL) pipeline_arrays_free (d, gg);

  arrayf_alloc (&d->tform, nr, nc);

  arrayl_alloc (&d->world, nr, nc);
  arrayl_alloc_zero (&d->jitdata, nr, nc);

  d->rows_in_plot = (gint *) g_malloc (nr * sizeof (gint));
  d->sampled = (gboolean *) g_malloc (nr * sizeof (gboolean));
}

void
pipeline_arrays_add_column (gint jvar, datad *d, ggobid *gg)
/*
 * Reallocate pipeline arrays to contain one more column, and
 * copy column jvar into the new column
*/
{
  gint nc = d->ncols + 1, nr = d->nrows;
  register gint i;

  arrayf_add_cols (&d->raw, nc);
  arrayf_add_cols (&d->tform, nc);

  arrayl_add_cols (&d->world, nc);
  arrayl_add_cols (&d->jitdata, nc);

  for (i=0; i<nr; i++) {
    /*-- no tform --*/
    d->raw.vals[i][nc-1] = d->tform.vals[i][nc-1] = d->raw.vals[i][jvar];
    /*-- no jitter --*/
    d->jitdata.vals[i][nc-1] = 0; 
  }

  /*-- world data is allocated but not populated --*/
}


/*-------------------------------------------------------------------------*/
/*               mucking about with the variable limits                    */
/*-------------------------------------------------------------------------*/

void
min_max (gfloat **vals, gint jvar, gfloat *min, gfloat *max,
  datad *d, ggobid *gg)
/*
 * Find the minimum and maximum values of variable jvar
 * using min-max scaling.
*/
{
  int i, k;
/*
 * Choose an initial value for *min and *max
*/
  *min = *max = vals[d->rows_in_plot[0]][jvar];

  for (i=0; i<d->nrows_in_plot; i++) {
    k = d->rows_in_plot[i];
    if (vals[k][jvar] < *min)
      *min = vals[k][jvar];
    else if (vals[k][jvar] > *max)
      *max = vals[k][jvar];
  }
}

void
limits_adjust (gfloat *min, gfloat *max)
/*
 * This test could be cleverer.  It could test the ratios
 * lim[i].min/rdiff and lim[i].max/rdiff for overflow or
 * rdiff/lim[i].min and rdiff/lim[i].max for underflow.
 * It should probably do it inside a while loop, too.
 * See Advanced C, p 187.  Set up gfloation point exception
 * handler which alters the values of lim[i].min and lim[i].max
 * until no exceptions occur.
*/
{
  if (*max - *min == 0) {
    if (*min == 0.0) {
      *min = -1.0;
      *max = 1.0;
    } else {
      *min = .9 * *min;
      *max = 1.1 * *max;
    }
  }

  /* This is needed to account for the case that max == min < 0 */
  if (*max < *min) {
    gfloat ftmp = *max;
    *max = *min;
    *min = ftmp;
  }
}

void
vartable_lim_update (datad *d, ggobid *gg)
{
  gint j;
  gfloat min, max;

  for (j=0; j<d->ncols; j++) {

    if (d->vartable[j].lim_specified_p) {
      min = d->vartable[j].lim_specified_tform.min;
      max = d->vartable[j].lim_specified_tform.max;
    } else {
      min = d->vartable[j].lim_tform.min;
      max = d->vartable[j].lim_tform.max;
    }

    limits_adjust (&min, &max);

    d->vartable[j].lim.min = min;
    d->vartable[j].lim.max = max;
  }
}

/*-------------------------------------------------------------------------*/
/*                       pipeline                                          */
/*-------------------------------------------------------------------------*/


gint
icompare (gint *x1, gint *x2)
{
  gint val = 0;

  if (*x1 < *x2)
    val = -1;
  else if (*x1 > *x2)
    val = 1;

  return (val);
}

gfloat
median_largest_dist (gfloat **vals, gint *cols, gint ncols,
  gfloat *min, gfloat *max, datad *d, ggobid *gg)
{
/*
 * Find the minimum and maximum values of each column or variable
 * group scaling by median and largest distance
*/
  gint i, j, k, n, np;
  gdouble dx, sumdist, lgdist = 0.0;
  gfloat *x, fmedian;
  gdouble dmedian = 0;

  np = ncols * d->nrows_in_plot;
  x = (gfloat *) g_malloc (np * sizeof (gfloat));
  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<d->nrows_in_plot; i++) {
      k = d->rows_in_plot[i];
      x[n*d->nrows_in_plot + i] = vals[k][j];
    }
  }

  qsort ((void *) x, np, sizeof (gfloat), fcompare);
  dmedian = ((np % 2) != 0) ?  x[(np-1)/2] : (x[np/2-1] + x[np/2])/2. ;

  /*
   * Find the maximum of the sum of squared differences
   * from the mean over all rows
  */

  for (i=0; i<d->nrows_in_plot; i++) {
    sumdist = 0.0;
    for (j=0; j<ncols; j++) {
      dx = (gdouble) vals[d->rows_in_plot[i]][cols[j]] - dmedian;
      sumdist += (dx*dx);
    }
    if (sumdist > lgdist)
      lgdist = sumdist;
  }

  lgdist = sqrt (lgdist);

  g_free ((gchar *) x);
  
  fmedian = (gfloat) dmedian;
  *min = fmedian - lgdist;
  *max = fmedian + lgdist;

  return fmedian;
}

gfloat
mean_largest_dist (gfloat **vals, gint *cols, gint ncols,
  gfloat *min, gfloat *max, datad *d, ggobid *gg)
{
/*
 * Find the minimum and maximum values of each column or variable
 * group scaling by mean and largest distance
*/
  gint i, j;
  gdouble dx, sumxi, mean, sumdist, lgdist = 0.0;

  /*
   * Find the overall mean for the columns
  */
  sumxi = 0.0;
  for (j=0; j<ncols; j++) {
    for (i=0; i<d->nrows_in_plot; i++) {
      dx = (gdouble) vals[d->rows_in_plot[i]][cols[j]];
      sumxi += dx;
    }
  }
  mean = sumxi / (gdouble) d->nrows_in_plot / (gdouble) ncols;

  /*
   * Find the maximum of the sum of squared differences
   * from the mean over all rows
  */

  for (i=0; i<d->nrows_in_plot; i++) {
    sumdist = 0.0;
    for (j=0; j<ncols; j++) {
      dx = (gdouble) vals[d->rows_in_plot[i]][cols[j]] - mean;
      sumdist += (dx*dx);
    }
    if (sumdist > lgdist)
      lgdist = sumdist;
  }

  lgdist = sqrt (lgdist);
  
  *min = mean - lgdist;
  *max = mean + lgdist;

  return mean;
}

void
tform_to_world (datad *d, ggobid *gg)
{
/*
 * Take tform[][], one column at a time, and generate
 * world_data[]
*/
  gint i, j, m;
  gfloat max, min, range, ftmp;
  gfloat precis = PRECISION1;

  for (j=0; j<d->ncols; j++) {

    max = d->vartable[j].lim.max;
    min = d->vartable[j].lim.min;
    range = max - min;

    for (i=0; i<d->nrows_in_plot; i++) {
      m = d->rows_in_plot[i];
      ftmp = -1.0 + 2.0*(d->tform.vals[m][j] - min) / range;
      d->world.vals[m][j] = (glong) (precis * ftmp);

      /* Add in the jitter values */
      d->world.vals[m][j] += d->jitdata.vals[m][j];
    }
  }
}

/*-------------------------------------------------------------------------*/
/*               keeping rows_in_plot up to date                           */
/*-------------------------------------------------------------------------*/

/*
 * Combine the values in two arrays:
 *   included[] (which come from the hide/exclude panel)
 *   sampled[] (which come from the subset panel)
 * to determine which cases should be plotted.
*/

void
rows_in_plot_set (datad *d, ggobid *gg) {
  gint i;

  d->nrows_in_plot = 0;

  for (i=0; i<d->nrows; i++) {
    if (d->included[i] && d->sampled[i]) {
      d->rows_in_plot[d->nrows_in_plot++] = i;
    }
  }
}

/*-------------------------------------------------------------------------*/
/*                     reverse pipeline                                    */
/*-------------------------------------------------------------------------*/

void
world_to_raw_by_var (gint pt, gint var, displayd *display, datad *d, ggobid *gg)
{
  gfloat precis = PRECISION1;
  gfloat ftmp, max, min, rdiff;
  gfloat x;

  if (display->missing_p) {
    max = d->missing_lim.max;
    min = d->missing_lim.min;
  } else {
    max = d->vartable[var].lim.max;
    min = d->vartable[var].lim.min;
  }
  rdiff = max - min;

  ftmp = d->world.vals[pt][var] / precis;
  x = (ftmp + 1.0) * .5 * rdiff;
  x += min;

  d->raw.vals[pt][var] = d->tform.vals[pt][var] = x;
}

  /*
   * allow the reverse pipeline only for
   *   scatterplots in xyplot mode
   *   the splotd members of a scatmat that are xyplots.
  */
void
world_to_raw (gint pt, splotd *sp, datad *d, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;

  if ((display->displaytype == scatterplot && cpanel->projection == XYPLOT) ||
      (display->displaytype == scatmat && sp->p1dvar == -1))
  {
    world_to_raw_by_var (pt, sp->xyvars.x, display, d, gg);
    world_to_raw_by_var (pt, sp->xyvars.y, display, d, gg);
  }
}

