/*-- limits.c: variable ranges --*/
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

#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


/*-------------------------------------------------------------------------*/
/*                       variable limits                                   */
/*-------------------------------------------------------------------------*/

/*-- this isn't being used --*/
void
min_max (gfloat **vals, gint jvar, gfloat *min, gfloat *max,
  GGobiData *d, ggobid *gg)
/*
 * Find the minimum and maximum values of variable jvar
 * using min-max scaling.
*/
{
  gint i, m;

  /*-- choose an initial value for *min and *max --*/
  *min = *max = vals[d->rows_in_plot.els[0]][jvar];

  if (gg->lims_use_visible) {  /*-- if using visible cases only --*/

    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot.els[m];
      if (vals[i][jvar] < *min)
        *min = vals[i][jvar];
      else if (vals[i][jvar] > *max)
        *max = vals[i][jvar];
    }

  } else {  /*-- if use all points --*/

    for (i=0; i<d->nrows; i++) {
      if (vals[i][jvar] < *min)
        *min = vals[i][jvar];
      else if (vals[i][jvar] > *max)
        *max = vals[i][jvar];
    }
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

static void
limits_raw_set_by_var (gint j, GGobiData *d, ggobid *gg) {
  gint i, m;
  vartabled *vt = vartable_element_get (j, d);
  greal min, max;

  min = d->raw.vals[d->rows_in_plot.els[0]][j];
  max = d->raw.vals[d->rows_in_plot.els[0]][j];

  /*-- if we're ignoring missings and the first element is missing, the
       limits will be wrong --*/
  if (vt->nmissing > 0 && !d->missings_show_p && MISSING_P(0,j)) {
    if (gg->lims_use_visible) {
      for (m=1; m<d->nrows_in_plot; m++) {
        i = d->rows_in_plot.els[m];
        if (!MISSING_P(i,j)) {
          min = d->raw.vals[i][j];
          max = d->raw.vals[i][j];
          break;
        }
      }
    } else {
      for (i=0; i<d->nrows; i++) {
        if (!MISSING_P(i,j)) {
          min = d->raw.vals[i][j];
          max = d->raw.vals[i][j];
          break;
        }
      }
    }
  }

  if (gg->lims_use_visible) {  /*-- if using visible cases only --*/
    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot.els[m];
      if (vt->nmissing > 0 && !d->missings_show_p && MISSING_P(i,j))
        ;
      else {
        if (d->raw.vals[i][j] < min)
          min = d->raw.vals[i][j];
        else if (d->raw.vals[i][j] > max)
          max = d->raw.vals[i][j];
      }
    }
  } else {

    for (i=0; i<d->nrows; i++) {
      if (vt->nmissing > 0 && !d->missings_show_p && MISSING_P(i,j))
        ;
      else {
        if (d->raw.vals[i][j] < min)
          min = d->raw.vals[i][j];
        else if (d->raw.vals[i][j] > max)
          max = d->raw.vals[i][j];
      }
    }
  }

  vt->lim_raw.min = min;
  vt->lim_raw.max = max;
}

static void
limits_raw_set (GGobiData *d, ggobid *gg) {
  gint j;

  g_assert (d->raw.nrows == d->nrows);
  g_assert (d->raw.ncols == d->ncols);

  for (j=0; j<d->ncols; j++)
    limits_raw_set_by_var (j, d, gg);
}

static void
limits_tform_set_by_var (gint j, GGobiData *d, ggobid *gg)
{
  gint i, m;
  vartabled *vt = vartable_element_get (j, d);
  greal min, max;

  min = d->tform.vals[d->rows_in_plot.els[0]][j];
  max = d->tform.vals[d->rows_in_plot.els[0]][j];

  /*-- if we're ignoring missings and the first element is missing, the
       limits will be wrong --*/
  if (vt->nmissing > 0 && !d->missings_show_p && MISSING_P(0,j)) {
    if (gg->lims_use_visible) {
      for (m=1; m<d->nrows_in_plot; m++) {
        i = d->rows_in_plot.els[m];
        if (!MISSING_P(i,j)) {
          min = d->tform.vals[i][j];
          max = d->tform.vals[i][j];
          break;
        }
      }
    } else {
      for (i=0; i<d->nrows; i++) {
        if (!MISSING_P(i,j)) {
          min = d->tform.vals[i][j];
          max = d->tform.vals[i][j];
          break;
        }
      }
    }
  }

  if (gg->lims_use_visible) {
    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot.els[m];
      if (vt->nmissing > 0 && !d->missings_show_p && MISSING_P(i,j))
        ;
      else {
        if (d->tform.vals[i][j] < min)
          min = d->tform.vals[i][j];
        else if (d->tform.vals[i][j] > max)
          max = d->tform.vals[i][j];
      }
    }
  } else {

    for (i=0; i<d->nrows; i++) {
      if (vt->nmissing > 0 && !d->missings_show_p && MISSING_P(i,j))
        ;
      else {
        if (d->tform.vals[i][j] < min)
          min = d->tform.vals[i][j];
        else if (d->tform.vals[i][j] > max)
          max = d->tform.vals[i][j];
      }
    }
  }
  vt->lim_tform.min = min;
  vt->lim_tform.max = max;
}

