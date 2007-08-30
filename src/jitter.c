/* jitter.c */
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

/*
 * Contains jittering routines for tform; see missing.c for the
 * jittering routines for missing data.
*/

#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*
 * generate a random value.
*/
gfloat
jitter_randval (gint type)
{
  gdouble drand = 0;

  if (type == UNIFORM) {
    drand = g_random_double_range (-1, 1);
  } else if (type == NORMAL) {
    drand = random_normal();
  }
  return (gfloat) drand;
}

void
rejitter (gint * selected_cols, gint nselected_cols, GGobiStage * d,
          GGobiSession * gg)
{
  gint i, j, k;
  greal frand, fworld, fjit;
  GGobiVariable *var;

  for (j = 0; j < nselected_cols; j++) {
    k = selected_cols[j];
    var = ggobi_stage_get_variable(d, k);

    for (i = 0; i < d->n_rows; i++) {
     frand = (greal) jitter_randval (d->jitter.type) * PRECISION1;

      /*
       * The world.vals used here is already jittered:
       * subtract out the previous jittered value ...
       */
      if (d->jitter.convex) {
        fworld = d->world.vals[i][k] - d->jitdata.vals[i][k];
        fjit = (greal) var->jitter_factor * (frand - fworld);
      }
      else
        fjit = var->jitter_factor * frand;

      d->jitdata.vals[i][k] = fjit;
    }
  }
  tform_to_world(d);
  displays_tailpipe (FULL, gg);
}


void
jitter_value_set (gfloat value, GGobiStage * d, GGobiSession * gg)
{
  GtkWidget *tree_view =
    get_tree_view_from_object (G_OBJECT (gg->jitter_ui.window));
  gint *vars;                   // = (gint *) g_malloc (d->n_cols * sizeof(gint));
  gint nvars;
  gint j;
  GGobiVariable *var;

  vars = get_selections_from_tree_view (tree_view, &nvars);

  for (j = 0; j < nvars; j++) {
    var = ggobi_stage_get_variable(d, vars[j]);
    var->jitter_factor = value;
  }

  g_free ((gpointer) vars);
}
