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

/**
 * limits_adjust:
 * @min: minimum value
 * @max: maximumum value
 *
 * Adjust limits to add 10% extra space.
 *
 */
void
limits_adjust (gfloat * min, gfloat * max)
{
  if (*max - *min == 0) {
    if (*min == 0.0) {
      *min = -1.0;
      *max = 1.0;
    }
    else {
      *min = 0.9 * *min;
      *max = 1.1 * *max;
    }
  }

  if (*max < *min) {
    gfloat ftmp = *max;
    *max = *min;
    *min = ftmp;
  }
}

/**
 * Specify the limits used: from data or user specification
 */
void
limits_set_from_vartable (vartabled * vt)
{
  gfloat min, max;

  if (vt->lim_specified_p) {
    min = vt->lim_specified_tform.min;
    max = vt->lim_specified_tform.max;
  }
  else {
    min = vt->lim_tform.min;
    max = vt->lim_tform.max;
  }

  limits_adjust (&min, &max);
  vt->lim.min = min;
  vt->lim.max = max;
}

/*
 * Set limits for raw variable
 */
static void
limits_raw_set_by_var (GGobiData * d, gint j, gboolean visible_only)
{
  gint i, m;
  vartabled *vt = ggobi_data_get_vartable(d, j);
  greal min, max;

  min = G_MAXFLOAT;
  max = -G_MAXFLOAT;

  if (visible_only) {  /*-- if using visible cases only --*/
    for (m = 0; m < d->nrows_in_plot; m++) {
      i = d->rows_in_plot.els[m];
      if (!d->missings_show_p && ggobi_data_is_missing(d, i,j));
      else {
        if (d->raw.vals[i][j] < min)
          min = d->raw.vals[i][j];
        else if (d->raw.vals[i][j] > max)
          max = d->raw.vals[i][j];
      }
    }
  }
  else {
    for (i = 0; i < d->nrows; i++) {
      if (!d->missings_show_p && ggobi_data_is_missing(d, i,j));
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

/*
 * Set limits for all raw variables
 */
static void
limits_raw_set (GGobiData * d, gboolean visible_only)
{
  gint j;

  for (j = 0; j < d->ncols; j++)
    limits_raw_set_by_var (d, j, visible_only);
}

/*
 * Set limits for tranformed variable
 */
static void
limits_tform_set_by_var (GGobiData * d, gint j, gboolean visible_only)
{
  gint i, m;
  vartabled *vt = ggobi_data_get_vartable(d, j);
  greal min, max;

  min = G_MAXFLOAT;
  max = -G_MAXFLOAT;

  if (visible_only) {
    for (m = 0; m < d->nrows_in_plot; m++) {
      i = d->rows_in_plot.els[m];
      if (!d->missings_show_p && ggobi_data_is_missing(d, i, j));
      else {
        if (d->tform.vals[i][j] < min)
          min = d->tform.vals[i][j];
        else if (d->tform.vals[i][j] > max)
          max = d->tform.vals[i][j];
      }
    }
  }
  else {
    for (i = 0; i < d->nrows; i++) {
      if (!d->missings_show_p && ggobi_data_is_missing(d, i, j));
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

void
limits_display_set_by_var (GGobiData * d, gint j, gboolean visible_only)
{
  gint i, m, np = 0;
  gfloat sum = 0.0;
  gfloat *x = (gfloat *) g_malloc (d->nrows * sizeof (gfloat));
  vartabled *vt = ggobi_data_get_vartable(d, j);
  greal min, max;

  min = G_MAXFLOAT;
  max = -G_MAXFLOAT;

  if (visible_only) {
    for (m = 0; m < d->nrows_in_plot; m++) {
      i = d->rows_in_plot.els[m];
      /*-- lim_display and stats: only use non-missing cases --*/
      if (ggobi_data_is_missing(d, i, j));
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
  else {

    for (i = 0; i < d->nrows; i++) {

      if (ggobi_data_is_missing(d, i, j));
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
  qsort ((void *) x, np, sizeof (gfloat), fcompare);
  vt->median =
    ((np % 2) != 0) ? x[(np - 1) / 2] : (x[np / 2 - 1] + x[np / 2]) / 2.;

  g_free ((gpointer) x);
}
static void
limits_tform_set (GGobiData * d, gboolean visible_only)
{
  gint j;

  for (j = 0; j < d->ncols; j++) {
    limits_tform_set_by_var (d, j, visible_only);
    limits_display_set_by_var (d, j, visible_only);
  }
}


void
limits_set (GGobiData * d, gboolean do_raw, gboolean do_tform,
            gboolean visible_only)
{
  gint j;
  vartabled *vt;

  if (do_raw)
    limits_raw_set (d, visible_only);
  if (do_tform)
    limits_tform_set (d, visible_only);

  for (j = 0; j < d->ncols; j++) {
    vt = ggobi_data_get_vartable(d, j);
    limits_set_from_vartable (vt);
  }
}


void
limits_set_by_var (GGobiData * d, guint j, gboolean do_raw, gboolean do_tform,
                   gboolean visible_only)
{
  vartabled *vt = ggobi_data_get_vartable(d, j);

  if (do_raw)
    limits_raw_set_by_var (d, j, visible_only);
  if (do_tform)
    limits_tform_set_by_var (d, j, visible_only);

  limits_set_from_vartable (vt);
}

/*  Recenter the data using the current sticky point */
void
recenter_data (gint i, GGobiData * d, ggobid * gg)
{
  vartabled *vt;
  greal x;
  gint j;

  for (j = 0; j < d->ncols; j++) {
    vt = ggobi_data_get_vartable(d, j);
    if (i >= 0) {
      x = (vt->lim_tform.max - vt->lim_tform.min) / 2;
      vt->lim_specified_p = true;
      vt->lim_specified_tform.min = d->tform.vals[i][j] - x;
      vt->lim_specified_tform.max = d->tform.vals[i][j] + x;
    }
    else {
     /*-- if no point was specified, recenter using defaults --*/
      vt->lim_specified_p = false;
    }
    
    g_signal_emit_by_name(d, "col_data_changed", (guint) j);
  }
}
