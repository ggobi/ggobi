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
 * Juergen says there should be points corresponding to the edges {i=j}.
 * I haven't added them yet, but I could do so -- they don's have to be
 * linked to any edges, I don't think.
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
  GGobiData *d = vcl->dsrc, *e, *dnew;
  gint var1 = vcl->var1, var2 = vcl->var2;
  gdouble xci, xcj, yci, ycj;
  gchar *lbl;
  const gchar *name = gtk_widget_get_name (w);

  /*
     If widget name is 'Cross', this should be a cross-variogram
     cloud plot; make sure that vcl->var1 != vcl->var2.
  */
  if (strcmp (name, "Cross") == 0) {
    if (var1 == var2) {
      quick_message("For a cross-variogram plot, Variable 1 should be different from Variable 2", false);
/**/  return;
    }
  } else var2 = var1;

  if (d->nrows <= 1)
    return;

  /* upper and lower triangle -- but skip the diagonal for now */
  npairs = d->nrows_in_plot*(d->nrows_in_plot-1);  
 
  /* Step 1: if necessary, add record ids to the original GGobiData */
  /*    Keep it simple: use row numbers */
  datad_record_ids_set(d, NULL, false);

  /* Step 2: if necessary, add an edge set for the complete graph.
      Call it 'allpairs'; it has no variables for now.
      This too needs record ids so it can be linked to the new data,
      and it needs rowlabels so that we can do linked brushing.
  */

  recordids = (gchar **) g_malloc (npairs * sizeof(gchar *));

  e = ggobi_data_new(npairs, 0);
  e->name = g_strdup("all pairs");
  rowlabels_alloc(e);

  k = 0;
  for (i=0; i<d->nrows_in_plot; i++)
    for (j=0; j<d->nrows_in_plot; j++) {
      if (i == j) 
        continue;
      lbl = g_strdup_printf ("%d,%d", 
        d->rows_in_plot.els[i], 
        d->rows_in_plot.els[j]);
      recordids[k++] = lbl;
      g_array_append_val(e->rowlab, lbl);
    }
      
  datad_record_ids_set(e, recordids, false);
  pipeline_init(e, gg);

  edges_alloc (npairs, e);
  e->edge.sym_endpoints = (SymbolicEndpoints *)
     g_malloc(sizeof(SymbolicEndpoints) * e->edge.n);

  k = 0;
  for (i=0; i<d->nrows_in_plot; i++) {
    for (j=0; j<d->nrows_in_plot; j++) {
      if (i == j)
        continue;
      ii = d->rows_in_plot.els[i];
      jj = d->rows_in_plot.els[j];
      e->edge.sym_endpoints[k].a = d->rowIds[ii];
      e->edge.sym_endpoints[k].b = d->rowIds[jj];
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
  for (i = 0; i<d->nrows_in_plot; i++) {
    for (j = 0; j<d->nrows_in_plot; j++) {
      if (i == j)
        continue;
      if (n == npairs) {
        g_printerr ("too many distances: n %d nr %d\n", n, npairs);
        break;
      }

      /* Verify that each of these indices points to something real */
      ii = d->rows_in_plot.els[i];
      jj = d->rows_in_plot.els[j];
      xci = d->tform.vals[ii][vcl->xcoord];
      yci = d->tform.vals[ii][vcl->ycoord];
      xcj = d->tform.vals[jj][vcl->xcoord];
      ycj = d->tform.vals[jj][vcl->ycoord];
      values[n + 0*npairs] = sqrt((xci-xcj)*(xci-xcj) + (yci-ycj)*(yci-ycj));
      values[n + 1*npairs] = sqrt(fabs((gdouble)(d->tform.vals[ii][var1] - 
						 d->tform.vals[jj][var2])));
      values[n + 2*npairs] = (gdouble) ii;
      values[n + 3*npairs] = (gdouble) jj;

      rownames[n] = g_strdup_printf ("%s,%s",
        (gchar *) g_array_index (d->rowlab, gchar *, ii),
        (gchar *) g_array_index (d->rowlab, gchar *, jj));

      n++;
    }
  }

  if (n) {
    displayd *dspnew;

    dnew = ggobi_data_new (n, nc);
    dnew->name = "VarCloud";
    GGOBI(setData) (values, rownames, colnames, n, nc, dnew,
		    false, gg, recordids, true, desc); 

    /* Open the new display */
    /* Now why does this new display have an Edges menu?  Something is
       still wrong with record ids, I fear. */
    dspnew = GGOBI(newScatterplot) (0, 1, dnew, gg);
    display_add(dspnew, gg);
    varpanel_refresh(dspnew, gg);
    display_tailpipe (dspnew, FULL, gg);
  }

  g_free (rownames);
  g_free (colnames);
  g_free (values);
  g_free (recordids);


}


void
close_vcl_window_cb (GtkWidget *w, PluginInstance *inst)
{
  extern void freePlugin(ggobid *, PluginInstance *);
  ggobid *gg = inst->gg;

  freePlugin(gg, inst);
}
