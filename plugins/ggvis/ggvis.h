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

/* used in mds.c, shepard.c, ggv_cbacks.c */
#define IJ i*ggv->Dtarget.ncols+j 
#define JI j*ggv->Dtarget.nrows+i

typedef int (*CompareFunc)(const void *, const void *);

typedef enum {within, between, anchorscales, anchorfixed} MDSGroupInd;
typedef enum {metric, nonmetric} MDSMetricInd;
typedef enum {KruskalShepard, classic} MDSKSInd;
typedef enum {LinkDist, VarValues} MDSDtargetSource;
typedef enum {DissimAnalysis, GraphLayout} MDSTask;

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

  gboolean running_p;
  guint idle_id;
  
  array_d Dtarget;  /*-- D in the documentation; dist in the xgvis code --*/
  array_d pos;

  GtkWidget *stressplot_da;
  GdkPixmap *stressplot_pix;
  vector_d stressvalues;  /*-- allocated to hold NSTRESSVALUES values --*/
  gint nstressvalues;     /*-- the number of stress values */

  dissimd *dissim;

  gint dim;
  gint maxdim;  /*-- this increases only, never decreases --*/
  gdouble stepsize;

  gdouble Dtarget_power;  /* was mds_power */
  gdouble weight_power;   /* was mds_weightpow */
  gdouble dist_power;     /* was mds_distpow */

  gdouble lnorm;
  gdouble dist_power_over_lnorm;
  gdouble lnorm_over_dist_power;

  gdouble isotonic_mix;
  gdouble within_between;
  gdouble rand_select_val;
  gdouble rand_select_new;
  gdouble perturb_val;
  gdouble threshold_high;
  gdouble threshold_low;

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
  gint freeze_var;
  gint ndistances;
  gint num_active_dist;
  gint prev_nonmetric_active_dist;

/* callbacks written */
  MDSMetricInd metric_nonmetric;
  MDSKSInd KruskalShepard_classic;
  MDSTask mds_task;  /* DissimAnalysis or GraphLayout */
  MDSDtargetSource Dtarget_source;
  GtkCList *clist_dist;
  gboolean complete_Dtarget;

  gboolean group_p;
  MDSGroupInd group_ind;
  vector_b anchor_group;

  /*-- for Shepard plot --*/
  gint shepard_iter;

} ggvisd;


/*----------------------------------------------------------------------*/
/*                          functions                                   */
/*----------------------------------------------------------------------*/

void ggvis_init (ggvisd *, ggobid *gg);
ggvisd* ggvisFromInst (PluginInstance *inst);
void mds_run_cb (GtkToggleButton *btn, PluginInstance *inst);
void mds_step_cb (GtkWidget *btn, PluginInstance *inst);
void mds_reinit_cb (PluginInstance *inst, guint action, GtkWidget *w);
void mds_scramble_cb (PluginInstance *inst, guint action, GtkWidget *w);
void mds_reset_params_cb (PluginInstance *inst, guint action, GtkWidget *w);
void update_ggobi (ggvisd *ggv, ggobid *gg);

/*void ggv_dsource_cb (GtkWidget *w, gpointer cbd);*/
void ggv_complete_distances_cb (GtkToggleButton *button, PluginInstance *inst);
void ggv_edge_weights_cb (GtkToggleButton *button, PluginInstance *inst);
void ggv_brush_groupsp_cb (GtkToggleButton *button, PluginInstance *inst);
void ggv_brush_groups_opt_cb (GtkWidget *w, gpointer cbd);
void ggv_compute_Dtarget_cb (GtkWidget *button, PluginInstance *inst);

gint ggv_stressplot_configure_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);
gint ggv_stressplot_expose_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);

void ggv_task_cb (GtkToggleButton *button, PluginInstance *inst);

gint ggv_histogram_configure_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);
gint ggv_histogram_expose_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);
void ggv_histogram_init (ggvisd *ggv, ggobid *gg);
gint ggv_histogram_motion_cb (GtkWidget *w, GdkEventMotion *xmotion, PluginInstance *inst);
void ggv_histogram_button_press_cb (GtkWidget *w, GdkEventButton *evnt, PluginInstance *inst);
void ggv_histogram_button_release_cb (GtkWidget *w, GdkEventButton *evnt, PluginInstance *inst);
void ggv_Dtarget_histogram_update (ggvisd *, ggobid *);

void ggv_metric_cb (GtkWidget *w, gpointer cbd);
void ggv_metric (GtkWidget *w, gint param);
void ggv_kruskal_cb (GtkWidget *w, gpointer cbd);
void ggv_groups_cb (GtkWidget *w, PluginInstance *inst);
void ggv_constrained_cb (GtkWidget *w, gpointer cbd);
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
void mds_open_display_cb (GtkWidget *btn, PluginInstance *inst);

void clusters_changed_cb (ggobid *, datad *, void *);
void create_shepard_data_cb (PluginInstance *inst, guint action, GtkWidget *w);

#define GGVIS_H
#endif
