#ifndef GLAYOUT_H

#include "defines.h"
#include "plugin.h"

typedef enum {deflt, within, between, anchorscales, anchorfixed} MDSGroupInd;

typedef struct _noded {
  gint i;

  gint inDegree, outDegree;
  GList *connectedEdges; /*-- each element a GINT_TO_POINTER --*/
  GList *connectedNodes; /*-- each element a noded --*/

  /*-- indexes of reference nodes --*/
  struct _noded *parentNode;

  gint nStepsToLeaf;
  gint nStepsToCenter;
  gint subtreeSize;
  gint nChildren;

  gdouble span, theta;
  fcoords pos;
} noded;

typedef struct {
  noded *centerNode;
  gint nStepsToLeaf;
  gint nStepsToCenter;
  noded *nodes;
  datad *d;
} radiald;

#ifdef GRAPHVIZ
#include <render.h>
typedef struct {
  Agraph_t *graph;
  datad *d;
} graphvizd;
#endif

typedef struct {

/*
  gboolean running_p;
  gboolean rescale_p;

  gint dist_type;
  MDSGroupInd mds_group_ind;

  gdouble mds_stepsize;
  gdouble mds_power;
  gdouble mds_distpow;
  gdouble mds_lnorm;
  gdouble mds_distpow_over_lnorm;
  gdouble mds_lnorm_over_distpow;
  gdouble mds_weightpow;
  gdouble mds_within_between;
  gdouble mds_rand_select_val;
  gdouble mds_rand_select_new;
  gdouble mds_perturb_val;
  gdouble mds_threshold_high;
  gdouble mds_threshold_low;
  gint    mds_dims;
  gint    mds_freeze_var;
*/

/*
 * Used in scaling during each mds loop; set in reset_data
*/
  gdouble *config_dist;
    /* spave vs time: store configuration distances to save recalculation */
  gdouble *raw_dist;
    /* pointer, a vector version of dist.data */
  gdouble *weights;
    /* formed only when mds_weightpow != 0. */
  gdouble *trans_dist;
    /* transformed dissimilarities: power (metric), isotonic (nonmetric) */
  gint *trans_dist_index; /* index array for sort of raw_dist */
  gint *bl;               /* blocklengths for isotonic regression */
  gdouble *bl_w;         
    /* blockweights for isotonic regression (only when mds_weightpow != 0.) */
  gdouble *rand_sel;
    /* random selection probabilities (only when mds_rand_select != 1.) */

  gint ndistances;
  gint num_active_dist;

  gdouble configuration_factor;

  array_d dist_orig;
  array_d dist;
  array_d pos_orig;
  array_d pos;

  gint metric_nonmetric;
  gint KruskalShepard_classic;

  radiald *radial;  /*-- data required for radial layout --*/

#ifdef GRAPHVIZ
  graphvizd *graphviz;  /*-- data required for dot and neato layouts --*/
#endif

} glayoutd;


/*----------------------------------------------------------------------*/
/*                          functions                                   */
/*----------------------------------------------------------------------*/

void glayout_init (glayoutd *);
void setParentNodes (glayoutd *gl, datad *d);
void setNChildren (glayoutd *gl, datad *d);
gint setSubtreeSize (noded *, glayoutd *, datad *);
void setSubtreeSpans (glayoutd *, datad *);
void setNodePositions (glayoutd *, datad *);
void radial_cb (GtkButton *button, PluginInstance *inst);
void highlight_edges_cb (GtkButton *button, PluginInstance *inst);
glayoutd* glayoutFromInst (PluginInstance *inst);
void highlight_sticky_edges (ggobid *, gint, gint , datad *d, void *inst);

#ifdef CMDS
gint cmds (array_d *D, array_d *X);
void spring_once (gint ndims, datad *d, datad *e, array_d *dist, array_d *pos);
#endif

#ifdef GRAPHVIZ
void dot_neato_layout_cb (GtkWidget *, PluginInstance *inst);
#endif

#define GLAYOUT_H
#endif
