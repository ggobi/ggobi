#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "graphact.h"

void
count_visible_edges (PluginInstance *inst)
{
  graphactd *ga = (graphactd *) inst->data;
  datad *d = ga->d;
  datad *e = ga->e;
  gint m, i, k, a, b, edgeid;
  endpointsd *endpoints;

  endpoints = e->edge.endpoints;

  if (ga->nInEdgesVisible.nels != ga->nnodes) {
    vectori_realloc (&ga->nInEdgesVisible, ga->nnodes);
    vectori_realloc (&ga->nOutEdgesVisible, ga->nnodes);
  }
  vectori_zero (&ga->nInEdgesVisible);
  vectori_zero (&ga->nOutEdgesVisible);

/*
 * I don't really need to distinguish between inEdges and outEdges
 * for anything I'm going to do for a while, but why not?
*/
  /*--   --*/
  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    for (k=0; k<ga->inEdges[i].nels; k++) {
      edgeid = ga->inEdges[i].els[k];
      a = d->rowid.idv.els[endpoints[edgeid].a];
      /*-- no need for b, because i = b --*/
      if (e->sampled.els[edgeid] && !e->hidden_now.els[edgeid] &&
          !d->hidden_now.els[a] && !d->hidden_now.els[i])
      {
        ga->nInEdgesVisible.els[i]++;
      }
    }
    for (k=0; k<ga->outEdges[i].nels; k++) {
      edgeid = ga->outEdges[i].els[k];
      b = d->rowid.idv.els[endpoints[edgeid].b];
      /*-- no need for a, because i = a --*/
      if (e->sampled.els[edgeid] && !e->hidden_now.els[edgeid] &&
          !d->hidden_now.els[b] && !d->hidden_now.els[i])
      {
        ga->nOutEdgesVisible.els[i]++;
      }
    }
  }
  
/*
for (i=0; i<ga->nnodes; i++)
  g_printerr ("%d %d\n",
    ga->nInEdgesVisible.els[i],
    ga->nOutEdgesVisible.els[i]);
*/
}

void
hide_inEdge (gint i, PluginInstance *inst)
{
  gint k, edgeid, a;
  graphactd *ga = graphactFromInst (inst);
  datad *d = ga->d;
  datad *e = ga->e;
  endpointsd *endpoints = e->edge.endpoints;
  ggobid *gg = inst->gg;
  gint nd = g_slist_length (gg->d);

  for (k=0; k<ga->inEdges[i].nels; k++) {
    edgeid = ga->inEdges[i].els[k];
    a = d->rowid.idv.els[endpoints[edgeid].a];
    /*-- no need for b, because i = b --*/

    e->hidden_now.els[edgeid] = e->hidden.els[edgeid] = true;
    d->hidden_now.els[i] = d->hidden.els[i] = true;

    if (!gg->linkby_cv && nd > 1)  {
      symbol_link_by_id (true, i, d, gg);
    }

    ga->nInEdgesVisible.els[i] = 0;   
    ga->nOutEdgesVisible.els[a]--;
  }
}
void
hide_outEdge (gint i, PluginInstance *inst)
{
  gint k, edgeid, b;
  graphactd *ga = graphactFromInst (inst);
  datad *d = ga->d;
  datad *e = ga->e;
  endpointsd *endpoints = e->edge.endpoints;
  ggobid *gg = inst->gg;
  gint nd = g_slist_length (gg->d);

  for (k=0; k<ga->outEdges[i].nels; k++) {
    edgeid = ga->outEdges[i].els[k];
    b = d->rowid.idv.els[endpoints[edgeid].b];

    e->hidden_now.els[edgeid] = e->hidden.els[edgeid] = true;
    d->hidden_now.els[i] = d->hidden.els[i] = true;

    if (!gg->linkby_cv && nd > 1) 
      symbol_link_by_id (true, i, d, gg);

    ga->nOutEdgesVisible.els[i] = 0;   
    ga->nInEdgesVisible.els[b]--;
  }
}

