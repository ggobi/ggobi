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
pipeline_arrays_alloc ()
/*
 * Dynamically allocate arrays.
*/
{
  gint nc = xg.ncols, nr = xg.nrows;
  register gint i;

  xg.tform1 = (gfloat **) g_malloc (nr * sizeof (gfloat *));
  xg.tform2 = (gfloat **) g_malloc (nr * sizeof (gfloat *));
  for (i=0; i<nr; i++) {
    xg.tform1[i] = (gfloat *) g_malloc0 (nc * sizeof (gfloat));
    xg.tform2[i] = (gfloat *) g_malloc0 (nc * sizeof (gfloat));
  }

  xg.world_data = (glong **) g_malloc (nr * sizeof (glong *));
  xg.jitter_data = (glong **) g_malloc (nr * sizeof (glong *));
  for (i=0; i<nr; i++) {
    xg.world_data[i] = (glong *) g_malloc (nc * sizeof (glong));
    xg.jitter_data[i] = (glong *) g_malloc0 (nc * sizeof (glong));
  }

  xg.rows_in_plot = (gint *) g_malloc (xg.nrows * sizeof (gint));
}

void
pipeline_arrays_add_column (gint jvar)
/*
 * Reallocate pipeline arrays to contain one more column, and
 * copy column jvar into the new column
*/
{
  gint nc = xg.ncols + 1, nr = xg.nrows;
  register gint i;

  for (i=0; i<nr; i++) {
    xg.raw_data[i] = (gfloat *) g_realloc ((gpointer) xg.raw_data[i],
      nc * sizeof (gfloat));
    xg.raw_data[i][nc-1] = xg.raw_data[i][jvar];

    xg.tform1[i] = (gfloat *) g_realloc ((gpointer) xg.tform1[i],
      nc * sizeof (gfloat));
    xg.tform1[i][nc-1] = xg.tform1[i][jvar];

    xg.tform2[i] = (gfloat *) g_realloc ((gpointer) xg.tform2[i],
      nc * sizeof (gfloat));
    xg.tform2[i][nc-1] = xg.tform2[i][jvar];

    xg.world_data[i] = (glong *) g_realloc ((gpointer) xg.world_data[i],
      nc * sizeof (glong));
    xg.world_data[i][nc-1] = xg.world_data[i][jvar];

    xg.jitter_data[i] = (glong *) g_realloc ((gpointer) xg.jitter_data[i],
      nc * sizeof (glong));
    xg.jitter_data[i][nc-1] = xg.jitter_data[i][jvar];
  }

}

