/*-- limits.c: variable ranges --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


/*-------------------------------------------------------------------------*/
/*                       variable limits                                   */
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
limits_raw_set_by_var (gint j, datad *d) {
  gint i, m;

  d->vartable[j].lim_raw.min = d->raw.vals[0][j];
  d->vartable[j].lim_raw.max = d->raw.vals[0][j];

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    if (d->nmissing > 0 && MISSING_P(i,j))
      ;
    else {
      if (d->raw.vals[i][j] < d->vartable[j].lim_raw.min)
        d->vartable[j].lim_raw.min = d->raw.vals[i][j];
      else if (d->raw.vals[i][j] > d->vartable[j].lim_raw.max)
        d->vartable[j].lim_raw.max = d->raw.vals[i][j];
    }
  }
}

void
limits_raw_set (datad *d) {
  gint j;

  for (j=0; j<d->ncols; j++)
    limits_raw_set_by_var (j, d);
}

void
limits_tform_set_by_var (gint j, datad *d)
{
  gint i, m, np = 0;
  gfloat sum = 0.0;
  gfloat *x = (gfloat *) g_malloc (d->nrows * sizeof (gfloat));

  d->vartable[j].lim_tform.min = d->tform.vals[0][j];
  d->vartable[j].lim_tform.max = d->tform.vals[0][j];

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    if (d->nmissing > 0 && MISSING_P(i,j))
    ;
    else {
      if (d->tform.vals[i][j] < d->vartable[j].lim_tform.min)
        d->vartable[j].lim_tform.min = d->tform.vals[i][j];
      else if (d->tform.vals[i][j] > d->vartable[j].lim_tform.max)
        d->vartable[j].lim_tform.max = d->tform.vals[i][j];

      sum += d->tform.vals[i][j];
      x[np] = d->tform.vals[i][j];
      np++;
    }
  }

  d->vartable[j].mean = sum / np;

  /*-- median: sort the temporary vector, and find its center --*/
  qsort((void *) x, np, sizeof (gfloat), fcompare);
  d->vartable[j].median = 
    ((np % 2) != 0) ?  x[(np-1)/2] : (x[np/2-1] + x[np/2])/2. ;

  g_free ((gpointer) x);
}

void
limits_tform_set (datad *d) {
  gint j;

  for (j=0; j<d->ncols; j++) {
    limits_tform_set_by_var (j, d);
  }
}


void
limits_set (gboolean do_raw, gboolean do_tform, datad *d)
{
  gint j;
  gfloat min, max;

  /*-- compute the limits for the raw data --*/
  if (do_raw)
    limits_raw_set (d);
  /*-- compute the limits for the transformed data --*/
  if (do_tform)
    limits_tform_set (d);

  /*-- specify the limits used: from data or user specification --*/
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

