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
  gint nnodes;
  noded *nodes;
} radiald;

#ifdef GRAPHVIZ
#include <render.h>
#endif

typedef struct {

  datad *dsrc, *e;
  GtkWidget *window;

  array_d dist;
  array_d pos;

  gint centerNodeIndex;
  radiald *radial;  /*-- data required for radial layout --*/

  gint neato_dim;

} glayoutd;


/*----------------------------------------------------------------------*/
/*                          functions                                   */
/*----------------------------------------------------------------------*/

void radial_cb (GtkButton *button, PluginInstance *inst);
void glayout_init (glayoutd *);
glayoutd* glayoutFromInst (PluginInstance *inst);
void highlight_sticky_edges (ggobid *, gint, gint , datad *d, void *inst);
gint visible_set (glong *visible, datad *d);

void radial_center_set_cb (ggobid *gg, gint index, gint state, datad *d, PluginInstance *inst);

#ifdef CMDS
gint cmds (array_d *D, array_d *X);
void spring_once (gint ndims, datad *d, datad *e, array_d *dist, array_d *pos);
#endif

#ifdef GRAPHVIZ
void dot_neato_layout_cb (GtkWidget *, PluginInstance *inst);
#endif

#define GLAYOUT_H
#endif
