#ifndef GLAYOUT_H

#include "plugin.h"

typedef struct {

  datad *d, *e;

  gint nnodes;      /*-- nrows, not nrows_in_plot --*/
  gint nedges;
  vector_i *inEdges;  /*-- one vector per node, each element an integer --*/
  vector_i *outEdges; /*-- one vector per node, each element an integer --*/

  vector_i nInEdgesVisible;
  vector_i nOutEdgesVisible;

  GtkWidget *window;

  /*-- finding neighborhoods --*/
  gboolean neighbors_find_p;
  gint nbor_radius;

} graphactd;


void graphact_init (graphactd *ga);
void init_edge_vectors (gboolean reinit, PluginInstance *inst);
graphactd * graphactFromInst (PluginInstance *inst);
void ga_leaf_hide_cb (GtkWidget *btn, PluginInstance *inst);
void ga_nodes_show_cb (GtkWidget *btn, PluginInstance *inst);
void count_visible_edges (PluginInstance *inst);
void show_neighbors_cb (GtkToggleButton *button, PluginInstance *inst);

#define GLAYOUT_H
#endif

