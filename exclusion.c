/* exclusion.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

void
cluster_free (gint k, ggobid *gg) {
  if (gg->clusv[k].da) {
    gtk_widget_destroy (gg->clusv[k].da);
    gtk_widget_destroy (gg->clusv[k].lbl);
    gtk_widget_destroy (gg->clusv[k].hide_tgl);
    gtk_widget_destroy (gg->clusv[k].exclude_tgl);
  }
}


void
clusters_set (ggobid *gg) {
  gint i, k, n, j;
  gboolean new_color, new_glyph, hidden, included, new_cluster;
  glyphv glyphs_used[NGLYPHS];
  gshort colors_used[NCOLORS];
  gint nglyphs_used, ncolors_used;
  gint nclust = 1;  /* There can never be 0 clusters */

  /*-- this is the groups variable and should become part of ggobi.h --*/

  if (gg->clusterid.nels != gg->nrows) {
    vectori_realloc_zero (&gg->clusterid, gg->nrows);
  }

  /*
   * Find all glyphs and colors used among the cases in the current sample
  */
  colors_used[0] = gg->color_ids[0];
  ncolors_used = 1;

  if (!gg->mono_p) {
    for (i=0; i<gg->nrows; i++) {
      if (gg->sampled[i]) {  /*-- if in current sample (subset) --*/
        new_color = true;
        for (k=0; k<ncolors_used; k++) {
          if (colors_used[k] == gg->color_ids[i]) {
            new_color = false;
            break;
          }
        }
        if (new_color) {
          colors_used[ncolors_used] = gg->color_ids[i];
          ncolors_used++;
        }
      }
    }
  }

  glyphs_used[0].type = gg->glyph_ids[0].type;
  glyphs_used[0].size = gg->glyph_ids[0].size;
  nglyphs_used = 1;
  for (i=0; i<gg->nrows; i++) {
    if (gg->sampled[i]) {  /*-- if in current sample (subset) --*/
      new_glyph = true;
      for (k=0; k<nglyphs_used; k++) {
        if (glyphs_used[k].type == gg->glyph_ids[i].type &&
            glyphs_used[k].size == gg->glyph_ids[i].size)
        {
          new_glyph = false;
          break;
        }
      }
      if (new_glyph) {
        glyphs_used[nglyphs_used].type = gg->glyph_ids[i].type;
        glyphs_used[nglyphs_used].size = gg->glyph_ids[i].size;
        nglyphs_used++;
      }
    }
  }

  /*-- allocate the maximum possible number of clusters given the above --*/
  gg->clusv = (clusterd *)
    g_realloc (gg->clusv, ncolors_used * nglyphs_used * sizeof (clusterd));

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
      for (i=0; i<gg->nrows; i++) {
        /*
         * If we find a pair ...
        */
        if (gg->glyph_ids[i].type == glyphs_used[k].type &&
            gg->glyph_ids[i].size == glyphs_used[k].size &&
            gg->color_ids[i] == colors_used[n])
        {
          new_cluster = true;
          hidden = gg->hidden[i];
          included = gg->included[i];
          /*
           * make sure it's not already a member of clusv[]
          */
          for (j=0; j<nclust; j++) {
            if (gg->clusv[j].glyphtype == glyphs_used[k].type &&
                gg->clusv[j].glyphsize == glyphs_used[k].size &&
                gg->clusv[j].color == colors_used[n])
            {
              new_cluster = false;
            }
          }
          break;
        }
      }

      if (new_cluster) {
        gg->clusv[nclust].glyphtype = glyphs_used[k].type;
        gg->clusv[nclust].glyphsize = glyphs_used[k].size;
        gg->clusv[nclust].color = colors_used[n];
        gg->clusv[nclust].hidden = hidden;
        gg->clusv[nclust].included = included;
        gg->clusv[nclust].n = 0;
        nclust++;
      }
    }
  }

  /*
   * If there are clusters, set the data in the groups column
  */
  for (n=0; n<nclust; n++) {
    for (i=0; i<gg->nrows; i++) {
      if (gg->sampled[i]) {
        if (gg->glyph_ids[i].type == gg->clusv[n].glyphtype &&
            gg->glyph_ids[i].size == gg->clusv[n].glyphsize &&
            gg->color_ids[i] == gg->clusv[n].color)
        {
          gg->clusterid.data[i] = n;
        }
      }
    }
  }
  gg->nclust = nclust;

  /*
   * Force all members of the same cluster to have the same hidden
   * and included status, which was set according to the first cluster
   * member encountered.  This is also a good time to count group
   * membership.
  */
  if (nclust > 1) {
    for (i=0; i<gg->nrows; i++) {
      if (gg->sampled[i]) {
        k = gg->clusterid.data[i];

        gg->hidden[i] = gg->clusv[k].hidden;
        gg->included[i] = gg->clusv[k].included;
        gg->clusv[k].n++;
      }
    }
  }

  /*-- realloc to any free extra clusters that may have been allocated --*/
  for (n=nclust; n<(ncolors_used * nglyphs_used); n++)
    cluster_free (k, gg);
  gg->clusv = (clusterd *) g_realloc ((gpointer) gg->clusv,
    nclust * sizeof (clusterd));
}
