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
subset_init (GGobiData *d, ggobid *gg)
{
  gfloat fnr = (gfloat) GGOBI_STAGE(d)->n_rows;

  d->subset.random_n = GGOBI_STAGE(d)->n_rows;

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
subset_set_all(GGobiData *d, gboolean value) {
  gint i;

  for (i=0; i<GGOBI_STAGE(d)->n_rows; i++)
    d->sampled.els[i] = value;
}

/*------------------------------------------------------------------*/

void
subset_apply (GGobiData *d) {
  ggobi_data_set_rows_in_plot(d);
  if (d->gg->cluster_ui.window != NULL)
    cluster_table_update (d, d->gg);
}

/*
 * This algorithm taken from Knuth, Seminumerical Algorithms;
 * Vol 2 of his series.
*/
gboolean
subset_random (gint n, GGobiData *d) {
  gint t, m, top = GGOBI_STAGE(d)->n_rows;
  gfloat rrand;

  subset_set_all(d, false);

  if (n < 0 || n > top) 
    return false;

  for (t=0, m=0; t<top && m<n; t++) {
    rrand = (gfloat) randvalue ();
    if (((top - t) * rrand) < (n - m) && !d->sampled.els[t]) {
      d->sampled.els[t] = true;
      m++;            
    }
  }

  return true;
}

gboolean
subset_block (gint bstart, gint bsize, GGobiData *d)
{
  gint i, k;
  gboolean subsetsize = 0;

  if (bstart >= 0 && bstart < GGOBI_STAGE(d)->n_rows && bsize > 0) {
    subset_set_all(d, false);

    for (i=bstart, k=1; i<GGOBI_STAGE(d)->n_rows && k<=bsize; i++, k++) {
      d->sampled.els[i] = true;
      subsetsize++;
    }
  }

  if (subsetsize == 0)
    quick_message ("The limits aren't correctly specified.", false);
 
  return (subsetsize > 0);
}

gboolean
subset_range (GGobiData *d)
{
  gint i, j;
  gint subsetsize = 0;
  GGobiVariable *var;
  gboolean add;

  subset_set_all(d, false);

  for (i=0; i<GGOBI_STAGE(d)->n_rows; i++) {
    add = true;
    for (j=0; j<GGOBI_STAGE(d)->n_cols; j++) {
      var = ggobi_stage_get_variable(GGOBI_STAGE(d), j);
      if (var->lim_specified_p) {
        if (d->tform.vals[i][j] < var->lim_specified.min ||
            d->tform.vals[i][j] > var->lim_specified.max)
        {
          add = false;
        }
      }
    }
    if (add) {
      d->sampled.els[i] = true;
      subsetsize++;
    }
  }

  if (subsetsize == 0)
    quick_message ("Use the variable manipulation panel to set ranges.", false);
 
  return (subsetsize > 0);
}

gboolean
subset_everyn (gint estart, gint estep, GGobiData *d)
{
  gint i;
  gint top = GGOBI_STAGE(d)->n_rows;

  if (!(estart >= 0 && estart < top-1 && estep >= 0 && estep < top)) {
    quick_message ("Interval not correctly specified.", false);
    return(false);
  }
  
  subset_set_all(d, false);

  for(i = estart; i < top; i += estep)
    d->sampled.els[i] = true;
  
  return true;
}

/*-- create a subset of only the points with sticky ids --*/
/*-- Added by James Brook, Oct 1994 --*/
gboolean
subset_sticky (GGobiData *d)
{
  gint id;
  GSList *l;

  if (g_slist_length (d->sticky_ids) == 0)
    return false;
    
  subset_set_all(d, false);

  for (l = d->sticky_ids; l; l = l->next) {
    id = GPOINTER_TO_INT (l->data);
    if (id < GGOBI_STAGE(d)->n_rows)
      d->sampled.els[id] = true;
  }

  return true;
}

gboolean
subset_rowlab (gchar *substr, gint substr_pos, gboolean ignore_case,
  GGobiData *d)
{
  gint i;
  gssize slen;

  g_debug("Rowlab %s", substr);
  
  if (substr == NULL || (slen = g_utf8_strlen(substr, -1)) == 0)
    return false;

  subset_set_all(d, false);

  if (ignore_case)
    substr = g_utf8_strdown(substr, -1);
  else 
    substr = g_strdup(substr);

  for (i=0; i < GGOBI_STAGE(d)->n_rows; i++) {
    gchar *label = ggobi_stage_get_row_id(GGOBI_STAGE(d), i);
    
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
        d->sampled.els[i] = true;
    } else if (!g_utf8_collate(g_utf8_offset_to_pointer(label, start), substr))
        d->sampled.els[i] = true;
    g_free(label);
  }
  g_free(substr);
  return true;
}
