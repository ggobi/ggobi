/* subset.c */
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

void
subset_init (GGobiStage *d, ggobid *gg)
{
  gfloat fnr = (gfloat) d->n_rows;

  d->subset.random_n = d->n_rows;

  d->subset.bstart_adj = (GtkAdjustment *)
    gtk_adjustment_new (1.0, 1.0, (fnr-2.0), 1.0, 5.0, 0.0);
  d->subset.bsize_adj = (GtkAdjustment *)
    gtk_adjustment_new (fnr/10.0, 1.0, fnr, 1.0, 5.0, 0.0);

  d->subset.estart_adj = (GtkAdjustment *)
    gtk_adjustment_new (1.0, 1.0, fnr-2.0, 1.0, 5.0, 0.0);
  d->subset.estep_adj = (GtkAdjustment *)
    gtk_adjustment_new (fnr/10.0, 1.0, fnr-1, 1.0, 5.0, 0.0);
}

/*------------------------------------------------------------------*/
/*         utilities used within this file                          */
/*------------------------------------------------------------------*/

void
subset_set_all(GGobiStage *d, gboolean value) {
  gint i;

  GGOBI_STAGE_ATTR_INIT(d, sampled);
  for (i=0; i<d->n_rows; i++)
    GGOBI_STAGE_SET_ATTR_SAMPLED(d, i, value);
}

/*------------------------------------------------------------------*/

void
subset_apply (GGobiStage *d) {
  ggobi_stage_set_rows_in_plot(d);
  if (d->gg->cluster_ui.window != NULL)
    cluster_table_update (d, d->gg);
}

/*
 * This algorithm taken from Knuth, Seminumerical Algorithms;
 * Vol 2 of his series.
*/
gboolean
subset_random (gint n, GGobiStage *d) {
  gint t, m, top = d->n_rows;
  gfloat rrand;

  subset_set_all(d, false);

  if (n < 0 || n > top) 
    return false;

  GGOBI_STAGE_ATTR_INIT(d, sampled);
  for (t=0, m=0; t<top && m<n; t++) {
    rrand = (gfloat) randvalue ();
    if (((top - t) * rrand) < (n - m) && !GGOBI_STAGE_GET_ATTR_SAMPLED(d, t)) {
      GGOBI_STAGE_SET_ATTR_SAMPLED(d, t, true);
      m++;            
    }
  }

  return true;
}

gboolean
subset_block (gint bstart, gint bsize, GGobiStage *d)
{
  gint i, k;
  gboolean subsetsize = 0;

  GGOBI_STAGE_ATTR_INIT(d, sampled);
  if (bstart >= 0 && bstart < d->n_rows && bsize > 0) {
    subset_set_all(d, false);

    for (i=bstart, k=1; i<d->n_rows && k<=bsize; i++, k++) {
      GGOBI_STAGE_SET_ATTR_SAMPLED(d, i, true);
      subsetsize++;
    }
  }

  if (subsetsize == 0)
    quick_message ("The limits aren't correctly specified.", false);
 
  return (subsetsize > 0);
}

gboolean
subset_range (GGobiStage *d)
{
  gint i, j;
  gint subsetsize = 0;
  GGobiVariable *var;
  gboolean add;

  subset_set_all(d, false);

  GGOBI_STAGE_ATTR_INIT(d, sampled);
  for (i=0; i<d->n_rows; i++) {
    add = true;
    for (j=0; j<d->n_cols; j++) {
      var = ggobi_stage_get_variable(d, j);
      if (var->lim_specified_p) {
        if (d->tform.vals[i][j] < var->lim_specified.min ||
            d->tform.vals[i][j] > var->lim_specified.max)
        {
          add = false;
        }
      }
    }
    if (add) {
      GGOBI_STAGE_SET_ATTR_SAMPLED(d, i, true);
      subsetsize++;
    }
  }

  if (subsetsize == 0)
    quick_message ("Use the variable manipulation panel to set ranges.", false);
 
  return (subsetsize > 0);
}

gboolean
subset_everyn (gint estart, gint estep, GGobiStage *d)
{
  gint i;
  gint top = d->n_rows;

  if (!(estart >= 0 && estart < top-1 && estep >= 0 && estep < top)) {
    quick_message ("Interval not correctly specified.", false);
    return(false);
  }
  
  subset_set_all(d, false);

  GGOBI_STAGE_ATTR_INIT(d, sampled);
  for(i = estart; i < top; i += estep)
    GGOBI_STAGE_SET_ATTR_SAMPLED(d, i, true);
  
  return true;
}

/*-- create a subset of only the points with sticky ids --*/
/*-- Added by James Brook, Oct 1994 --*/
gboolean
subset_sticky (GGobiStage *d)
{
  gint id;
  GSList *l;

  if (g_slist_length (d->sticky_ids) == 0)
    return false;
    
  subset_set_all(d, false);

  GGOBI_STAGE_ATTR_INIT(d, sampled);
  for (l = d->sticky_ids; l; l = l->next) {
    id = GPOINTER_TO_INT (l->data);
    if (id < d->n_rows)
      GGOBI_STAGE_SET_ATTR_SAMPLED(d, id, true);
  }

  return true;
}

gboolean
subset_rowlab (gchar *substr, gint substr_pos, gboolean ignore_case,
  GGobiStage *d)
{
  gint i;
  gssize slen;

  g_debug("Rowlab %s", substr);
  
  if (substr == NULL || (slen = g_utf8_strlen(substr, -1)) == 0)
    return false;

  GGOBI_STAGE_ATTR_INIT(d, sampled);
  subset_set_all(d, false);

  if (ignore_case)
    substr = g_utf8_strdown(substr, -1);
  else 
    substr = g_strdup(substr);

  for (i=0; i < d->n_rows; i++) {
    gchar *label = ggobi_stage_get_row_id(d, i);
    
    if (!label)
      continue;
    
    gint llen = g_utf8_strlen(label, -1);
    gint start = substr_pos == 3 ? llen - slen : 0;
    gint safe_len = llen < slen ? llen : slen;

    if (start < 0)
      continue;
    if (ignore_case)
      label = g_utf8_strdown(label, substr_pos == 2 ? safe_len : llen);
    else 
      label = g_strndup(label, substr_pos == 2 ? safe_len : llen);

    if (substr_pos == 1 || substr_pos == 4) {
      gchar *inside = strstr(label, substr);
      if ((inside && substr_pos == 1) || (!inside && substr_pos == 4))
        GGOBI_STAGE_SET_ATTR_SAMPLED(d, i, true);
    } else if (!g_utf8_collate(g_utf8_offset_to_pointer(label, start), substr))
        GGOBI_STAGE_SET_ATTR_SAMPLED(d, i, true);
    g_free(label);
  }
  g_free(substr);
  return true;
}