void
pipeline_arrays_free ()
/*
 * Dynamically free arrays used in data pipeline.
*/
{
  gint i;

  for (i=0; i<xg.nrows; i++)
    g_free ((gpointer) xg.tform1[i]);
  g_free ((gpointer) xg.tform1);
  for (i=0; i<xg.nrows; i++)
    g_free ((gpointer) xg.tform2[i]);
  g_free ((gpointer) xg.tform2);

  for (i=0; i<xg.nrows; i++)
    g_free ((gpointer) xg.world_data[i]);
  g_free ((gpointer) xg.world_data);

  for (i=0; i<xg.nrows; i++)
    g_free ((gpointer) xg.jitter_data[i]);
  g_free ((gpointer) xg.jitter_data);

  g_free ((gpointer) xg.rows_in_plot);
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

  cols = (gint *) g_malloc (xg.ncols * sizeof (gint));
  for (k=0; k<nvgr; k++) {
    ncols = 0;
    for (j=0; j<xg.ncols; j++) {
      if (xg.vardata[j].groupid == k)
        cols[ncols++] = j;
    }

    min_max (xg.raw_data, cols, ncols, &min, &max);
    limits_adjust (&min, &max);
    for (j=0; j<ncols; j++) {
      xg.vardata[cols[j]].lim_raw_gp.min = min;
      xg.vardata[cols[j]].lim_raw_gp.max = max;
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

  cols = (gint *) g_malloc (xg.ncols * sizeof (gint));
  for (k=0; k<nvgr; k++) {
    ncols = 0;
    for (j=0; j<xg.ncols; j++) {
      if (xg.vardata[j].groupid == k)
        cols[ncols++] = j;
    }

    min_max (xg.tform2, cols, ncols, &min, &max);
    limits_adjust (&min, &max);
    for (n=0; n<ncols; n++) {
      xg.vardata[cols[n]].lim_tform_gp.min = min;
      xg.vardata[cols[n]].lim_tform_gp.max = max;
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
  cols = (gint *) g_malloc (xg.ncols * sizeof (gint));
  for (k=0; k<nvgr; k++) {
    ncols = 0;
    for (j=0; j<xg.ncols; j++) {
      if (xg.vardata[j].groupid == k)
        cols[ncols++] = j;
    }

    switch (xg.std_type)
    {
      case 0:
        /*-- isn't this already done? --*/
/*      min_max (xg.tform2, cols, ncols, &min, &max);*/
        min = xg.vardata[cols[0]].lim_tform_gp.min;
        max = xg.vardata[cols[0]].lim_tform_gp.max;
        break;
      case 1:
        mean_largest_dist (xg.tform2, cols, ncols, &min, &max);
        break;
      case 2:
        median_largest_dist (xg.tform2, cols, ncols, &min, &max);
        break;
    }

    limits_adjust (&min, &max);

    for (n=0; n<ncols; n++) {
      xg.vardata[cols[n]].lim.min = min;
      xg.vardata[cols[n]].lim.max = max;
    }
  }
  g_free ((gpointer) cols);
}

/*-------------------------------------------------------------------------*/
/*                       pipeline                                          */
/*-------------------------------------------------------------------------*/

void
raw_to_tform_copy ()
{
  gint i, j;

  for (i=0; i<xg.nrows; i++)
    for (j=0; j<xg.ncols; j++)
      xg.tform1[i][j] = xg.tform2[i][j] = xg.raw_data[i][j];
}

void
raw_to_tform1_copy ()
{
  gint i, j;

  for (j=0; j<xg.ncols; j++) {
    for (i=0; i<xg.nrows; i++)
      xg.tform1[i][j] = xg.raw_data[i][j];
  }
}

void
tform1_to_tform2_copy ()
{
  gint i, j;

  for (j=0; j<xg.ncols; j++) {
    for (i=0; i<xg.nrows; i++) {
      xg.tform2[i][j] = xg.tform1[i][j];
    }
  }
}

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
  *min = *max = data[xg.rows_in_plot[0]][cols[0]];

  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<xg.nrows_in_plot; i++) {
      k = xg.rows_in_plot[i];
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

  np = ncols * xg.nrows_in_plot;
  x = (gfloat *) g_malloc (np * sizeof (gfloat));
  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<xg.nrows_in_plot; i++) {
      k = xg.rows_in_plot[i];
      x[n*xg.nrows_in_plot + i] = data[k][j];
    }
  }

  qsort ((void *) x, np, sizeof (gfloat), fcompare);
  dmedian = ((np % 2) != 0) ?  x[(np-1)/2] : (x[np/2-1] + x[np/2])/2. ;

  /*
   * Find the maximum of the sum of squared differences
   * from the mean over all rows
  */

  for (i=0; i<xg.nrows_in_plot; i++) {
    sumdist = 0.0;
    for (j=0; j<ncols; j++) {
      dx = (gdouble) data[xg.rows_in_plot[i]][cols[j]] - dmedian;
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
    for (i=0; i<xg.nrows_in_plot; i++) {
      dx = (gdouble) data[xg.rows_in_plot[i]][cols[j]];
      sumxi += dx;
    }
  }
  mean = sumxi / (gdouble) xg.nrows_in_plot / (gdouble) ncols;

  /*
   * Find the maximum of the sum of squared differences
   * from the mean over all rows
  */

  for (i=0; i<xg.nrows_in_plot; i++) {
    sumdist = 0.0;
    for (j=0; j<ncols; j++) {
      dx = (gdouble) data[xg.rows_in_plot[i]][cols[j]] - mean;
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

  for (j=0; j<xg.ncols; j++) {

    max = xg.vardata[j].lim.max;
    min = xg.vardata[j].lim.min;
    range = max - min;

    for (i=0; i<xg.nrows_in_plot; i++) {
      m = xg.rows_in_plot[i];
      ftmp = -1.0 + 2.0*(xg.tform2[m][j] - min) / range;
      xg.world_data[m][j] = (glong) (precis * ftmp);

      /* Add in the jitter values */
      xg.world_data[m][j] += xg.jitter_data[m][j];
    }
  }
}

