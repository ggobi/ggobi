/*-- pipeline.c --*/
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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

gfloat mean_largest_dist (gfloat **, gint *, gint, gfloat *, gfloat *,
                          GGobiData *, ggobid *);
gfloat median_largest_dist (gfloat **, gint *, gint, gfloat *, gfloat *,
                            GGobiData *, ggobid *);

/* ------------ Dynamic allocation, freeing section --------- */

void
pipeline_init (GGobiData * d, ggobid * gg)
{
  gint i;

  /*-- a handful of allocations and initializations --*/
  pipeline_arrays_alloc (d, gg);
  for (i = 0; i < d->nrows; i++) {
    d->sampled.els[i] = true;
    d->excluded.els[i] = false;
  }
  /*-- maybe some points are tagged "hidden" in the data --*/
  rows_in_plot_set (d, gg);

  /*-- some initializations --*/
  edgeedit_init (gg);
  brush_init (d, gg);


  /*-- run the first half of the pipeline --*/
  arrayf_copy (&d->raw, &d->tform);

  limits_set (d, true, true, gg->lims_use_visible);

  vartable_limits_set (d);  /*-- does this do something here?  --*/
  vartable_stats_set (d);  /*-- does this do something here?  --*/

  /*
   * If there are missings, they've been initialized with a value
   * of 0.  Here, re-set that value to 15% below the minimum for each
   * variable.  (dfs -- done at Di's request, September 2004)
   */

  if (ggobi_data_has_missings(d)) {
    gint j;
    gint *vars = (gint *) g_malloc(d->ncols * sizeof(gint));
    for (j = 0; j < d->ncols; j++) vars[j] = j;

    impute_fixed (IMP_BELOW, 15.0, d->ncols, vars, d, gg);
    limits_set (d, true, true, gg->lims_use_visible);
    vartable_limits_set (d);
    vartable_stats_set (d);
    g_free(vars);
  }

  tform_to_world (d, gg);
}

/*
 * Dynamically free arrays used in data pipeline.
*/
void
pipeline_arrays_free (GGobiData * d, ggobid * gg)
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

/*
 * Dynamically allocate arrays.
*/
void
pipeline_arrays_alloc (GGobiData * d, ggobid * gg)
{
  gint nc = d->ncols, nr = d->nrows;

  if (d->tform.vals != NULL)
    pipeline_arrays_free (d, gg);

  arrayf_alloc (&d->tform, nr, nc);

  arrayg_alloc (&d->world, nr, nc);
  arrayg_alloc_zero (&d->jitdata, nr, nc);

  vectori_alloc (&d->rows_in_plot, nr);
  vectorb_alloc (&d->sampled, nr);
  vectorb_alloc (&d->excluded, nr);
}

