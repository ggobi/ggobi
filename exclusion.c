/* exclusion.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

void
cluster_free (gint k, datad *d, ggobid *gg) {
  if (d->clusv[k].da) {
    gtk_widget_destroy (d->clusv[k].da);
    gtk_widget_destroy (d->clusv[k].lbl);
    gtk_widget_destroy (d->clusv[k].hide_tgl);
    gtk_widget_destroy (d->clusv[k].exclude_tgl);
  }
}

void
clusters_set (datad *d, ggobid *gg) {
  gint i, k, n, j;
  gboolean new_color, new_glyph, hidden, included, new_cluster;
  glyphv glyphs_used[NGLYPHS];
  gshort colors_used[NCOLORS];
  gint nglyphs_used, ncolors_used;
  gint nclust = 1;  /* There can never be 0 clusters */

  /*
   *  clusterid is the groups vector: an integer for each case,
   *  indicating its cluster membership
  */
  if (d->clusterids.nels != d->nrows) {
    vectori_realloc_zero (&d->clusterids, d->nrows);
  }

  /*
   * Find all glyphs and colors used among the cases in the current sample
  */
  colors_used[0] = d->color_ids[0];
  ncolors_used = 1;

  if (!gg->mono_p) {
    for (i=0; i<d->nrows; i++) {
      if (d->sampled[i]) {  /*-- if in current sample (subset) --*/
        new_color = true;
        for (k=0; k<ncolors_used; k++) {
          if (colors_used[k] == d->color_ids[i]) {
            new_color = false;
            break;
          }
        }
        if (new_color) {
          colors_used[ncolors_used] = d->color_ids[i];
          ncolors_used++;
        }
      }
    }
  }

  glyphs_used[0].type = d->glyph_ids[0].type;
  glyphs_used[0].size = d->glyph_ids[0].size;
  nglyphs_used = 1;
  for (i=0; i<d->nrows; i++) {
    if (d->sampled[i]) {  /*-- if in current sample (subset) --*/
      new_glyph = true;
      for (k=0; k<nglyphs_used; k++) {
        if (glyphs_used[k].type == d->glyph_ids[i].type &&
            glyphs_used[k].size == d->glyph_ids[i].size)
        {
          new_glyph = false;
          break;
        }
      }
      if (new_glyph) {
        glyphs_used[nglyphs_used].type = d->glyph_ids[i].type;
        glyphs_used[nglyphs_used].size = d->glyph_ids[i].size;
        nglyphs_used++;
      }
    }
  }

  /*-- allocate the maximum possible number of clusters given the above --*/
  d->clusv = (clusterd *)
    g_realloc (d->clusv, ncolors_used * nglyphs_used * sizeof (clusterd));

  /*
   * Loop over glyphs and colors to find out how many
   * clusters there really are.
  */
  nclust = 0;
  for (k=0; k<nglyphs_used; k++) {
    for (n=0; n<ncolors_used; n++) {
      new_cluster = false;

      /*
       * Loop over all points, looking at glyph and color ids.
      */
      for (i=0; i<d->nrows; i++) {
        /*
         * If we find a pair ...
        */
        if (d->glyph_ids[i].type == glyphs_used[k].type &&
            d->glyph_ids[i].size == glyphs_used[k].size &&
            d->color_ids[i] == colors_used[n])
        {
          new_cluster = true;
          hidden = d->hidden[i];
          included = d->included[i];
          /*
           * make sure it's not already a member of clusv[]
          */
          for (j=0; j<nclust; j++) {
            if (d->clusv[j].glyphtype == glyphs_used[k].type &&
                d->clusv[j].glyphsize == glyphs_used[k].size &&
                d->clusv[j].color == colors_used[n])
            {
              new_cluster = false;
            }
          }
          break;
        }
      }

      if (new_cluster) {
        d->clusv[nclust].glyphtype = glyphs_used[k].type;
        d->clusv[nclust].glyphsize = glyphs_used[k].size;
        d->clusv[nclust].color = colors_used[n];
        d->clusv[nclust].hidden = hidden;
        d->clusv[nclust].included = included;
        d->clusv[nclust].n = 0;
        nclust++;
      }
    }
  }

  /*
   * If there are clusters, set the data in the groups column
  */
  for (n=0; n<nclust; n++) {
    for (i=0; i<d->nrows; i++) {
      if (d->sampled[i]) {
        if (d->glyph_ids[i].type == d->clusv[n].glyphtype &&
            d->glyph_ids[i].size == d->clusv[n].glyphsize &&
            d->color_ids[i] == d->clusv[n].color)
        {
          d->clusterids.vals[i] = n;
        }
      }
    }
  }
  d->nclusters = nclust;

  /*
   * Force all members of the same cluster to have the same hidden
   * and included status, which was set according to the first cluster
   * member encountered.  This is also a good time to count group
   * membership.
  */
  if (nclust > 1) {
    for (i=0; i<d->nrows; i++) {
      if (d->sampled[i]) {
        k = d->clusterids.vals[i];

        d->hidden[i] = d->clusv[k].hidden;
        d->included[i] = d->clusv[k].included;
        d->clusv[k].n++;
      }
    }
  }

  /*-- realloc to free any free extra clusters that may have been allocated --*/
  d->clusv = (clusterd *) g_realloc ((gpointer) d->clusv,
    nclust * sizeof (clusterd));
}
