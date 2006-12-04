#ifndef GLAYOUT_H

#include "plugin.h"
#include "config.h"

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
  gint nnodes;
  noded *nodes;
} radiald;

typedef enum {
  neato_shortest_path, neato_circuit_resistance, neato_subset
} NeatoModel;

typedef struct {

  GGobiData *dsrc, *e;
  GGobiData *d;  /* the last datad created */
  GtkWidget *window;

  array_d dist;
  array_d pos;

  gint centerNodeIndex;
  gboolean radialAutoUpdate;
  gboolean radialNewData;
  radiald *radial;  /*-- data required for radial layout --*/

  gint neato_dim;
  NeatoModel neato_model;
  gboolean neato_use_edge_length_p;

} glayoutd;


/*--------------------------------------------------------------------*/
/*                        functions                                   */
/*--------------------------------------------------------------------*/

void radial_cb (GtkButton *button, PluginInstance *inst);
void radial_new_data_cb (GtkToggleButton *, PluginInstance *);
void radial_auto_update_cb (GtkToggleButton *, PluginInstance *);
void glayout_init (glayoutd *);
glayoutd* glayoutFromInst (PluginInstance *inst);
void highlight_sticky_edges (ggobid *, gint, gint , GGobiData *d, void *inst);
gint visible_set (glong *visible, GGobiData *d);

void radial_center_set_cb (ggobid *gg, gint index, gint state, GGobiData *d, PluginInstance *inst);

#if defined HAVE_LIBGVC
void neato_dim_cb (GtkAdjustment *adj, PluginInstance *inst);
void neato_model_cb (GtkWidget *w, PluginInstance *inst);
void dot_neato_layout_cb (GtkWidget *, PluginInstance *inst);
void neato_use_edge_length_cb (GtkToggleButton *button, PluginInstance *inst);
#endif

#define GLAYOUT_H
#endif
