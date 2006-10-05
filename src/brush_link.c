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
symbol_link_by_id (gboolean persistentp, gint k, GGobiStage * sd, ggobid * gg)
{
  GGobiStage *d;
  GSList *l;
  gint i, id = -1;
  /*-- this is the cpanel for the display being brushed --*/
  cpaneld *cpanel = &gg->current_display->cpanel;
  GGobiAttrSetMethod brush = (persistentp || cpanel->br.mode == BR_PERSISTENT) ?
    ATTR_SET_PERSISTENT : ATTR_SET_TRANSIENT;
  gboolean changed = false;

  /*-- k is the row number in source_d --*/

  id = ggobi_stage_get_row_for_id(sd, GGOBI_DATA(sd)->rowIds[k]);
  if (id < 0)
    return false;

  for (l = gg->d; l; l = l->next) {
    d = (GGobiStage *) l->data;
    if (d == sd)
      continue;        /*-- skip the originating datad --*/

    i = ggobi_stage_get_row_for_id(d, GGOBI_DATA(sd)->rowIds[id]);

    if (i < 0)              /*-- then no cases in d have this id --*/
      continue;
    
    GGOBI_STAGE_ATTR_INIT_ALL(d);  
    
    /*-- if we get here, d has one case with the indicated id --*/
    changed = true;
    if (GGOBI_STAGE_GET_ATTR_VISIBLE(d, i)) {
       if (!GGOBI_STAGE_GET_ATTR_HIDDEN(d, i)) {
         GGOBI_STAGE_SET_ATTR_COLOR(d, i, GGOBI_STAGE_GET_ATTR_COLOR(sd, k), brush);
         GGOBI_STAGE_SET_ATTR_SIZE(d, i, GGOBI_STAGE_GET_ATTR_SIZE(sd, k), brush);
         GGOBI_STAGE_SET_ATTR_TYPE(d, i, GGOBI_STAGE_GET_ATTR_TYPE(sd, k), brush);
       }
       GGOBI_STAGE_SET_ATTR_HIDDEN(d, i, GGOBI_STAGE_GET_ATTR_HIDDEN(sd, k), brush);
    }
  }
  return changed;
}

gboolean
exclude_link_by_id (guint k, GGobiStage * sd, ggobid * gg)
{
/*-- sd = source_d --*/
  GGobiStage *d;
  GSList *l;
  gint i, id = -1;
  gboolean changed = false;

  /*-- k is the row number in source_d --*/
  id = ggobi_stage_get_row_for_id(sd, GGOBI_DATA(sd)->rowIds[k]);
  if (id < 0)
    return false;

  for (l = gg->d; l; l = l->next) {
    d = (GGobiStage *) l->data;
    if (d == sd)
      continue;        /*-- skip the originating datad --*/
    GGOBI_STAGE_ATTR_INIT_ALL(d);

    i = ggobi_stage_get_row_for_id(d, GGOBI_DATA(sd)->rowIds[id]);

    if (i < 0)              /*-- then no cases in d have this id --*/
      continue;

    /*-- if we get here, d has one case with the indicated id --*/
    changed = true;
    if (GGOBI_STAGE_GET_ATTR_SAMPLED(d, i))
      GGOBI_STAGE_SET_ATTR_EXCLUDED(d, i, GGOBI_STAGE_GET_ATTR_EXCLUDED(sd, k));
  }
  return changed;
}

/*----------------------------------------------------------------------*/
/*   Linking within and between datad's using a categorical variable    */
/*----------------------------------------------------------------------*/

void
brush_link_by_var (gint jlinkby, vector_b * levelv,
                   cpaneld * cpanel, GGobiStage * d, ggobid * gg)
{
  gint m, i, level_value;

  GGOBI_STAGE_ATTR_INIT_ALL(d); 
  for (m = 0; m < d->nrows_in_plot; m++) {
    i = d->rows_in_plot.els[m];

    level_value = (gint) ggobi_stage_get_raw_value(d, i, jlinkby);
    GGOBI_STAGE_BRUSH_POINT(d, i, levelv->els[level_value], cpanel->br.point_targets, cpanel->br.mode);
  }
}

/*
 * We're working too hard here, looping whether there's any
 * change or not.  Maybe there's an easy way to set the value
 * of changed by using pts_under_brush_prev?
*/
gboolean
build_symbol_vectors_by_var (cpaneld * cpanel, GGobiStage * d, ggobid * gg)
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
  brush_link_by_var (j, &levelv, cpanel, d, gg);

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
      brush_link_by_var (j, &levelv, cpanel, dd, gg);
    }
  }

  vectorb_free (&levelv);

  return (true);
}

