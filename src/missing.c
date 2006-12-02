/* missing.c */
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
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
/*#include <limits.h>*/
/*#include <float.h>*/
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*
 * For the datad currently selected in gg->impute.notebook,
 * generate a new datad using d->missing.  
 *
*/
void
missings_datad_cb (GtkWidget * w, ggobid * gg)
{
  GObject *obj = G_OBJECT (gg->impute.window);
  GtkWidget *tree_view = get_tree_view_from_object (obj);
  GGobiStage *d = GGOBI_STAGE(g_object_get_data (G_OBJECT (tree_view), "datad"));
  static gchar *lnames[] = { "present", "missing" };
  
  if (!ggobi_stage_has_missings(d)) return;
  
  gint i, j, k;
  gint *cols_with_missings, ncols_with_missings;

  ncols_with_missings = 0;
  cols_with_missings = g_malloc (d->n_cols * sizeof (gint));
  for (j = 0; j < d->n_cols; j++) {
    if (ggobi_variable_has_missings(ggobi_stage_get_variable(d, j)))
      cols_with_missings[ncols_with_missings++] = j;
  }

  GGobiStage *dnew = GGOBI_STAGE(ggobi_data_new (d->n_rows, ncols_with_missings));
  ggobi_stage_set_name(dnew, g_strdup_printf ("%s (missing)", d->name));

  for (j = 0; j < ncols_with_missings; j++) {
    k = cols_with_missings[j];

    ggobi_stage_set_col_name(dnew, j, ggobi_stage_get_col_name(d, k));
    for (i = 0; i < d->n_rows; i++) {
      ggobi_stage_set_categorical_value(dnew, i, j, lnames[ ggobi_stage_is_missing(d, i, k)]);
    }
  }

  GGOBI_STAGE_ATTR_INIT_ALL(d);  
  for (i = 0; i < d->n_rows; i++) {
    GGOBI_STAGE_SET_ATTR_COLOR(dnew, i, GGOBI_STAGE_GET_ATTR_COLOR(d, i), ATTR_SET_PERSISTENT);
    GGOBI_STAGE_SET_ATTR_SIZE(dnew, i, GGOBI_STAGE_GET_ATTR_SIZE(d, i), ATTR_SET_PERSISTENT);
    GGOBI_STAGE_SET_ATTR_TYPE(dnew, i, GGOBI_STAGE_GET_ATTR_TYPE(d, i), ATTR_SET_PERSISTENT);
    //dnew->color.els[i] = d->color.els[i];
    //dnew->color_now.els[i] = d->color_now.els[i];
    //dnew->glyph.els[i].type = d->glyph.els[i].type;
    //dnew->glyph_now.els[i].type = d->glyph_now.els[i].type;
    //dnew->glyph.els[i].size = d->glyph.els[i].size;
    //dnew->glyph_now.els[i].size = d->glyph_now.els[i].size;

    ggobi_stage_set_row_id(dnew, i, ggobi_stage_get_row_id(dnew, i));
  }
  limits_set(dnew, FALSE);

  ggobi_stage_attach(dnew, gg, false);

  g_free (cols_with_missings);
}
