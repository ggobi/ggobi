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


//FIXME: all of this should be moved inside of GGobiVariable and
// cached lazily.  Can remove a lot of duplication once FilterStage 
// is in place
// This code will be gone after tform stage arrives.

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
limits_set_from_vartable (GGobiVariable * var)
{
  gfloat min, max;

  if (var->lim_specified_p) {
    min = var->lim_specified_tform.min;
    max = var->lim_specified_tform.max;
  }
  else {
    min = var->lim_tform.min;
    max = var->lim_tform.max;
  }

  limits_adjust (&min, &max);
  var->lim.min = min;
  var->lim.max = max;
}

/*
 * Set limits for raw variable
 */
static void
limits_raw_set_by_var (GGobiStage * s, gint j, gboolean visible_only)
{
  gint i;
  GGobiVariable *var = ggobi_stage_get_variable(s, j);
  greal min, max;

  min = G_MAXFLOAT;
  max = -G_MAXFLOAT;

  // FIXME: this isn't pretty but this limits stuff doesn't match the new design
  s = ggobi_stage_get_root(s);
  
  if (visible_only) {  /*-- if using visible cases only --*/
    GGobiStage *tmp_s = ggobi_stage_find(s, GGOBI_MAIN_STAGE_FILTER);
    if (tmp_s) /* this (obsolete) code can be called before pipeline is built */
      s = tmp_s;
  }
  for (i = 0; i < s->n_rows; i++) {
    if (!s->missings_show_p && ggobi_stage_is_missing(s, i,j));
    else {
      if (ggobi_stage_get_raw_value(s, i, j) < min)
        min = ggobi_stage_get_raw_value(s, i, j);
      else if (ggobi_stage_get_raw_value(s, i, j) > max)
        max = ggobi_stage_get_raw_value(s, i, j);
    }
  }

  var->lim_raw.min = min;
  var->lim_raw.max = max;
}

/*
 * Set limits for all raw variables
 */
static void
limits_raw_set (GGobiStage * d, gboolean visible_only)
{
  gint j;

  for (j = 0; j < d->n_cols; j++)
    limits_raw_set_by_var (d, j, visible_only);
}

/*
 * Set limits for tranformed variable
 */
static void
limits_tform_set_by_var (GGobiStage * d, gint j, gboolean visible_only)
{
  gint i;
  GGobiVariable *var = ggobi_stage_get_variable(d, j);
  greal min, max;

  min = G_MAXFLOAT;
  max = -G_MAXFLOAT;

  d = ggobi_stage_get_root(d);
  if (visible_only) {
    GGobiStage *tmp_d = ggobi_stage_find(d, GGOBI_MAIN_STAGE_FILTER);
    if (tmp_d) /* this (obsolete) code can be called before pipeline is built */
      d = tmp_d;
  }
  
  for (i = 0; i < d->n_rows; i++) {
    if (!d->missings_show_p && ggobi_stage_is_missing(d, i, j));
    else {
      if (d->tform.vals[i][j] < min)
        min = d->tform.vals[i][j];
      else if (d->tform.vals[i][j] > max)
        max = d->tform.vals[i][j];
    }
  }

  var->lim_tform.min = min;
  var->lim_tform.max = max;
}

void
limits_display_set_by_var (GGobiStage * d, gint j, gboolean visible_only)
{
  gint i, m, np = 0;
  gfloat sum = 0.0;
  gfloat *x = (gfloat *) g_malloc (d->n_rows* sizeof (gfloat));
  GGobiVariable *var = ggobi_stage_get_variable(d, j);
  greal min, max;

  min = G_MAXFLOAT;
  max = -G_MAXFLOAT;

  d = ggobi_stage_get_root(d);
  if (visible_only) {
    d = ggobi_stage_find(d, GGOBI_MAIN_STAGE_FILTER);
    for (m = 0; m < d->n_rows; m++) {
      /*-- lim_display and stats: only use non-missing cases --*/
      if (ggobi_stage_is_missing(d, m, j));
      else {
        if (d->tform.vals[m][j] < min)
          min = d->tform.vals[m][j];
        else if (d->tform.vals[m][j] > max)
          max = d->tform.vals[m][j];

        sum += d->tform.vals[m][j];
        x[np] = d->tform.vals[m][j];
        np++;
      }
    }
  }
  else {

    for (i = 0; i < d->n_rows; i++) {

      if (ggobi_stage_is_missing(d, i, j));
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

  var->lim_display.min = min;
  var->lim_display.max = max;

  var->mean = sum / np;

  /*-- median: sort the temporary vector, and find its center --*/
  qsort ((void *) x, np, sizeof (gfloat), fcompare);
  var->median =
    ((np % 2) != 0) ? x[(np - 1) / 2] : (x[np / 2 - 1] + x[np / 2]) / 2.;

  g_free ((gpointer) x);
}
static void
limits_tform_set (GGobiStage * d, gboolean visible_only)
{
  gint j;

  for (j = 0; j < d->n_cols; j++) {
    limits_tform_set_by_var (d, j, visible_only);
    limits_display_set_by_var (d, j, visible_only);
  }
}


void
limits_set (GGobiStage * d, gboolean do_raw, gboolean do_tform,
            gboolean visible_only)
{
  gint j;
  GGobiVariable *var;

  if (do_raw)
    limits_raw_set (d, visible_only);
  if (do_tform)
    limits_tform_set (d, visible_only);

  for (j = 0; j < d->n_cols; j++) {
    var = ggobi_stage_get_variable(d, j);
    limits_set_from_vartable (var);
  }
}


void
limits_set_by_var (GGobiStage * d, guint j, gboolean do_raw, gboolean do_tform,
                   gboolean visible_only)
{
  GGobiVariable *var = ggobi_stage_get_variable(d, j);

  if (do_raw)
    limits_raw_set_by_var (d, j, visible_only);
  if (do_tform)
    limits_tform_set_by_var (d, j, visible_only);

  limits_set_from_vartable (var);
}

/*  Recenter the data using the current sticky point */
void
recenter_data (gint i, GGobiStage * d, ggobid * gg)
{
  GGobiVariable *var;
  greal x;
  gint j;

  for (j = 0; j < d->n_cols; j++) {
    var = ggobi_stage_get_variable(d, j);
    if (i >= 0) {
      x = (var->lim_tform.max - var->lim_tform.min) / 2;
      var->lim_specified_p = true;
      var->lim_specified_tform.min = d->tform.vals[i][j] - x;
      var->lim_specified_tform.max = d->tform.vals[i][j] + x;
    }
    else {
     /*-- if no point was specified, recenter using defaults --*/
      var->lim_specified_p = false;
    }
    
    //g_signal_emit_by_name(d, "col_data_changed", (guint) j);
  }
}
