/* pipeline.c */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static gfloat mean_largest_dist (gfloat **, gint *, gint, gfloat *, gfloat *);
static gfloat median_largest_dist (gfloat **, gint *, gint, gfloat *, gfloat *);
static void min_max (gfloat **, gint *, gint, gfloat *, gfloat *);

/* ------------ Dynamic allocation, freeing section --------- */

void
pipeline_arrays_free ()
/*
 * Dynamically free arrays used in data pipeline.
*/
{
  arrayf_free (&gg.tform1, 0, 0);
  arrayf_free (&gg.tform2, 0, 0);

  arrayl_free (&gg.world, 0, 0);
  arrayl_free (&gg.jitter, 0, 0);

  g_free ((gpointer) gg.rows_in_plot);
  g_free ((gpointer) gg.sampled);
}

void
pipeline_arrays_alloc ()
/*
 * Dynamically allocate arrays.
*/
{
  gint nc = gg.ncols, nr = gg.nrows;

  if (gg.tform1.data != NULL) pipeline_arrays_free ();

  arrayf_alloc (&gg.tform1, nr, nc);
  arrayf_alloc (&gg.tform2, nr, nc);

  arrayl_alloc (&gg.world, nr, nc);
  arrayl_alloc_zero (&gg.jitter, nr, nc);

  gg.rows_in_plot = (gint *) g_malloc (nr * sizeof (gint));
  gg.sampled = (gboolean *) g_malloc (nr * sizeof (gboolean));
}

