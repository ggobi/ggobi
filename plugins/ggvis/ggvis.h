#ifndef GGVIS_H

#include "defines.h"
#include "plugin.h"

#define STRESSPLOT_WIDTH  250
#define STRESSPLOT_HEIGHT 100
#define STRESSPLOT_MARGIN  10

#define HISTOGRAM_WIDTH  250
#define HISTOGRAM_HEIGHT 100
#define HISTOGRAM_MARGIN  10

typedef enum {deflt, within, between, anchorscales, anchorfixed} MDSGroupInd;
typedef enum {metric, nonmetric} MDSMetricInd;
typedef enum {KruskalShepard, classic} MDSKSInd;

typedef struct {

  array_d dist_orig;
  array_d dist;
  array_d pos_orig;
  array_d pos;

  GdkPixmap *stressplot_pix;
  GdkPixmap *histogram_pix;

/* callbacks written */
  gint mds_dims;
  gdouble mds_stepsize;
  gdouble mds_power;
  gdouble mds_dist_power;
  gdouble mds_lnorm;
  gdouble mds_weight_power;

/* callbacks not written */
  gdouble mds_isotonic_mix;
  gdouble mds_distpow_over_lnorm;
  gdouble mds_lnorm_over_distpow;
  gdouble mds_within_between;
  gdouble mds_rand_select_val;
  gdouble mds_rand_select_new;
  gdouble mds_perturb_val;
  gdouble mds_threshold_high;
  gdouble mds_threshold_low;

/* callbacks written */
  MDSMetricInd metric_nonmetric;
  MDSKSInd KruskalShepard_classic;

} ggvisd;


/*----------------------------------------------------------------------*/
/*                          functions                                   */
/*----------------------------------------------------------------------*/

void ggv_dsource_cb (GtkWidget *w, PluginInstance *inst);
void ggvis_init (ggvisd *);
ggvisd* ggvisFromInst (PluginInstance *inst);
gint ggv_stressplot_configure_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);
gint ggv_stressplot_expose_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);
gint ggv_histogram_configure_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);
gint ggv_histogram_expose_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);
void ggv_metric_cb (GtkWidget *w, gpointer cbd);
void ggv_kruskal_cb (GtkWidget *w, gpointer cbd);
void ggv_groups_cb (GtkWidget *w, PluginInstance *inst);
void ggv_constrained_cb (GtkWidget *w, PluginInstance *inst);
void ggv_dims_cb (GtkAdjustment *adj, PluginInstance *inst);
void ggv_stepsize_cb (GtkAdjustment *adj, PluginInstance *inst);
void ggv_power_cb (GtkAdjustment *adj, PluginInstance *inst);
void ggv_dist_power_cb (GtkAdjustment *adj, PluginInstance *inst);
void ggv_lnorm_cb (GtkAdjustment *adj, PluginInstance *inst);
void ggv_weight_power_cb (GtkAdjustment *adj, PluginInstance *inst);

#define GGVIS_H
#endif
