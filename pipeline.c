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

gfloat mean_largest_dist (gfloat **, gint *, gint, gfloat *, gfloat *, datad *, ggobid *);
gfloat median_largest_dist (gfloat **, gint *, gint, gfloat *, gfloat *, datad *, ggobid *);

/* ------------ Dynamic allocation, freeing section --------- */

void
pipeline_arrays_free (datad *d, ggobid *gg)
/*
 * Dynamically free arrays used in data pipeline.
*/
{
  arrayf_free (&d->tform, 0, 0);

  arrayg_free (&d->world, 0, 0);
  arrayg_free (&d->jitdata, 0, 0);

  /*-- should these be freed here as well? --*/
  vectori_free (&d->clusterid);

  vectori_free (&d->rows_in_plot);
  vectorb_free (&d->sampled);
  vectorb_free (&d->excluded);
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

  arrayg_alloc (&d->world, nr, nc);
  arrayg_alloc_zero (&d->jitdata, nr, nc);

  vectori_alloc (&d->rows_in_plot, nr);
  vectorb_alloc (&d->sampled, nr);
  vectorb_alloc (&d->excluded, nr);
}

void
pipeline_arrays_check_dimensions (datad *d)
{
  gint n;

  /*-- d->raw --*/
  if (d->raw.ncols < d->ncols)
    arrayf_add_cols (&d->raw, d->ncols);
  if (d->raw.nrows < d->nrows)
    arrayf_add_rows (&d->raw, d->nrows);

  /*-- d->tform --*/
  if (d->tform.ncols < d->ncols)
    arrayf_add_cols (&d->tform, d->ncols);
  if (d->tform.nrows < d->nrows)
    arrayf_add_rows (&d->tform, d->nrows);

  /*-- d->world --*/
  if (d->world.ncols < d->ncols)
    arrayg_add_cols (&d->world, d->ncols);
  if (d->world.nrows < d->nrows)
    arrayg_add_rows (&d->world, d->nrows);

  /*-- d->jitdata --*/
  if (d->jitdata.ncols < d->ncols) {
    gint i, j, nc = d->jitdata.ncols;
    arrayg_add_cols (&d->jitdata, d->ncols);
    for (j=nc; j<d->ncols; j++) {
      for (i=0; i<d->nrows; i++)
        d->jitdata.vals[i][j] = 0;
    }
  }
  if (d->jitdata.nrows < d->nrows)
    arrayg_add_rows (&d->jitdata, d->nrows);

  if ((n = d->sampled.nels) < d->nrows) {
    gint i;
    /*-- include any new rows in the sample -- add to rows_in_plot? --*/
    vectorb_realloc (&d->sampled, d->nrows);
    for (i=n; i<d->nrows; i++)
      d->sampled.els[i] = true;
  }

  if ((n = d->excluded.nels) < d->nrows) {
    gint i;
    /*-- don't excluded new rows --*/
    vectorb_realloc (&d->excluded, d->nrows);
    for (i=n; i<d->nrows; i++)
      d->excluded.els[i] = false;
  }

  /*-- d->rows_in_plot --*/
  if (d->rows_in_plot.nels < d->nrows)
    vectori_realloc (&d->rows_in_plot, d->nrows);
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
 * Find the minimum and maximum values of each variable,
 * scaling by median and largest distance
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
      k = d->rows_in_plot.els[i];
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
      dx = (gdouble) vals[d->rows_in_plot.els[i]][cols[j]] - dmedian;
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
 * Find the minimum and maximum values of each variable,
 * scaling by mean and largest distance
*/
  gint i, j;
  gdouble dx, sumxi, mean, sumdist, lgdist = 0.0;

  /*
   * Find the overall mean for the columns
  */
  sumxi = 0.0;
  for (j=0; j<ncols; j++) {
    for (i=0; i<d->nrows_in_plot; i++) {
      dx = (gdouble) vals[d->rows_in_plot.els[i]][cols[j]];
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
      dx = (gdouble) vals[d->rows_in_plot.els[i]][cols[j]] - mean;
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
tform_to_world_by_var (gint j, datad *d, ggobid *gg)
{
  gint i, m;
  greal max, min, range, ftmp;
  gfloat precis = PRECISION1;
  vartabled *vt = vartable_element_get (j, d);

  pipeline_arrays_check_dimensions (d);  /*-- realloc as necessary --*/

/*
 * This is excluding missings -- which we don't want for
 * scaling, but we do want for display, so some more thought
 * is needed, or maybe another set of limits.
*/
  max = (greal) vt->lim.max;
  min = (greal) vt->lim.min;
  range = max - min;

  for (i=0; i<d->nrows_in_plot; i++) {
    m = d->rows_in_plot.els[i];
    ftmp = -1.0 + 2.0*((greal)d->tform.vals[m][j] - min) / range;
    d->world.vals[m][j] = (greal) (precis * ftmp);

    /* Add in the jitter values */
    d->world.vals[m][j] += d->jitdata.vals[m][j];
  }
}

void
tform_to_world (datad *d, ggobid *gg)
{
/*
 * Take tform.vals[][], one column at a time, and generate world[]
*/
  gint j;

  for (j=0; j<d->ncols; j++)
    tform_to_world_by_var (j, d, gg);
}

/*-------------------------------------------------------------------------*/
/*               keeping rows_in_plot up to date                           */
/*-------------------------------------------------------------------------*/

/*
 * Combine the values in two arrays:
 *   excluded[] (which comes from the exclusion panel or from linking)
 *   sampled[] (which come from the subset panel)
 * to determine which cases should be plotted.
 *
 * rows_in_plot = sampled && !excluded
*/

void
rows_in_plot_set (datad *d, ggobid *gg) {
  gint i;
  GtkGGobiDataClass *klass;
  gint nprev = d->nrows_in_plot;

  d->nrows_in_plot = 0;

  for (i=0; i<d->nrows; i++)
    if (d->sampled.els[i] && !d->excluded.els[i])
      d->rows_in_plot.els[d->nrows_in_plot++] = i;

  klass = GTK_GGOBI_DATA_CLASS(GTK_OBJECT(d)->klass);
  gtk_signal_emit (GTK_OBJECT(d),
    klass->signals[ROWS_IN_PLOT_CHANGED_SIGNAL], 
    nprev, -1, gg);  /* the argument shown with -1 has no current use */

  return; /* (nprev == d->nrows_in_plot); */
}

/*-------------------------------------------------------------------------*/
/*                     reverse pipeline                                    */
/*-------------------------------------------------------------------------*/

/* XXX duncan and dfs: you need to sort this out
void
world_to_raw_by_var (gint pt, gint j, displayd *display, datad *d, ggobid *gg)
{
  gfloat precis = PRECISION1;
  gfloat ftmp, rdiff;
  gfloat x;
  vartabled *vt = vartable_element_get (j, d);

  rdiff = vt->lim.max - vt->lim.min;

  ftmp = (gfloat) (d->world.vals[pt][j] - d->jitdata.vals[pt][j]) / precis;
  x = (ftmp + 1.0) * .5 * rdiff;
  x += vt->lim.min;

  d->raw.vals[pt][j] = d->tform.vals[pt][j] = x;
}
*/

/*
 * allow the reverse pipeline only for
 *   scatterplots in xyplot mode
 *   the splotd members of a scatmat that are xyplots.
*/
/* XXX duncan and dfs: you need to sort this out
void
world_to_raw (gint pt, splotd *sp, datad *d, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;

  if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
     GtkGGobiExtendedDisplayClass *klass;
     klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
     if(klass->world_to_raw)
         klass->world_to_raw(display, sp, pt, d, gg);
  } 
}
*/
