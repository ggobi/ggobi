#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "defines.h"
#include "ggvis.h"

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
void ggv_dist_power_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->mds_dist_power = adj->value;
 g_printerr ("mds_dist_power = %f\n", ggv->mds_dist_power);
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
