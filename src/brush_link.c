/* brush_link.c */
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

#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*----------------------------------------------------------------------*/
/*                Linking to other datad's by id                        */
/*----------------------------------------------------------------------*/

gboolean
brush_all_matching_id (GGobiStage * sd, gint k, gboolean condition, BrushTargetType brush_mode, GGobiAttrSetMethod brush)
{
  gboolean changed = false;
  GGobiStage *root = ggobi_stage_get_root(sd);
  
  gchar* rowid = ggobi_stage_get_row_id(sd, k);
  if (!rowid)
    return false;

  for (GSList* l = sd->gg->d; l; l = l->next) {
    GGobiStage* d = l->data;
    if (d == root)
      continue;        /*-- skip the originating datad --*/

    gint i = ggobi_stage_get_row_for_id(d, rowid);
    if (i < 0)              /*-- then no cases in d have this id --*/
      continue;  
    
    /*-- if we get here, d has one case with the indicated id --*/
    changed = true;    
    GGOBI_STAGE_ATTR_INIT_ALL(d);  
    GGOBI_STAGE_BRUSH_POINT(d, i, condition, brush_mode, brush);
  }
  return changed;
}

/*----------------------------------------------------------------------*/
/*   Linking within and between datad's using a categorical variable    */
/*----------------------------------------------------------------------*/

void
brush_matching_cv (gint jlinkby, vector_b * levelv,
                   cpaneld * cpanel, GGobiStage * d, ggobid * gg)
{
  gint m, level_value;

  GGOBI_STAGE_ATTR_INIT_ALL(d); 
  for (m = 0; m < d->n_rows; m++) {
    level_value = (gint) ggobi_stage_get_raw_value(d, m, jlinkby);
    GGOBI_STAGE_BRUSH_POINT(d, m, levelv->els[level_value], cpanel->br.point_targets, cpanel->br.mode);
  }
}

/*
 * We're working too hard here, looping whether there's any
 * change or not.  Maybe there's an easy way to set the value
 * of changed by using pts_under_brush_prev?
*/
gboolean
brush_all_matching_cv (cpaneld * cpanel, GGobiStage * d, ggobid * gg)
{
  gint i, m, j, level_value, level_value_max;
  vector_b levelv;
  GSList *l;

  if (!d->linkvar)
    return false;

  j = ggobi_stage_get_col_index_for_name(d, d->linkvar);
  level_value_max = ggobi_variable_get_max(ggobi_stage_get_variable(d, j));

  /*-- find the levels which are among the points under the brush --*/
  vectorb_init_null (&levelv);
  vectorb_alloc (&levelv, level_value_max + 1);
  vectorb_zero (&levelv);
  for (m = 0; m < d->nrows_under_brush; m++) {
    i = d->rows_under_brush.els[m];
    level_value = (gint) ggobi_stage_get_raw_value(d, i, j);
    levelv.els[level_value] = true;
  }

  /*-- first do this d --*/
  brush_matching_cv (j, &levelv, cpanel, d, gg);

  /*-- now for the rest of them --*/
  for (l = gg->d; l; l = l->next) {
    GGobiStage *dd = l->data;
    if (dd == d) continue;

    level_value_max = ggobi_variable_get_max(ggobi_stage_get_variable(dd, j));
    vectorb_init_null (&levelv);
    vectorb_alloc (&levelv, level_value_max + 1);
    vectorb_zero (&levelv);


    j = ggobi_stage_get_col_index_for_name(dd, d->linkvar);
    if (j != -1) {
      brush_matching_cv (j, &levelv, cpanel, dd, gg);
    }
  }

  vectorb_free (&levelv);

  return (true);
}