void
ga_leaf_hide_cb (GtkWidget *btn, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  graphactd *ga = graphactFromInst (inst);
  datad *d = ga->d;
  gboolean changing;
  gboolean need_to_link_p = false;
  gint nvisible;
  gint i, m;

  count_visible_edges (inst);

  while (changing) {
    changing = false;
    nvisible = 0;
    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot[m];
      if (ga->nInEdgesVisible.els[i] + ga->nOutEdgesVisible.els[i] == 1) {
        /*-- leaf node; hide the node and the edge;  decrease a counter --*/
        if (ga->nInEdgesVisible.els[i] == 1) {
          hide_inEdge (i, inst);
        } else {
          hide_outEdge (i, inst);
        }
        changing = true;
        need_to_link_p = true;
      } else {
        if (nvisible < 1)
          nvisible = MAX (nvisible,
            (ga->nInEdgesVisible.els[i] + ga->nOutEdgesVisible.els[i]));
      }
    }
    if (nvisible < 2) break;
  }

  if (need_to_link_p) {
  }

  displays_tailpipe (FULL, gg);
}

void
ga_nodes_show_cb (GtkWidget *btn, PluginInstance *inst)
{
  graphactd *ga = graphactFromInst (inst);
  datad *d = ga->d;
  datad *e = ga->e;
  gint i;
  ggobid *gg = inst->gg;
  gint nd = g_slist_length (gg->d);

  for (i=0; i<d->nrows; i++) {
    d->hidden_now.els[i] = d->hidden.els[i] = false;
    if (!gg->linkby_cv && nd > 1)
      symbol_link_by_id (false, i, d, gg);
  }
  for (i=0; i<e->nrows; i++)
    e->hidden_now.els[i] = e->hidden.els[i] = false;
    /*-- probably need to link here as well --*/

  displays_tailpipe (FULL, gg);
}

/*---------------------------------------------------------------------*/
/*                    Neighbors routines                               */
/*---------------------------------------------------------------------*/

void ga_all_hide (PluginInstance *inst)
{
  graphactd *ga = graphactFromInst (inst);
  datad *d = ga->d;
  datad *e = ga->e;
  gint i;

  /*-- no linking at this stage --*/
  for (i=0; i<d->nrows; i++)
    d->hidden_now.els[i] = d->hidden.els[i] = true;
  for (i=0; i<e->nrows; i++)
    e->hidden_now.els[i] = e->hidden.els[i] = true;
}

void show_neighbors (ggobid *gg, gint index, gint state,
  datad *d, void *data)
{
  PluginInstance *inst = (PluginInstance *)data;
  graphactd *ga = (graphactd *) inst->data;
  datad *e = ga->e;
  gint k, a, b, edgeid;
  endpointsd *endpoints;

  endpoints = e->edge.endpoints;

  if (state == STICKY) {
    /*
     * Find the neighbors of node 'index' and show them; hide
     * all others.
    */
    ga_all_hide (inst);

    for (k=0; k<ga->inEdges[index].nels; k++) {
      edgeid = ga->inEdges[index].els[k];
      a = d->rowid.idv.els[endpoints[edgeid].a];
      b = d->rowid.idv.els[endpoints[edgeid].b];

      e->hidden_now.els[edgeid] = e->hidden.els[edgeid] = false;
      d->hidden_now.els[a] = d->hidden.els[a] = false;
      d->hidden_now.els[b] = d->hidden.els[b] = false;
    }
    for (k=0; k<ga->outEdges[index].nels; k++) {
      edgeid = ga->outEdges[index].els[k];
      a = d->rowid.idv.els[endpoints[edgeid].a];
      b = d->rowid.idv.els[endpoints[edgeid].b];

      e->hidden_now.els[edgeid] = e->hidden.els[edgeid] = false;
      d->hidden_now.els[a] = d->hidden.els[a] = false;
      d->hidden_now.els[b] = d->hidden.els[b] = false;
    }
    displays_tailpipe (FULL, gg);
  }
}

void
show_neighbors_cb (GtkToggleButton *button, PluginInstance *inst)
{
  graphactd *ga = (graphactd *) inst->data;

  if (ga->neighbors_find_p) {
    gtk_signal_disconnect_by_func(GTK_OBJECT(inst->gg),
      GTK_SIGNAL_FUNC (show_neighbors), inst);
    ga->neighbors_find_p = false;
  } else {
    gtk_signal_connect (GTK_OBJECT(inst->gg),
      "sticky_point_added", show_neighbors, inst);
/*
    gtk_signal_connect (GTK_OBJECT(gg),
      "sticky_point_removed", show_all, inst);
*/
    ga->neighbors_find_p = true;
  }
}

