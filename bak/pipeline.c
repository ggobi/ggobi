/* pipeline.c */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"

/* ------------ Dynamic allocation, freeing section --------- */

void
alloc_pipeline_arrays()
/*
 * Dynamically allocate arrays.
*/
{
  gint nc = xg.ncols, nr = xg.nrows;
  register gint i;

  xg.sphered_data = (gfloat **) g_malloc(nr*sizeof(gfloat *));
  for (i=0; i<nr; i++)
    xg.sphered_data[i] = (gfloat *)g_malloc(nc * sizeof(gfloat));

  xg.tform1 = (gfloat **) g_malloc(nr * sizeof(gfloat *));
  for (i=0; i<nr; i++)
    xg.tform1[i] = (gfloat *) g_malloc(nc * sizeof(gfloat));
  xg.tform2 = (gfloat **) g_malloc(nr * sizeof(gfloat *));
  for (i=0; i<nr; i++)
    xg.tform2[i] = (gfloat *) g_malloc(nc * sizeof(gfloat));

  xg.world_data = (glong **) g_malloc(nr * sizeof(glong *));
  for (i=0; i<nr; i++)
    xg.world_data[i] = (glong *) g_malloc(nc * sizeof(glong));

  /* initialize to 0 */
  xg.jitter_data = (glong **) g_malloc0(nr * sizeof(glong *));
  for (i=0; i<nr; i++)
    xg.jitter_data[i] = (glong *) g_malloc0(nc * sizeof(glong));

  xg.rows_in_plot = (gint *) g_malloc(xg.nrows * sizeof(int));

  xg.lim0 = (lims *) g_malloc(nc * sizeof(lims));
  xg.lim = (lims *) g_malloc(nc * sizeof(lims));
  xg.lim_tform = (lims *) g_malloc(nc * sizeof(lims));
  xg.lim_raw = (lims *) g_malloc(nc * sizeof(lims));
}

void
free_pipeline_arrays()
/*
 * Dynamically free arrays used in data pipeline.
*/
{
  gint i;

  for (i=0; i<xg.nrows; i++)
    g_free((gpointer) xg.sphered_data[i]);
  g_free((gpointer) xg.sphered_data);

  for (i=0; i<xg.nrows; i++)
    g_free((gpointer) xg.tform1[i]);
  g_free((gpointer) xg.tform1);
  for (i=0; i<xg.nrows; i++)
    g_free((gpointer) xg.tform2[i]);
  g_free((gpointer) xg.tform2);

  for (i=0; i<xg.nrows; i++)
    g_free((gpointer) xg.world_data[i]);
  g_free((gpointer) xg.world_data);

  for (i=0; i<xg.nrows; i++)
    g_free((gpointer) xg.jitter_data[i]);
  g_free((gpointer) xg.jitter_data);

  g_free((gpointer) xg.rows_in_plot);

  g_free((gpointer) xg.lim0);
  g_free((gpointer) xg.lim);
  g_free((gpointer) xg.lim_tform);
  g_free((gpointer) xg.lim_raw);
}

/* ------------ End of dynamic allocation section --------- */

/* ------------ Data pipeline section --------- */

void
copy_raw_to_tform()
{
  gint i, j;

  for (i=0; i<xg.nrows; i++)
    for (j=0; j<xg.ncols; j++)
      xg.tform1[i][j] = xg.tform2[i][j] = xg.raw_data[i][j];
}

void
copy_raw_to_tform1()
{
  gint i, j;

  for (i=0; i<xg.nrows; i++)
    for (j=0; j<xg.ncols; j++)
      xg.tform1[i][j] = xg.raw_data[i][j];
}

void
copy_tform1_to_tform2()
{
  gint i, j;

  for (j=0; j<xg.ncols; j++) {
    (void) strcpy(xg.collab_tform2[j], xg.collab_tform1[j]);
    for (i=0; i<xg.nrows; i++) {
      xg.tform2[i][j] = xg.tform1[i][j];
    }
  }
}

void
copy_tform_to_sphered()
{
/*
 * Copy tform1 to sphered_data.
*/
  gint i, j;

  for (i=0; i<xg.nrows; i++)
    for (j=0; j<xg.ncols; j++)
      xg.sphered_data[i][j] = xg.tform1[i][j];  /* ?? */
}

void
min_max(gfloat **data, gint *cols, gint ncols, gfloat *min, gfloat *max)
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
icompare(gint *x1, gint *x2)
{
  gint val = 0;

  if (*x1 < *x2)
    val = -1;
  else if (*x1 > *x2)
    val = 1;

  return(val);
}

gfloat
median_largest_dist(gfloat **data, gint *cols, gint ncols,
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
  x = (gfloat *) g_malloc(np * sizeof(gfloat));
  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<xg.nrows_in_plot; i++) {
      k = xg.rows_in_plot[i];
      x[n*xg.nrows_in_plot + i] = data[k][j];
    }
  }

  qsort((void *) x, np, sizeof(gfloat), fcompare);
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

  lgdist = sqrt(lgdist);

  g_free((gchar *) x);
  
  fmedian = (gfloat) dmedian;
  *min = fmedian - lgdist;
  *max = fmedian + lgdist;

  return fmedian;
}

/*
 * This code is also used in the "transpose plots", the logical
 * zooming plots done in identification.
*/
void
adjust_limits(gfloat *min, gfloat *max)
/*
 * This test should be cleverer.  It should test the ratios
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

gfloat
mean_largest_dist(gfloat **data, gint *cols, gint ncols,
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

  lgdist = sqrt(lgdist);
  
  *min = mean - lgdist;
  *max = mean + lgdist;

  return mean;
}

/* This only operates on tour variables; what is it that we
 * really want?
*/

