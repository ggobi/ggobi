/* exclusion.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

void
cluster_free (gint k) {
  gtk_widget_destroy (xg.clusv[k].da);
  gtk_widget_destroy (xg.clusv[k].hide_tgl);
  gtk_widget_destroy (xg.clusv[k].exclude_tgl);
}

void
clusters_set (void) {
  gint i, k, n, j;
  gboolean new_color, new_glyph, hidden, included, new_cluster;
  glyphv glyphs_used[NGLYPHS];
  gshort colors_used[NCOLORS+2];
  gint nglyphs_used, ncolors_used;
  gint nclust = 1;  /* There can never be 0 clusters */

  /*-- this is the groups variable and should become part of ggobi.h --*/
  gint *clusterid = (gint *) g_malloc0 (xg.nlinkable * sizeof (gint));

/*
 * Find all glyphs and colors used among the cases in the current sample
*/
  colors_used[0] = xg.color_ids[0];
  ncolors_used = 1;
  if (!xg.mono_p) {
    for (i=0; i<xg.nlinkable; i++) {
      if (xg.sampled[i]) {  /*-- if in current sample (subset) --*/
        new_color = true;
        for (k=0; k<ncolors_used; k++) {
          if (colors_used[k] == xg.color_ids[i]) {
            new_color = false;
            break;
          }
        }
        if (new_color) {
          colors_used[ncolors_used] = xg.color_ids[i];
          ncolors_used++;
        }
      }
    }
  }

  glyphs_used[0].type = xg.glyph_ids[0].type;
  glyphs_used[0].size = xg.glyph_ids[0].size;
  nglyphs_used = 1;
  for (i=0; i<xg.nlinkable; i++) {
    if (xg.sampled[i]) {  /*-- if in current sample (subset) --*/
      new_glyph = true;
      for (k=0; k<nglyphs_used; k++) {
        if (glyphs_used[k].type == xg.glyph_ids[i].type &&
            glyphs_used[k].size == xg.glyph_ids[i].size)
        {
          new_glyph = false;
          break;
        }
      }
      if (new_glyph) {
        glyphs_used[nglyphs_used].type = xg.glyph_ids[i].type;
        glyphs_used[nglyphs_used].size = xg.glyph_ids[i].size;
        nglyphs_used++;
      }
    }
  }
  /*-- allocate the maximum possible number of clusters given the above --*/
  xg.clusv = (clusterd *)
    g_realloc (xg.clusv, ncolors_used * nglyphs_used * sizeof (clusterd));

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
      for (i=0; i<xg.nlinkable; i++) {
        /*
         * If we find a pair ...
        */
        if (xg.glyph_ids[i].type == glyphs_used[k].type &&
            xg.glyph_ids[i].size == glyphs_used[k].size &&
            xg.color_ids[i] == colors_used[n])
        {
          new_cluster = true;
          hidden = xg.hidden[i];
          included = xg.included[i];
          /*
           * make sure it's not already a member of clusv[]
          */
          for (j=0; j<nclust; j++) {
            if (xg.clusv[j].glyphtype == glyphs_used[k].type &&
                xg.clusv[j].glyphsize == glyphs_used[k].size &&
                xg.clusv[j].color == colors_used[n])
            {
              new_color = false;
            }
          }
          break;
        }
      }

      if (new_cluster) {
        xg.clusv[nclust].glyphtype = glyphs_used[k].type;
        xg.clusv[nclust].glyphsize = glyphs_used[k].size;
        xg.clusv[nclust].color = colors_used[n];
        xg.clusv[nclust].hidden = hidden;
        xg.clusv[nclust].included = included;
        xg.clusv[nclust].n = 0;
        nclust++;
      }
    }
  }

  /*
   * If there are clusters, set the data in the groups column
  */
  for (n=0; n<nclust; n++) {
    for (i=0; i<xg.nlinkable; i++) {
      if (xg.sampled[i]) {
        if (xg.glyph_ids[i].type == xg.clusv[n].glyphtype &&
            xg.glyph_ids[i].size == xg.clusv[n].glyphsize &&
            xg.color_ids[i] == xg.clusv[n].color)
        {
          clusterid[i] = n;
      }
      }
    }
  }
  xg.nclust = nclust;

  /*
   * Force all members of the same cluster to have the same hidden
   * and included status, which was set according to the first cluster
   * member encountered.  This is also a good time to count group
   * membership.
  */
  if (nclust > 1) {
    for (i=0; i<xg.nlinkable; i++) {
      if (xg.sampled[i]) {
        k = clusterid[i];
        xg.hidden[i] = xg.clusv[k].hidden;
        xg.included[i] = xg.clusv[k].included;

        xg.clusv[i].n++;
      }
    }
  }

  /*-- realloc to any free extra clusters that may have been allocated --*/
  xg.clusv = (clusterd *) g_realloc (xg.clusv, nclust * sizeof (clusterd *));

  g_free ((gpointer) clusterid);
}
