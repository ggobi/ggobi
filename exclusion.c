/* exclusion.c */
/*
 * This is no longer appropriately named: think of it as cluster.c
*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

void
symbol_table_zero (datad *d)
{
  gint j, k, m;

  for (j=0; j<NGLYPHTYPES; j++)
    for (k=0; k<NGLYPHSIZES; k++)
      for (m=0; m<NCOLORS; m++)
        d->symbol_table[j][k][m].n =
          d->symbol_table[j][k][m].nhidden = 
          d->symbol_table[j][k][m].nshown = 0;
}

/*
 * loop over colors within glyph types and sizes, and
 * populate d->symbol_table
*/
gint 
symbol_table_populate (datad *d)
{
  register gint i, j, k, m;
  gint nclusters = 0;

  symbol_table_zero (d);

  /*-- loop over all data --*/
  for (i=0; i<d->nrows; i++) {
    j = d->glyph[i].type;   g_assert (j>=0 && j<NGLYPHTYPES);
    k = d->glyph[i].size;   g_assert (k>=0 && k<NGLYPHSIZES);
    m = d->color.els[i];    g_assert (m>=0 && m<NCOLORS);
    if (d->symbol_table[j][k][m].n == 0) nclusters++;
    d->symbol_table[j][k][m].n++;
    if (d->hidden.els[i])
      d->symbol_table[j][k][m].nhidden++;
    else
      d->symbol_table[j][k][m].nshown++;
  }

  return nclusters;
}

void
clusters_set (datad *d, ggobid *gg) {
  gint i, j, k, m;
  gint n, nclusters;

  nclusters = symbol_table_populate (d);

  /*-- reallocate the array of cluster structures --*/
  d->clusv = (clusterd *) g_realloc (d->clusv, nclusters * sizeof (clusterd));

  /*
   * populate the clusv structures using the information in the
   * 3-d table of counts of each size/type/color combination.
  */
  n = 0;
  for (j=0; j<NGLYPHTYPES; j++) {
    for (k=0; k<NGLYPHSIZES; k++) {
      for (m=0; m<NCOLORS; m++) {
        if (d->symbol_table[j][k][m].n > 0) {
          d->clusv[n].glyphtype = j;  g_assert (j>=0 && j<NGLYPHTYPES);
          d->clusv[n].glyphsize = k;  g_assert (k>=0 && k<NGLYPHSIZES);
          d->clusv[n].color = m;      g_assert (m>=0 && m<NCOLORS);
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
  vectori_alloc_zero (&d->clusterid, d->nrows);

  if (nclusters != d->nrows) {
    for (i=0; i<d->nrows; i++) {
      for (n=0; n<nclusters; n++) {
        if (d->sampled.els[i]) {
          if (d->glyph[i].type == d->clusv[n].glyphtype &&
              d->glyph[i].size == d->clusv[n].glyphsize &&
              d->color.els[i] == d->clusv[n].color)
          {
            d->clusterid.els[i] = n;
            break;
          }
        }
      }
    }
  }

  d->nclusters = nclusters;
}
