#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <string.h>

#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include "plugin.h"
#include "varcloud.h"

void
launch_varcloud_cb (GtkWidget *w, PluginInstance *inst)
/*
 * Create a new datad containing the pairwise distance variables.
*/
{
  vcld *vcl = vclFromInst (inst);
  ggobid *gg = inst->gg;
  gint i, j, k, ii, jj;
  gchar **colnames, **rownames, **recordids;
  gint npairs, n, nc = 4;
  static gchar *clab[] = {"D_ij", "diff_ij", "i", "j"};
  InputDescription *desc = NULL;
  gdouble *values;
  datad *dsrc = vcl->dsrc, *e, *dnew;
  gdouble xci, xcj, yci, ycj;
  gchar *lbl;

  if (dsrc->nrows <= 1)
    return;

  /* lower triangle only */
  npairs = dsrc->nrows_in_plot*(dsrc->nrows_in_plot-1)/2;  
 
  /* Step 1: if necessary, add record ids to the original datad */
  /*    Keep it simple: use row numbers */
  datad_record_ids_set(dsrc, NULL, false);

  /* Step 2: if necessary, add an edge set for the complete graph.
      Call it 'allpairs'; it has no variables for now.
      This too needs record ids so it can be linked to the new data,
      and it needs rowlabels so that we can do linked brushing.
  */

  recordids = (gchar **) g_malloc (npairs * sizeof(gchar *));

  e = datad_create(npairs, 0, gg);
  e->name = g_strdup("all pairs");
  rowlabels_alloc(e);

  k = 0;
  for (i=0; i<dsrc->nrows_in_plot-1; i++)
    for (j=i+1; j<dsrc->nrows_in_plot; j++) {
      lbl = g_strdup_printf ("%d,%d", 
        dsrc->rows_in_plot.els[i], 
        dsrc->rows_in_plot.els[j]);
      recordids[k++] = lbl;
      g_array_append_val(e->rowlab, lbl);
    }
      
  datad_record_ids_set(e, recordids, false);
  pipeline_init(e, gg);

  edges_alloc (npairs, e);
  e->edge.sym_endpoints = (SymbolicEndpoints *)
     g_malloc(sizeof(SymbolicEndpoints) * e->edge.n);

  k = 0;
  for (i=0; i<dsrc->nrows_in_plot-1; i++) {
    for (j=i+1; j<dsrc->nrows_in_plot; j++) {
      ii = dsrc->rows_in_plot.els[i];
      jj = dsrc->rows_in_plot.els[j];
      e->edge.sym_endpoints[k].a = dsrc->rowIds[ii];
      e->edge.sym_endpoints[k].b = dsrc->rowIds[jj];
      e->edge.sym_endpoints[k].jpartner = -1;
      k++;
    }
  }

  /* Update the current display, which is presumably a scatterplot of
     y vs x
   */
  unresolveAllEdgePoints(e);
  if(gg->current_display) {
    edgeset_add(gg->current_display);
    displays_plot(NULL, FULL, gg);
  }
  gdk_flush();


  /* Step 3: Create the new dataset, npairs by nc */
  /*   The new data has to have the same record ids as the edges */

  colnames = (gchar **) g_malloc(nc * sizeof (gchar *));
  values = (gdouble *) g_malloc (npairs * nc * sizeof(gdouble));
  rownames = (gchar **) g_malloc (npairs * sizeof(gchar *));

  for (j=0; j<nc; j++)
    colnames[j] = g_strdup (clab[j]);

  n = 0;
  for (i = 0; i<dsrc->nrows_in_plot-1; i++) {
    for (j = i+1; j<dsrc->nrows_in_plot; j++) {
      if (n == npairs) {
        g_printerr ("too many distances: n %d nr %d\n", n, npairs);
        break;
      }
      /* Verify that each of these indices points to something real */
      ii = dsrc->rows_in_plot.els[i];
      jj = dsrc->rows_in_plot.els[j];
      xci = dsrc->tform.vals[ii][vcl->xcoord];
      yci = dsrc->tform.vals[ii][vcl->ycoord];
      xcj = dsrc->tform.vals[jj][vcl->xcoord];
      ycj = dsrc->tform.vals[jj][vcl->ycoord];
      values[n + 0*npairs] = sqrt((xci-xcj)*(xci-xcj) + (yci-ycj)*(yci-ycj));
      values[n + 1*npairs] = sqrt(fabs((gdouble)(dsrc->tform.vals[ii][vcl->var1] - 
						 dsrc->tform.vals[jj][vcl->var1])));
      values[n + 2*npairs] = (gdouble) ii;
      values[n + 3*npairs] = (gdouble) jj;

      rownames[n] = g_strdup_printf ("%s,%s",
        (gchar *) g_array_index (dsrc->rowlab, gchar *, ii),
        (gchar *) g_array_index (dsrc->rowlab, gchar *, jj));

      n++;
    }
  }

  if (n) {
    displayd *dspnew;

    dnew = datad_create (n, nc, gg);
    dnew->name = "VarCloud";
    GGOBI(setData) (values, rownames, colnames, n, nc, dnew,
		    false, gg, recordids, true, desc); 

    /* Open the new display */
    /* Now why does this new display have an Edges menu?  Something is
       still wrong with record ids, I fear. */
    dspnew = GGOBI(newScatterplot) (0, 1, dnew, gg);
    display_tailpipe (dspnew, FULL, gg);
  }

  g_free (rownames);
  g_free (colnames);
  g_free (values);
  g_free (recordids);


}
