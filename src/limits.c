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
 * Adjusts limits so that:
 *   1) if they are equal, they are separated by 20%, unless they are zero,
 *      in which case they become [-1.0, 1.0]
 *   2) the lesser one is the min
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

/*  Recenter the data using the current sticky point */
// FIXME: This should probably just operate at the plot/viewport level, especially
// since specified limits are going away
void
recenter_data (gint i, GGobiStage * d, ggobid * gg)
{
  GGobiVariable *var;
  greal x;
  gint j;

  for (j = 0; j < d->n_cols; j++) {
    var = ggobi_stage_get_variable(d, j);
    if (i >= 0) {
      x = ggobi_variable_get_range(var) / 2;
      var->lim_specified_p = true;
      var->lim_specified.min = ggobi_stage_get_raw_value(d, i, j) - x;
      var->lim_specified.max = ggobi_stage_get_raw_value(d, i, j) + x;
    }
    else {
     /*-- if no point was specified, recenter using defaults --*/
      var->lim_specified_p = false;
    }
    
    //g_signal_emit_by_name(d, "col_data_changed", (guint) j);
  }
}
