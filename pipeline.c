/* pipeline.c */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static gfloat mean_largest_dist (gfloat **, gint *, gint, gfloat *, gfloat *, datad *, ggobid *);
static gfloat median_largest_dist (gfloat **, gint *, gint, gfloat *, gfloat *, datad *, ggobid *);
static void min_max (gfloat **, gint *, gint, gfloat *, gfloat *, datad *, ggobid *);

/* ------------ Dynamic allocation, freeing section --------- */

void
pipeline_arrays_free (datad *d, ggobid *gg)
/*
 * Dynamically free arrays used in data pipeline.
*/
{
  arrayf_free (&d->tform1, 0, 0);
  arrayf_free (&d->tform2, 0, 0);

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

  if (d->tform1.vals != NULL) pipeline_arrays_free (d, gg);

  arrayf_alloc (&d->tform1, nr, nc);
  arrayf_alloc (&d->tform2, nr, nc);

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
  arrayf_add_cols (&d->tform1, nc);
  arrayf_add_cols (&d->tform2, nc);

  arrayl_add_cols (&d->world, nc);
  arrayl_add_cols (&d->jitdata, nc);

  for (i=0; i<nr; i++) {
    d->raw.vals[i][nc-1] = d->tform1.vals[i][nc-1] =
      d->tform2.vals[i][nc-1] = d->raw.vals[i][jvar];  /*-- no tform --*/
    d->jitdata.vals[i][nc-1] = 0;  /*-- no jitter --*/
  }

  /*-- world data is not populated --*/
}


/*-------------------------------------------------------------------------*/
/*               mucking about with the variable limits                    */
/*-------------------------------------------------------------------------*/

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
vartable_lim_raw_gp_set (datad *d, ggobid *gg)
{
  gint j, *cols, ncols;
  gfloat min, max;
  gint k;
  gint nvgr = nvgroups (d, gg);

  cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  for (k=0; k<nvgr; k++) {
    ncols = 0;
    for (j=0; j<d->ncols; j++) {
      if (d->vartable[j].groupid == k)
        cols[ncols++] = j;
      else
        cols[j] = j;
    }

    min_max (d->raw.vals, cols, ncols, &min, &max, d, gg);
    limits_adjust (&min, &max);
    for (j=0; j<ncols; j++) {
      d->vartable[cols[j]].lim_raw_gp.min = min;
      d->vartable[cols[j]].lim_raw_gp.max = max;
    }
  }

  g_free ((gpointer) cols);
}

void
vartable_lim_tform_gp_set (datad *d, ggobid *gg)
{
  gint j, n, *cols, ncols;
  gfloat min, max;
  gint k;
  gint nvgr = nvgroups (d, gg);

  cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  for (k=0; k<nvgr; k++) {
    ncols = 0;
    for (j=0; j<d->ncols; j++) {
      if (d->vartable[j].groupid == k)
        cols[ncols++] = j;
    }

    min_max (d->tform2.vals, cols, ncols, &min, &max, d, gg);
    limits_adjust (&min, &max);
    for (n=0; n<ncols; n++) {
      d->vartable[cols[n]].lim_tform_gp.min = min;
      d->vartable[cols[n]].lim_tform_gp.max = max;
    }
  }

  g_free ((gpointer) cols);
}

void
vartable_lim_update (datad *d, ggobid *gg)
{
  gint j, k, n;
  gfloat min, max;
  gint *cols, ncols;
  gint nvgr = nvgroups (d, gg);

  /* 
   * First update the limits taken from the tform2 data. 
  */
  vartable_lim_tform_gp_set (d, gg);

  /*
   * Take tform2[][], one variable group at a time, and generate
   * the min and max for each variable group (and thus for each
   * column).
  */
  cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  for (k=0; k<nvgr; k++) {
    ncols = 0;
    for (j=0; j<d->ncols; j++) {
      if (d->vartable[j].groupid == k)
        cols[ncols++] = j;
    }

    switch (d->std_type)
    {
      case 0:
        /*-- isn't this already done? --*/
/*      min_max (gg->tform2, cols, ncols, &min, &max);*/
        min = d->vartable[cols[0]].lim_tform_gp.min;
        max = d->vartable[cols[0]].lim_tform_gp.max;
        break;
      case 1:
        mean_largest_dist (d->tform2.vals, cols, ncols, &min, &max, d, gg);
        break;
      case 2:
        median_largest_dist (d->tform2.vals, cols, ncols, &min, &max, d, gg);
        break;
    }

    limits_adjust (&min, &max);

    for (n=0; n<ncols; n++) {
      d->vartable[cols[n]].lim.min = min;
      d->vartable[cols[n]].lim.max = max;
    }
  }
  g_free ((gpointer) cols);
}

/*-------------------------------------------------------------------------*/
/*                       pipeline                                          */
/*-------------------------------------------------------------------------*/

void
min_max (gfloat **vals, gint *cols, gint ncols, gfloat *min, gfloat *max,
  datad *d, ggobid *gg)
/*
 * Find the minimum and maximum values of each column or variable
 * group using using the min-max scaling.
*/
{
  int i, j, k, n;
/*
 * Choose an initial value for *min and *max
*/
  *min = *max = vals[d->rows_in_plot[0]][cols[0]];

  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<d->nrows_in_plot; i++) {
      k = d->rows_in_plot[i];
      if (vals[k][j] < *min)
        *min = vals[k][j];
      else if (vals[k][j] > *max)
        *max = vals[k][j];
    }
  }
}

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
 * Take tform2[][], one column at a time, and generate
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
      ftmp = -1.0 + 2.0*(d->tform2.vals[m][j] - min) / range;
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

  d->raw.vals[pt][var] =
    d->tform1.vals[pt][var] =
    d->tform2.vals[pt][var] = x;
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