void
pipeline_arrays_add_column (gint jvar)
/*
 * Reallocate pipeline arrays to contain one more column, and
 * copy column jvar into the new column
*/
{
  gint nc = gg.ncols + 1, nr = gg.nrows;
  register gint i;

  arrayf_add_cols (&gg.raw, nc);
  arrayf_add_cols (&gg.tform1, nc);
  arrayf_add_cols (&gg.tform2, nc);

  arrayl_add_cols (&gg.world, nc);
  arrayl_add_cols (&gg.jitter, nc);

  for (i=0; i<nr; i++) {
    gg.raw.data[i][nc-1] = gg.raw.data[i][jvar];

    gg.tform1.data[i][nc-1] = gg.tform1.data[i][jvar];
    gg.tform2.data[i][nc-1] = gg.tform2.data[i][jvar];

    gg.world.data[i][nc-1] = gg.world.data[i][jvar];
    gg.jitter.data[i][nc-1] = gg.jitter.data[i][jvar];
  }
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
vardata_lim_raw_gp_set ()
{
  gint j, *cols, ncols;
  gfloat min, max;
  gint k;
  gint nvgr = nvgroups ();

  cols = (gint *) g_malloc (gg.ncols * sizeof (gint));
  for (k=0; k<nvgr; k++) {
    ncols = 0;
    for (j=0; j<gg.ncols; j++) {
      if (gg.vardata[j].groupid == k)
        cols[ncols++] = j;
      else
        cols[j] = j;
    }

    min_max (gg.raw.data, cols, ncols, &min, &max);
    limits_adjust (&min, &max);
    for (j=0; j<ncols; j++) {
      gg.vardata[cols[j]].lim_raw_gp.min = min;
      gg.vardata[cols[j]].lim_raw_gp.max = max;
    }
  }

  g_free ((gpointer) cols);
}

void
vardata_lim_tform_gp_set ()
{
  gint j, n, *cols, ncols;
  gfloat min, max;
  gint k;
  gint nvgr = nvgroups ();

  cols = (gint *) g_malloc (gg.ncols * sizeof (gint));
  for (k=0; k<nvgr; k++) {
    ncols = 0;
    for (j=0; j<gg.ncols; j++) {
      if (gg.vardata[j].groupid == k)
        cols[ncols++] = j;
    }

    min_max (gg.tform2.data, cols, ncols, &min, &max);
    limits_adjust (&min, &max);
    for (n=0; n<ncols; n++) {
      gg.vardata[cols[n]].lim_tform_gp.min = min;
      gg.vardata[cols[n]].lim_tform_gp.max = max;
    }
  }

  g_free ((gpointer) cols);
}

void
vardata_lim_update ()
{
  gint j, k, n;
  gfloat min, max;
  gint *cols, ncols;
  gint nvgr = nvgroups ();

  /* 
   * First update the limits taken from the tform2 data. 
  */
  vardata_lim_tform_gp_set ();

  /*
   * Take tform2[][], one variable group at a time, and generate
   * the min and max for each variable group (and thus for each
   * column).
  */
  cols = (gint *) g_malloc (gg.ncols * sizeof (gint));
  for (k=0; k<nvgr; k++) {
    ncols = 0;
    for (j=0; j<gg.ncols; j++) {
      if (gg.vardata[j].groupid == k)
        cols[ncols++] = j;
    }

    switch (gg.std_type)
    {
      case 0:
        /*-- isn't this already done? --*/
/*      min_max (gg.tform2, cols, ncols, &min, &max);*/
        min = gg.vardata[cols[0]].lim_tform_gp.min;
        max = gg.vardata[cols[0]].lim_tform_gp.max;
        break;
      case 1:
        mean_largest_dist (gg.tform2.data, cols, ncols, &min, &max);
        break;
      case 2:
        median_largest_dist (gg.tform2.data, cols, ncols, &min, &max);
        break;
    }

    limits_adjust (&min, &max);

    for (n=0; n<ncols; n++) {
      gg.vardata[cols[n]].lim.min = min;
      gg.vardata[cols[n]].lim.max = max;
    }
  }
  g_free ((gpointer) cols);
}

/*-------------------------------------------------------------------------*/
/*                       pipeline                                          */
/*-------------------------------------------------------------------------*/

void
min_max (gfloat **data, gint *cols, gint ncols, gfloat *min, gfloat *max)
/*
 * Find the minimum and maximum values of each column or variable
 * group using using the min-max scaling.
*/
{
  int i, j, k, n;
/*
 * Choose an initial value for *min and *max
*/
  *min = *max = data[gg.rows_in_plot[0]][cols[0]];

  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<gg.nrows_in_plot; i++) {
      k = gg.rows_in_plot[i];
      if (data[k][j] < *min)
        *min = data[k][j];
      else if (data[k][j] > *max)
        *max = data[k][j];
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
median_largest_dist (gfloat **data, gint *cols, gint ncols,
  gfloat *min, gfloat *max)
{
/*
 * Find the minimum and maximum values of each column or variable
 * group scaling by median and largest distance
*/
  gint i, j, k, n, np;
  gdouble dx, sumdist, lgdist = 0.0;
  gfloat *x, fmedian;
  gdouble dmedian = 0;
  extern gint fcompare (const void *, const void *);

  np = ncols * gg.nrows_in_plot;
  x = (gfloat *) g_malloc (np * sizeof (gfloat));
  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<gg.nrows_in_plot; i++) {
      k = gg.rows_in_plot[i];
      x[n*gg.nrows_in_plot + i] = data[k][j];
    }
  }

  qsort ((void *) x, np, sizeof (gfloat), fcompare);
  dmedian = ((np % 2) != 0) ?  x[(np-1)/2] : (x[np/2-1] + x[np/2])/2. ;

  /*
   * Find the maximum of the sum of squared differences
   * from the mean over all rows
  */

  for (i=0; i<gg.nrows_in_plot; i++) {
    sumdist = 0.0;
    for (j=0; j<ncols; j++) {
      dx = (gdouble) data[gg.rows_in_plot[i]][cols[j]] - dmedian;
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
mean_largest_dist (gfloat **data, gint *cols, gint ncols,
  gfloat *min, gfloat *max)
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
    for (i=0; i<gg.nrows_in_plot; i++) {
      dx = (gdouble) data[gg.rows_in_plot[i]][cols[j]];
      sumxi += dx;
    }
  }
  mean = sumxi / (gdouble) gg.nrows_in_plot / (gdouble) ncols;

  /*
   * Find the maximum of the sum of squared differences
   * from the mean over all rows
  */

  for (i=0; i<gg.nrows_in_plot; i++) {
    sumdist = 0.0;
    for (j=0; j<ncols; j++) {
      dx = (gdouble) data[gg.rows_in_plot[i]][cols[j]] - mean;
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
tform_to_world ()
{
/*
 * Take tform2[][], one column at a time, and generate
 * world_data[]
*/
  gint i, j, m;
  gfloat max, min, range, ftmp;
  gfloat precis = PRECISION1;

  for (j=0; j<gg.ncols; j++) {

    max = gg.vardata[j].lim.max;
    min = gg.vardata[j].lim.min;
    range = max - min;

    for (i=0; i<gg.nrows_in_plot; i++) {
      m = gg.rows_in_plot[i];
      ftmp = -1.0 + 2.0*(gg.tform2.data[m][j] - min) / range;
      gg.world.data[m][j] = (glong) (precis * ftmp);

      /* Add in the jitter values */
      gg.world.data[m][j] += gg.jitter.data[m][j];
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
rows_in_plot_set (ggobid *gg) {
  gint i;

  gg->nrows_in_plot = 0;

  for (i=0; i<gg->nrows; i++) {
    if (gg->included[i] && gg->sampled[i]) {
      gg->rows_in_plot[gg->nrows_in_plot++] = i;
    }
  }
}