void
mean_lgdist (splotd *sp, gfloat **data)
/*
 * Find the minimum and maximum values of each column or variable
 * group scaling by mean and largest distance.
*/
{
  gint i, j, n;
  gfloat min, max;
  gdouble dx;
  gdouble sumxi;
  gdouble sumdist = 0.0;
  gdouble *mean, lgdist = 0.0;

  /*
   * Find the mean for each column
  */
  mean = (gdouble *) g_malloc (sp->n_tourvars * sizeof(gdouble));

  for (j=0; j<sp->n_tourvars; j++) {
    sumxi = 0.0;
    for (i=0; i<xg.nrows_in_plot; i++) {
      dx = (gdouble) data[xg.rows_in_plot[i]][sp->tourvars[j]];
      sumxi += dx;
    }
    mean[j] = sumxi / xg.nrows_in_plot;
  }

  /*
   * Find the maximum of the sum of squared differences
   * from the column mean over all rows
  */
  for (i=0; i<xg.nrows_in_plot; i++) {
    sumdist = 0.0;
    for (j=0; j<sp->n_tourvars; j++) {
      dx = (data[xg.rows_in_plot[i]][sp->tourvars[j]]-mean[j]);
      dx *= dx;
      sumdist += dx;
    }
    if (sumdist > lgdist)
      lgdist = sumdist;
  }

  lgdist = sqrt(lgdist);
  
  for (j=0; j<sp->n_tourvars; j++) {
    min = mean[j] - lgdist;
    max = mean[j] + lgdist;
    adjust_limits(&min, &max);
    n = sp->tourvars[j];
    xg.lim[n].min = xg.lim0[n].min = min;
    xg.lim[n].max = xg.lim0[n].max = max;
  }

  g_free((gpointer) mean);
}


/*
 * This only gives the correct result if the
 * vgroups vector is of the form {0,1,2,...,ngroups-1}
*/
gint
numvargroups()
{
  gint j, nvgroups = 0;

  for (j=0; j<xg.ncols; j++)
     if (xg.vgroup_ids[j] > nvgroups)
       nvgroups = xg.vgroup_ids[j];
  nvgroups++;

  return(nvgroups);
}

void
init_lim0()
{
  gint j, *cols, ncols;
  gfloat min, max;
  gint k;
  gint nvgroups = numvargroups();

  cols = (gint *) g_malloc(xg.ncols * sizeof(gint));
  for (k=0; k<nvgroups; k++) {
    ncols = 0;
    for (j=0; j<xg.ncols; j++) {
      if (xg.vgroup_ids[j] == k)
        cols[ncols++] = j;
    }

    min_max(xg.tform2, cols, ncols, &min, &max);
    adjust_limits(&min, &max);
    for (j=0; j<ncols; j++) {
      xg.lim0[cols[j]].min = min;
      xg.lim0[cols[j]].max = max;
    }

    min_max(xg.raw_data, cols, ncols, &min, &max);
    adjust_limits(&min, &max);
    for (j=0; j<ncols; j++) {
      xg.lim_raw[cols[j]].min = min;
      xg.lim_raw[cols[j]].max = max;
    }

  }

  g_free((gpointer) cols);
}

void
update_lims()
{
  gint j, k, n;
  gfloat min, max;
  gint *cols, ncols;
  gint nvgroups = numvargroups();
g_printerr("(update_lims) nvgroups %d\n", nvgroups);

  /* 
   * First update the limits for the tform2.  Then 
   * override lim and lim0 if necessary.
  */
  init_lim0();

  /*
   * Take tform2[][], one variable group at a time, and generate
   * the min and max for each variable group (and thus for each
   * column).
  */
  cols = (gint *) g_malloc(xg.ncols * sizeof(gint));
  for (k=0; k<nvgroups; k++) {
    ncols = 0;
    for (j=0; j<xg.ncols; j++) {
      if (xg.vgroup_ids[j] == k)
        cols[ncols++] = j;
    }

    switch(xg.std_type)
    {
      case 0:
        min_max(xg.tform2, cols, ncols, &min, &max);
        break;
      case 1:
        mean_largest_dist(xg.tform2, cols, ncols, &min, &max);
        break;
      case 2:
        median_largest_dist(xg.tform2, cols, ncols, &min, &max);
        break;
    }

    adjust_limits(&min, &max);

    for (n=0; n<ncols; n++) {
      xg.lim[cols[n]].min = min;
      xg.lim[cols[n]].max = max;
    }
  }
  g_free((gpointer) cols);

/*
 * Set the limits that are based exclusively on tform data;
 * they need to be kept separately for parallel coordinate
 * plots.
*/
  for (j=0; j<xg.ncols; j++) {
    xg.lim_tform[j].min = xg.lim[j].min;
    xg.lim_tform[j].max = xg.lim[j].max;
  }
}

void
tform_to_world()
{
/*
 * Take tform2[][], one column at a time, and generate
 * world_data[]
*/
  gint i, j, m;
  gfloat rdiff, ftmp;
  gfloat precis = PRECISION1;  /* 32768 */

  for (j=0; j<xg.ncols; j++) {
    rdiff = xg.lim[j].max - xg.lim[j].min;
    for (i=0; i<xg.nrows_in_plot; i++) {
      m = xg.rows_in_plot[i];
      ftmp = -1.0 + 2.0*(xg.tform2[m][j] - xg.lim[j].min)/rdiff;
      xg.world_data[m][j] = (glong) (precis * ftmp);

      /* Add in the jitter values */
      xg.world_data[m][j] += xg.jitter_data[m][j];
    }
  }
}

void
update_world()
{
  tform_to_world(xg);
}



/*  turned off:  rotation, touring */

