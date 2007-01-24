#include <sys/types.h>

#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include "plugin.h"
#include "ggvis.h"

void
create_shepard_data_cb (GtkAction *action, PluginInstance *inst)
/*
 * Create a new datad containing the Shepard plot describing the
 * distance matrix.
*/
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggobid *gg = inst->gg;
  gint i, j;
  gchar **colnames, **rownames;
  gint n, nr, nc = 7;
  static gchar *clab[] = {"d_ij", "f(D_ij)", "D_ij", "Res_ij", "Wgt_ij", "i", "j"};
  static gchar *blab[] = {"b_ij", "f(D_ij)", "D_ij", "Res_ij", "Wgt_ij", "i", "j"};
  InputDescription *desc = NULL;
  gdouble *values;

  if (ggv->dpos == NULL) {
    g_printerr ("For now, run mds first ...\n");
    return;
  }

  colnames = (gchar **) g_malloc(nc * sizeof (gchar *));
  values = (gdouble *) g_malloc (ggv->num_active_dist * nc * sizeof(gdouble));
  rownames = (gchar **) g_malloc (ggv->num_active_dist * sizeof(gchar *));

  for (j=0; j<nc; j++)
    colnames[j] = (ggv->KruskalShepard_classic == KruskalShepard) ?
      g_strdup (clab[j]) : g_strdup (blab[j]);

  mds_once (false, ggv, gg);

  nr = ggv->num_active_dist;
  n = 0;
  for (i = 0; i < ggv->Dtarget.nrows; i++) {
    for (j = 0; j < ggv->Dtarget.ncols; j++) {
      if (ggv->trans_dist.els[IJ] == G_MAXDOUBLE)
        continue;
      else {
        if (n == nr) {
          g_printerr ("too many distances: n %d nr %d\n", n, nr);
          break;
        }
        values[n + 0*nr] = ggv->config_dist.els[IJ];
        values[n + 1*nr] = ggv->trans_dist.els[IJ];
        values[n + 2*nr] = ggv->Dtarget.vals[i][j];
        /* residual */
        values[n + 3*nr] = ggv->trans_dist.els[IJ] - ggv->config_dist.els[IJ];
        /* weight */
        values[n + 4*nr] =
          (ggv->weight_power == 0. && ggv->within_between == 1.) ? 1.0 :
          ggv->weights.els[IJ];
        values[n + 5*nr] = (gdouble) i;
        values[n + 6*nr] = (gdouble) j;

        rownames[n] = g_strdup_printf ("%s|%s",
          (gchar *) g_array_index (ggv->dsrc->rowlab, gchar *, i),
          (gchar *) g_array_index (ggv->dsrc->rowlab, gchar *, j));

        n++;
      }
    }
  }

  if (n) {
    displayd *dspnew;
    GGobiData *dnew;

    ggv->shepard_iter++;

    dnew = ggobi_data_new (n, nc);
    dnew->name = g_strdup_printf ("Shepard Plot %d", ggv->shepard_iter);

    GGOBI(setData) (values, rownames, colnames, n, nc, dnew,
      false, gg, /*rowids*/NULL, false, desc);  /*no rowids to start */
	
    /* Because n tends to be quite large, and Windows drawing is
     * so slow, use a single pixel point. (Suggestion made by
     * Brian Ripley) */
    for (i=0; i<n; i++)
      dnew->glyph.els[i].type = dnew->glyph.els[i].size = 
        dnew->glyph_now.els[i].type = dnew->glyph_now.els[i].size = 
	dnew->glyph_prev.els[i].type = dnew->glyph_prev.els[i].size = 0;

    dspnew = GGOBI(newScatterplot) (0, 1, dnew, gg); 
    display_add(dspnew, gg);
    varpanel_refresh(dspnew, gg);
    display_tailpipe (dspnew, FULL, gg);
  }

  g_free (rownames);
  g_free (colnames);
  g_free (values);

/*
  * variable groups file: keep variables "i" and "j" on same scale *
  fprintf(fpvgrp, "1\n2\n3\n4\n5\n6\n6\n");
*/
  

}