/*-- this version of the limits never includes missings --*/
void
limits_display_set_by_var (gint j, GGobiData *d, ggobid *gg)
{
  gint i, m, np = 0;
  gfloat sum = 0.0;
  gfloat *x = (gfloat *) g_malloc (d->nrows * sizeof (gfloat));
  vartabled *vt = vartable_element_get (j, d);
  greal min, max;

  min = d->tform.vals[d->rows_in_plot.els[0]][j];
  max = d->tform.vals[d->rows_in_plot.els[0]][j];

  /*-- if the first element is missing, the limits will be wrong --*/
  if (vt->nmissing > 0 && MISSING_P(0,j)) {
    if (gg->lims_use_visible) {
      for (m=0; m<d->nrows_in_plot; m++) {
        i = d->rows_in_plot.els[m];
        if (!MISSING_P(i,j)) {
          min = d->tform.vals[i][j];
          max = d->tform.vals[i][j];
          break;
        }
      }
    } else {
      for (i=0; i<d->nrows; i++) {
        if (!MISSING_P(i,j)) {
          min = d->tform.vals[i][j];
          max = d->tform.vals[i][j];
          break;
        }
      }
    }
  }

  if (gg->lims_use_visible) {

    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot.els[m];

      /*-- lim_display and stats: only use non-missing cases --*/
      if (d->nmissing > 0 && MISSING_P(i,j))
        ;
      else {
        if (d->tform.vals[i][j] < min)
          min = d->tform.vals[i][j];
        else if (d->tform.vals[i][j] > max)
          max = d->tform.vals[i][j];

        sum += d->tform.vals[i][j];
        x[np] = d->tform.vals[i][j];
        np++;
      }
    }
  } else {

    for (i=0; i<d->nrows; i++) {

      if (d->nmissing > 0 && MISSING_P(i,j))
        ;
      else {
        if (d->tform.vals[i][j] < min)
          min = d->tform.vals[i][j];
        else if (d->tform.vals[i][j] > max)
          max = d->tform.vals[i][j];

        sum += d->tform.vals[i][j];
        x[np] = d->tform.vals[i][j];
        np++;
      }
    }
  }

  vt->lim_display.min = min;
  vt->lim_display.max = max;

  vt->mean = sum / np;

  /*-- median: sort the temporary vector, and find its center --*/
  qsort((void *) x, np, sizeof (gfloat), fcompare);
  vt->median = ((np % 2) != 0) ?  x[(np-1)/2] : (x[np/2-1] + x[np/2])/2. ;

  g_free ((gpointer) x);
}
static void
limits_tform_set (GGobiData *d, ggobid *gg) {
  gint j;

  g_assert (d->tform.nrows == d->nrows);
  g_assert (d->tform.ncols == d->ncols);

  for (j=0; j<d->ncols; j++) {
    limits_tform_set_by_var (j, d, gg);
    limits_display_set_by_var (j, d, gg);
  }
}


void
limits_set (gboolean do_raw, gboolean do_tform, GGobiData *d, ggobid *gg)
{
  gint j;
  gfloat min, max;
  vartabled *vt;

  /*-- compute the limits for the raw data --*/
  if (do_raw)
    limits_raw_set (d, gg);

  if (do_tform)
    limits_tform_set (d, gg);

  /*-- specify the limits used: from data or user specification --*/
  for (j=0; j<d->ncols; j++) {
    vt = vartable_element_get (j, d);

    if (vt->lim_specified_p) {
      min = vt->lim_specified_tform.min;
      max = vt->lim_specified_tform.max;
    } else {
      min = vt->lim_tform.min;
      max = vt->lim_tform.max;
    }

    limits_adjust (&min, &max);

    vt->lim.min = min;
    vt->lim.max = max;
  }
}

void
limits_set_by_var (gint j, gboolean do_raw, gboolean do_tform,
  GGobiData *d, ggobid *gg)
{
  gfloat min, max;
  vartabled *vt = vartable_element_get (j, d);

  /*-- compute the limits for the raw data --*/
  if (do_raw)
    limits_raw_set_by_var (j, d, gg);
  /*-- compute the limits for the transformed data --*/
  if (do_tform)
    limits_tform_set_by_var (j, d, gg);

  /*-- specify the limits used: from data or user specification --*/
  if (vt->lim_specified_p) {
    min = vt->lim_specified_tform.min;
    max = vt->lim_specified_tform.max;
  } else {
    min = vt->lim_tform.min;
    max = vt->lim_tform.max;
  }

  limits_adjust (&min, &max);

  vt->lim.min = min;
  vt->lim.max = max;
}

/*-------------------------------------------------------------------------*/
/*           recenter: called from identify                                */
/*-------------------------------------------------------------------------*/

/*  Recenter the data using the current sticky point */
void
recenter_data (gint i, GGobiData *d, ggobid *gg) {
  vartabled *vt;
  greal x;
  gint j;

  g_assert (d->tform.nrows == d->nrows);
  g_assert (d->tform.ncols == d->ncols);

  for (j=0; j<d->ncols; j++) {
    vt = vartable_element_get (j, d);
    if (i >= 0) {
      x = (vt->lim_tform.max - vt->lim_tform.min)/2;
      vt->lim_specified_p = true;
      vt->lim_specified_tform.min = d->tform.vals[i][j] - x;
      vt->lim_specified_tform.max = d->tform.vals[i][j] + x;
    } else {
     /*-- if no point was specified, recenter using defaults --*/
      vt->lim_specified_p = false;
    }
  }
  limits_set (false, true, d, gg);
  vartable_limits_set (d);
  vartable_stats_set (d);

  tform_to_world (d, gg);
  displays_tailpipe (FULL, gg);
}
