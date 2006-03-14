#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>
#include <math.h>

#include "plugin.h"
#include "defines.h"
#include "ggvis.h"

void
ggv_init_Dtarget (gint selected_var, ggvisd *ggv)
{
  gint i, j;
  gdouble infinity, largest = -1;
  GGobiData *e = ggv->e;
  gint indx = -1;

  /*-- initalize Dtarget --*/
  infinity = (gdouble) (2 * ggv->Dtarget.nrows);
  if (selected_var >= 0 && selected_var < e->tform.ncols) {
    largest = e->tform.vals[0][selected_var];
    for (i=0; i<e->edge.n; i++) {
      if (e->tform.vals[i][selected_var] > infinity) {
        infinity = e->tform.vals[i][selected_var];
        indx = i;
      }
      if (e->tform.vals[i][selected_var] > largest) {
        largest = e->tform.vals[i][selected_var];
      }
    }
  }

  /* Report the value of largest */
  if (largest != -1) {
    g_printerr ("largest dissimilarity: %.3f\n", largest);
    if (largest > 100000) {
      gchar *stmp = g_strdup_printf ("Warning: your largest weight, %.2f (index %d), is extremely large. ", largest, indx);
      quick_message (stmp, false);
      g_free (stmp);
    }
  }

/* Continue to initialize using the value of infinity.  Is that ok? */
  for (i=0; i<ggv->Dtarget.nrows; i++) {
    for (j=0; j<ggv->Dtarget.ncols; j++)
      ggv->Dtarget.vals[i][j] = infinity;
    ggv->Dtarget.vals[i][i] = 0.0;
  }
}

void
ggv_compute_Dtarget (gint selected_var, ggvisd *ggv)
{
  GGobiData *e, *dsrc;
  endpointsd *endpoints;
  gboolean changing;
  gint end1, end2, end3;
  gdouble d12;
  gdouble **Dvals;
  gdouble dtmp;
  gint i, j, nsteps;

  dsrc = ggv->dsrc;
  e = ggv->e;
  Dvals = ggv->Dtarget.vals;
  endpoints = resolveEdgePoints(e, dsrc);

  /*-- populate --*/
  if (!ggv->complete_Dtarget) {
    for (i = 0; i < e->edge.n; i++) {
      end1 = endpoints[i].a;
      end2 = endpoints[i].b;
      Dvals[end1][end2] = 
       (ggv->mds_task == DissimAnalysis || ggv->Dtarget_source == VarValues) ?
        e->tform.vals[i][selected_var] : 1.0;
    }
  } else {  /*-- complete Dtarget using a shortest path algorithm --*/

    nsteps = 0;
    changing = true;
    while (changing) {
      changing = false;
      for (i = 0; i < e->edge.n; i++) {
        /*end1 = dsrc->rowid.idv.els[endpoints[i].a];*/
        /*end2 = dsrc->rowid.idv.els[endpoints[i].b];*/
        end1 = endpoints[i].a;
        end2 = endpoints[i].b;
        d12 = 
          (ggv->mds_task == DissimAnalysis || ggv->Dtarget_source == VarValues) ?
          e->tform.vals[i][selected_var] : 1.0;
        if (d12 < 0) {
          g_printerr ("Re-setting negative dissimilarity to zero: index %d, value %f\n",
            i, d12);
          d12 = 0;
        }

        for (end3 = 0; end3 < dsrc->nrows; end3++) {
          /* So we have a direct link from end1 to end2.  Can this be */
          /* used to shortcut a path from end1 to end3 or end2 to end3? */
          if (end3 != end1 && Dvals[end1][end3] > d12 + Dvals[end2][end3]) {
            Dvals[end3][end1] = Dvals[end1][end3] = d12 + Dvals[end2][end3];
            changing = true;
          }
          if (end3 != end2 && Dvals[end2][end3] > d12 + Dvals[end1][end3]) {
            Dvals[end3][end2] = Dvals[end2][end3] = d12 + Dvals[end1][end3];
            changing = true;
          }
        }    /* end3 */
      }    /* end1 and end2 */
      nsteps++;
      if (nsteps > 10) {
        g_printerr ("looping too many times; something's wrong ...\n");
        break;
      }
    }    /* while changing. */
  }

/*
{
gint n = (ggv->Dtarget.nrows < 10)?ggv->Dtarget.nrows:10;
g_printerr ("n: %d\n", n);
for (i=0; i<n; i++) {
  for (j=0; j<n; j++) {
    g_printerr ("%.2f ", Dvals[i][j]);
  }
  g_printerr ("\n");
}
}
*/

  ggv->ndistances = ggv->Dtarget.nrows * ggv->Dtarget.ncols;

  ggv->Dtarget_max = -G_MAXDOUBLE;  ggv->Dtarget_min = G_MAXDOUBLE;
  for (i=0; i<ggv->Dtarget.nrows; i++) {
    for (j=0; j<ggv->Dtarget.ncols; j++) {
      dtmp = ggv->Dtarget.vals[i][j]; 
      if (dtmp < 0) {
        g_printerr ("negative dissimilarity: D[%d][%d] = %3.6f -> NA\n",
          i, j, dtmp);
        dtmp = ggv->Dtarget.vals[i][j] = G_MAXDOUBLE;
      }
      if(dtmp != G_MAXDOUBLE) {
        if (dtmp > ggv->Dtarget_max) ggv->Dtarget_max = dtmp;
        if (dtmp < ggv->Dtarget_min) ggv->Dtarget_min = dtmp;
      }
    }
  }
  ggv->threshold_low =  ggv->Dtarget_min;
  ggv->threshold_high = ggv->Dtarget_max;
}