void
pipeline_arrays_check_dimensions (GGobiData * d)
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
    for (j = nc; j < d->ncols; j++) {
      for (i = 0; i < d->nrows; i++)
        d->jitdata.vals[i][j] = 0;
    }
  }
  if (d->jitdata.nrows < d->nrows)
    arrayg_add_rows (&d->jitdata, d->nrows);

  if ((n = d->sampled.nels) < d->nrows) {
    gint i;
    /*-- include any new rows in the sample -- add to rows_in_plot? --*/
    vectorb_realloc (&d->sampled, d->nrows);
    for (i = n; i < d->nrows; i++)
      d->sampled.els[i] = true;
  }

  if ((n = d->excluded.nels) < d->nrows) {
    gint i;
    /*-- don't excluded new rows --*/
    vectorb_realloc (&d->excluded, d->nrows);
    for (i = n; i < d->nrows; i++)
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
icompare (gint * x1, gint * x2)
{
  gint val = 0;

  if (*x1 < *x2)
    val = -1;
  else if (*x1 > *x2)
    val = 1;

  return (val);
}

gfloat
median_largest_dist (gfloat ** vals, gint * cols, gint ncols,
                     gfloat * min, gfloat * max, GGobiData * d, ggobid * gg)
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
  for (n = 0; n < ncols; n++) {
    j = cols[n];
    for (i = 0; i < d->nrows_in_plot; i++) {
      k = d->rows_in_plot.els[i];
      x[n * d->nrows_in_plot + i] = vals[k][j];
    }
  }

  qsort ((void *) x, np, sizeof (gfloat), fcompare);
  dmedian =
    ((np % 2) != 0) ? x[(np - 1) / 2] : (x[np / 2 - 1] + x[np / 2]) / 2.;

  /*
   * Find the maximum of the sum of squared differences
   * from the mean over all rows
   */

  for (i = 0; i < d->nrows_in_plot; i++) {
    sumdist = 0.0;
    for (j = 0; j < ncols; j++) {
      dx = (gdouble) vals[d->rows_in_plot.els[i]][cols[j]] - dmedian;
      sumdist += (dx * dx);
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
mean_largest_dist (gfloat ** vals, gint * cols, gint ncols,
                   gfloat * min, gfloat * max, GGobiData * d, ggobid * gg)
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
  for (j = 0; j < ncols; j++) {
    for (i = 0; i < d->nrows_in_plot; i++) {
      dx = (gdouble) vals[d->rows_in_plot.els[i]][cols[j]];
      sumxi += dx;
    }
  }
  mean = sumxi / (gdouble) d->nrows_in_plot / (gdouble) ncols;

  /*
   * Find the maximum of the sum of squared differences
   * from the mean over all rows
   */

  for (i = 0; i < d->nrows_in_plot; i++) {
    sumdist = 0.0;
    for (j = 0; j < ncols; j++) {
      dx = (gdouble) vals[d->rows_in_plot.els[i]][cols[j]] - mean;
      sumdist += (dx * dx);
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
tform_to_world_by_var (gint j, GGobiData * d, ggobid * gg)
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

  for (i = 0; i < d->nrows_in_plot; i++) {
    m = d->rows_in_plot.els[i];
    ftmp = -1.0 + 2.0 * ((greal) d->tform.vals[m][j] - min) / range;
    d->world.vals[m][j] = (greal) (precis * ftmp);

    /* Add in the jitter values */
    d->world.vals[m][j] += d->jitdata.vals[m][j];
  }
}

void
tform_to_world (GGobiData * d, ggobid * gg)
{
/*
 * Take tform.vals[][], one column at a time, and generate world[]
*/
  gint j;

  for (j = 0; j < d->ncols; j++)
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
rows_in_plot_set (GGobiData * d, ggobid * gg)
{
  gint i;
  GGobiDataClass *klass;
  gint nprev = d->nrows_in_plot;

  d->nrows_in_plot = 0;

  for (i = 0; i < d->nrows; i++)
    if (d->sampled.els[i] && !d->excluded.els[i])
      d->rows_in_plot.els[d->nrows_in_plot++] = i;

  klass = GGOBI_DATA_GET_CLASS (d);
  g_signal_emit_by_name (G_OBJECT (d), "rows-in-plot-changed", 0, nprev, -1, gg); /* the argument shown with -1 has no current use */

  return;                       /* (nprev == d->nrows_in_plot); */
}

/*-------------------------------------------------------------------------*/
/*                     reverse pipeline                                    */
/*-------------------------------------------------------------------------*/

/* XXX duncan and dfs: you need to sort this out
void
world_to_raw_by_var (gint pt, gint j, displayd *display, GGobiData *d, ggobid *gg)
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
world_to_raw (gint pt, splotd *sp, GGobiData *d, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;

  if(GGOBI_IS_EXTENDED_DISPLAY(display)) {
     GGobiExtendedDisplayClass *klass;
     klass = GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
     if(klass->world_to_raw)
         klass->world_to_raw(display, sp, pt, d, gg);
  } 
}
*/
