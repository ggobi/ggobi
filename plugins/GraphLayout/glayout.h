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

  array_d dist_orig;
  array_d dist;
  array_d pos_orig;
  array_d pos;

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
