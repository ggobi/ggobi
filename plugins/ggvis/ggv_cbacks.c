#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "defines.h"
#include "ggvis.h"

/*
 * Definition of D
*/
void ggv_dsource_cb (GtkWidget *w, gpointer cbd)
{
  PluginInstance *inst = (PluginInstance *)
     gtk_object_get_data (GTK_OBJECT (w), "PluginInst");
  ggvisd *ggv = ggvisFromInst (inst);

  ggv->Dtarget_source = (MDSDtargetSource) GPOINTER_TO_INT (cbd);

 g_printerr ("dsource = %d\n", ggv->Dtarget_source);
}
void ggv_complete_distances_cb (GtkToggleButton *button, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->Dtarget_source = button->active;
g_printerr ("complete distances? %d\n", ggv->Dtarget_source);
}

/*
 * This code should actually be moved out of the callback
 * and stashed someplace else.
*/
void ggv_compute_Dtarget_cb (GtkWidget *button, PluginInstance *inst)
{
  GtkWidget *notebook = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(button), "notebook");
  ggvisd *ggv = ggvisFromInst (inst);
  ggobid *gg = inst->gg;
  /*-- for the time being, obtain datad from the current display --*/
  datad *d = gg->current_display->d;
  datad *e = gg->current_display->e;
  GtkWidget *clist;
  gint i, j, selected_var;
  gdouble infinity;
  gboolean changing;
  gint end1, end2, end3;
  gdouble d12;
  gdouble **Dvals;
  endpointsd *endpoints;

/*
How big is this distance matrix?  if (ggv->complete_Dtarget), then
it can be larger than e->edge.n.    Perhaps it should be of
dimension d->nrows x d->nrows.
Sometimes it will be extremely sparse, but then we probably
aren't running ggvis on extremely large data, so maybe the
large size of it isn't important.
*/

  /*-- allocate --*/
  arrayd_alloc (&ggv->Dtarget, d->nrows, d->nrows);
  /*-- initalize --*/
  infinity = (gdouble) (2 * d->nrows);
  for (i=0; i<d->nrows; i++) {
    for (j=0; j<d->nrows; j++)
      ggv->Dtarget.vals[i][j] = infinity;
    ggv->Dtarget.vals[i][i] = 0.0;
  }

  if (ggv->Dtarget_source == VarValues) {
    clist = get_clist_from_object (GTK_OBJECT (button));
    if (!clist) {
      quick_message ("I can't identify a set of edges", false);
      return;
    }
    e = gtk_object_get_data (GTK_OBJECT(clist), "datad");
    if (!e) {
      quick_message ("I can't identify a set of edges", false);
      return;
    }
    selected_var = get_one_selection_from_clist (clist, e);
  }
  if (!e) {
    quick_message ("I can't identify a set of edges", false);
    return;
  }

  Dvals = ggv->Dtarget.vals;
  endpoints = e->edge.endpoints;

  /*-- populate --*/
  if (!ggv->complete_Dtarget) {
    for (i = 0; i < e->edge.n; i++) {
      end1 = d->rowid.idv.els[endpoints[i].a];
      end2 = d->rowid.idv.els[endpoints[i].b];
/*
      end1 = edges_arrp->data[i][0]-1;
      end2 = edges_arrp->data[i][1]-1;
*/
      Dvals[end1][end2] = (ggv->Dtarget_source == VarValues) ?
        e->tform.vals[i][selected_var] : 1.0;
    }
  } else {  /*-- complete Dtarget using a shortest path algorithm --*/

    while (changing) {
      changing = false;
      for (i = 0; i < e->edge.n; i++) {
        end1 = d->rowid.idv.els[endpoints[i].a];
        end2 = d->rowid.idv.els[endpoints[i].b];
/*
        end1 = edges_arrp->data[i][0]-1;
        end2 = edges_arrp->data[i][1]-1;
*/
        d12 = (ggv->Dtarget_source == VarValues) ?
          e->tform.vals[i][selected_var] : 1.0;

        for (end3 = 0; end3 < d->nrows; end3++) {
          /* So we have a direct link from end1 to end2.  Can this be */
          /* used to shortcut a path from end1 to end3 or end2 to end3? */
          if (Dvals[end1][end3] > d12 + Dvals[end2][end3]) {
            Dvals[end3][end1] = Dvals[end1][end3] = d12 + Dvals[end2][end3];
            changing = true;
          }
          if (Dvals[end2][end3] > d12 + Dvals[end1][end3]) {
            Dvals[end3][end2] = Dvals[end2][end3] = d12 + Dvals[end1][end3];
            changing = true;
          }
        }    /* end3 */
      }    /* end1 and end2 */
    }    /* while changing. */
  }
}


/*-- --*/

void ggv_stepsize_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->mds_stepsize = adj->value;
 g_printerr ("mds_stepsize = %f\n", ggv->mds_stepsize);
}

void ggv_dims_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->mds_dims = (gint) (adj->value);
 g_printerr ("mds_dims = %d\n", ggv->mds_dims);
}
void ggv_power_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->mds_power = adj->value;
 g_printerr ("mds_power = %f\n", ggv->mds_power);
}
void ggv_D_power_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->mds_D_power = adj->value;
 g_printerr ("mds_D_power = %f\n", ggv->mds_D_power);
}
void ggv_lnorm_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->mds_lnorm = adj->value;
 g_printerr ("mds_lnorm = %f\n", ggv->mds_lnorm);
}
void ggv_weight_power_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->mds_weight_power = adj->value;
 g_printerr ("mds_weight_power = %f\n", ggv->mds_weight_power);
}

void ggv_metric_cb (GtkWidget *w, gpointer cbd)
{
  PluginInstance *inst = (PluginInstance *)
    gtk_object_get_data (GTK_OBJECT(w), "PluginInst");
  ggvisd *ggv = ggvisFromInst (inst);

  ggv->metric_nonmetric = (MDSMetricInd) GPOINTER_TO_INT (cbd);
 g_printerr ("metric = %d\n", ggv->metric_nonmetric);

}
void ggv_kruskal_cb (GtkWidget *w, gpointer cbd)
{
  PluginInstance *inst = (PluginInstance *)
    gtk_object_get_data (GTK_OBJECT(w), "PluginInst");
  ggvisd *ggv = ggvisFromInst (inst);

  ggv->KruskalShepard_classic = (MDSKSInd) GPOINTER_TO_INT (cbd);
 g_printerr ("KruskalShepardInd = %d\n", ggv->KruskalShepard_classic);
}


void ggv_groups_cb (GtkWidget *w, PluginInstance *inst)
{
}


void ggv_constrained_cb (GtkWidget *w, PluginInstance *inst)
{
}
