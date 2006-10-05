/* exclusion.c -- for manipulating clusters */
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

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

//FIXME:  I think this can be considerably simplified by using
// a hash function that takes a colour, and glyph and returns 
// a unique identifier (maybe a double)
//
// This will also make it more flexibile if we introduce new
// attribute mappings or make colour or size continuous.
//
// The functionality of this file could then be moved into data.gob
// and this function deleted.

void
symbol_table_zero (GGobiStage * d)
{
  gint j, k, m;

  for (j = 0; j < NGLYPHTYPES; j++)
    for (k = 0; k < NGLYPHSIZES; k++)
      for (m = 0; m < MAXNCOLORS; m++)
        d->symbol_table[j][k][m].n =
          d->symbol_table[j][k][m].nhidden =
          d->symbol_table[j][k][m].nshown = 0;
}

/*
 * loop over colors within glyph types and sizes, and
 * populate d->symbol_table
*/
gint
symbol_table_populate (GGobiStage * d)
{
  gint i, j, k, m;
  gint nclusters = 0;

  symbol_table_zero (d);

  GGOBI_STAGE_ATTR_INIT_ALL(d);  
  /*-- loop over all data --*/
  for (i = 0; i < d->n_rows; i++) {
    j = GGOBI_STAGE_GET_ATTR_TYPE(d, i);
    k = GGOBI_STAGE_GET_ATTR_SIZE(d, i);
    m = GGOBI_STAGE_GET_ATTR_COLOR(d, i);

    if (d->symbol_table[j][k][m].n == 0)
      nclusters++;

    d->symbol_table[j][k][m].n++;

    if (GGOBI_STAGE_GET_ATTR_HIDDEN(d, i))
      d->symbol_table[j][k][m].nhidden++;
    else
      d->symbol_table[j][k][m].nshown++;
  }

  return nclusters;
}

void
clusters_set (GGobiStage * d)
{
  guint i, j, k, m;
  gint n, nclusters;
  colorschemed *scheme;

  if (!GGOBI_IS_GGOBI(d->gg))
    return;
  scheme = d->gg->activeColorScheme;

  nclusters = symbol_table_populate (d);

  /*-- reallocate the array of cluster structures --*/
  d->clusv = (clusterd *)
    g_realloc (d->clusv, nclusters * sizeof (clusterd));

  /*
   * make sure new clusters are not excluded, without changing the
   * status of pre-existing clusters.
   */
  for (i = d->nclusters; i < nclusters; i++)
    d->clusv[i].hidden_p = false;

  GGOBI_STAGE_ATTR_INIT_ALL(d);  

  /*
   * populate the clusv structures using the information in the
   * 3-d table of counts of each size/type/color combination.
   */
  n = 0;
  for (j = 0; j < NGLYPHTYPES; j++) {
    for (k = 0; k < NGLYPHSIZES; k++) {
      for (m = 0; m < scheme->n; m++) {
        if (d->symbol_table[j][k][m].n > 0) {
          d->clusv[n].glyphtype = j;
          g_assert (j >= 0 && j < NGLYPHTYPES);
          d->clusv[n].glyphsize = k;
          g_assert (k >= 0 && k < NGLYPHSIZES);
          d->clusv[n].color = m;
          g_assert (m >= 0 && m < MAXNCOLORS);
          d->clusv[n].nhidden = d->symbol_table[j][k][m].nhidden;
          d->clusv[n].nshown = d->symbol_table[j][k][m].nshown;
          d->clusv[n].n = d->symbol_table[j][k][m].n;
          n++;
        }
      }
    }
  }

  /*
   *  clusterid is the groups vector: an integer for each case,
   *  indicating its cluster membership
   */
  if (nclusters > 0 && nclusters != 1) {
    for (i = 0; i < d->n_rows; i++) {
      for (n = 0; n < nclusters; n++) {
        if (GGOBI_STAGE_GET_ATTR_SAMPLED(d, i)) {
          if (GGOBI_STAGE_GET_ATTR_TYPE(d, i) == d->clusv[n].glyphtype &&
              GGOBI_STAGE_GET_ATTR_SIZE(d, i) == d->clusv[n].glyphsize &&
              GGOBI_STAGE_GET_ATTR_COLOR(d, i) == d->clusv[n].color) {
            d->clusterid.els[i] = n;
            break;
          }
        }
      }
    }
  }

  d->nclusters = nclusters;
}
