#ifndef GGVIS_H

#include "defines.h"
#include "plugin.h"

#define UNIFORM 0
#define NORMAL  1

#define STRESSPLOT_WIDTH  250
#define STRESSPLOT_HEIGHT 100
#define STRESSPLOT_MARGIN  10
#define NSTRESSVALUES    1000
#define STRESSPLOT_MARGIN  10

#define HISTOGRAM_WIDTH      250
#define HISTOGRAM_HEIGHT     100
#define HISTOGRAM_VMARGIN      5
#define HISTOGRAM_HMARGIN     24  /* should be an even number */
#define HISTOGRAM_GRIP_SPACE  10
#define HISTOGRAM_GRIP_HEIGHT 10  /* should be even number */
#define HISTOGRAM_GRIP_WIDTH  20  /* should be even number */
#define HISTOGRAM_BWIDTH       5

typedef int (*CompareFunc)(const void *, const void *);

typedef enum {deflt, within, between, anchorscales, anchorfixed} MDSGroupInd;
typedef enum {metric, nonmetric} MDSMetricInd;
typedef enum {KruskalShepard, classic} MDSKSInd;
typedef enum {LinkDist, VarValues} MDSDtargetSource;

#define EXCLUDED 0
#define INCLUDED 1
#define ANCHOR   2
#define DRAGGED  4 

typedef struct {
  GtkWidget *da;
  GdkPixmap *pix;
  gdouble low, high;
  gint lgrip_pos, rgrip_pos;
  gint lgrip_down, rgrip_down;
  GdkRectangle *bars;
  vector_b bars_included;
  vector_i bins;
  gint nbins;
} dissimd;

typedef struct {

  datad *dsrc;  /*-- original data values --*/
  datad *dpos;  /*-- the new datad which contains the values in pos --*/
  datad *e;     /*-- edge set, corresponds both to dsrc and dpos --*/

  gboolean mds_running;
  guint idle_id;
  
  array_d Dtarget;  /*-- D in the documentation; dist in the xgvis code --*/
  array_d pos;

  GtkWidget *stressplot_da;
  GdkPixmap *stressplot_pix;
  vector_d stressvalues;  /*-- allocated to hold NSTRESSVALUES values --*/
  gint nstressvalues;     /*-- the number of stress values */

  dissimd *dissim;

  gint mds_dims;
  gdouble mds_stepsize;

  gdouble mds_Dtarget_power;  /* was mds_power */
  gdouble mds_weight_power;   /* was mds_weightpow */
  gdouble mds_dist_power;     /* was mds_distpow */

  gdouble mds_lnorm;
  gdouble mds_dist_power_over_lnorm;
  gdouble mds_lnorm_over_dist_power;

  gdouble mds_isotonic_mix;
  gdouble mds_within_between;
  gdouble mds_rand_select_val;
  gdouble mds_rand_select_new;
  gdouble mds_perturb_val;
  gdouble mds_threshold_high;
  gdouble mds_threshold_low;

  vector_d pos_mean;
  vector_d weights;
  vector_d trans_dist;
  vector_d config_dist;
  vector_i point_status;
  vector_i trans_dist_index, bl;
  array_d gradient;
  vector_d bl_w;
  gdouble pos_scl;
  gdouble Dtarget_max, Dtarget_min;
  vector_d rand_sel;
  gint mds_freeze_var;
  gint ndistances;
  gint num_active_dist;
  gint prev_nonmetric_active_dist;

/* callbacks written */
  MDSMetricInd metric_nonmetric;
  MDSKSInd KruskalShepard_classic;
  MDSDtargetSource Dtarget_source;
  gboolean complete_Dtarget;

  MDSGroupInd mds_group_ind;

} ggvisd;


/*----------------------------------------------------------------------*/
/*                          functions                                   */
/*----------------------------------------------------------------------*/

void ggvis_init (ggvisd *);
ggvisd* ggvisFromInst (PluginInstance *inst);
void mds_run_cb (GtkToggleButton *btn, PluginInstance *inst);
void mds_step_cb (GtkWidget *btn, PluginInstance *inst);
void mds_reinit_cb (GtkWidget *btn, PluginInstance *inst);
void mds_scramble_cb (GtkWidget *btn, PluginInstance *inst);
void update_ggobi (ggvisd *ggv, ggobid *gg);

void ggv_dsource_cb (GtkWidget *w, gpointer cbd);
void ggv_complete_distances_cb (GtkToggleButton *button, PluginInstance *inst);
void ggv_compute_Dtarget_cb (GtkWidget *button, PluginInstance *inst);

gint ggv_stressplot_configure_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);
gint ggv_stressplot_expose_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);

gint ggv_histogram_configure_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);
gint ggv_histogram_expose_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);
void ggv_histogram_init (ggvisd *ggv, ggobid *gg);
gint ggv_histogram_motion_cb (GtkWidget *w, GdkEventMotion *xmotion, PluginInstance *inst);
void ggv_histogram_button_press_cb (GtkWidget *w, GdkEventButton *evnt, PluginInstance *inst);
void ggv_histogram_button_release_cb (GtkWidget *w, GdkEventButton *evnt, PluginInstance *inst);
void ggv_Dtarget_histogram_update (ggvisd *, ggobid *);

void ggv_metric_cb (GtkWidget *w, gpointer cbd);
void ggv_kruskal_cb (GtkWidget *w, gpointer cbd);
void ggv_groups_cb (GtkWidget *w, PluginInstance *inst);
void ggv_constrained_cb (GtkWidget *w, PluginInstance *inst);
void ggv_dims_cb (GtkAdjustment *adj, PluginInstance *inst);
void ggv_perturb_adj_cb (GtkAdjustment *adj, PluginInstance *inst);
void ggv_perturb_btn_cb (GtkWidget *w, PluginInstance *inst);
void ggv_selection_prob_adj_cb (GtkAdjustment *adj, PluginInstance *inst);
void ggv_selection_prob_btn_cb (GtkWidget *w, PluginInstance *inst);
void ggv_stepsize_cb (GtkAdjustment *adj, PluginInstance *inst);
void ggv_dist_power_cb (GtkAdjustment *adj, PluginInstance *inst);
void ggv_Dtarget_power_cb (GtkAdjustment *adj, PluginInstance *inst);
void ggv_lnorm_cb (GtkAdjustment *adj, PluginInstance *inst);
void ggv_weight_power_cb (GtkAdjustment *adj, PluginInstance *inst);
void ggv_center_scale_pos (ggvisd *ggv);
greal ggv_randvalue (gint type);
void update_stress (ggvisd *ggv, ggobid *gg);
void ggv_compute_Dtarget (gint selected_var, ggvisd *ggv);
void ggv_init_Dtarget (gint, ggvisd *ggv);

void mds_func (gboolean, PluginInstance *);
void mds_once (gboolean doit, ggvisd *ggv, ggobid *gg);

#define GGVIS_H
#endif
