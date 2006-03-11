#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "graphact.h"

/*
My earlier strategy of using 'visibleEdges' fails because it
doesn't take into account the case where there's a single in
edge and a single out edge, but the endpoint is the same.  In
other words, it looks like two edges in the code but one edge
on the screen.
*/

#if 0
void
count_visible_edges (PluginInstance *inst)
{
  graphactd *ga = (graphactd *) inst->data;
  GGobiData *d = ga->d;
  GGobiData *e = ga->e;
  gint m, i, k, a, b, edgeid;
  endpointsd *endpoints;

  if (e == NULL) {
    quick_message ("You haven't designated a set of edges.", false);
/**/return;
  }
  endpoints = resolveEdgePoints(e, d);
  if (endpoints == NULL) {
    g_printerr ("failed to resolve edges. d: %s, e: %s\n", d->name, e->name);
/**/return;
  }

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
    i = d->rows_in_plot.els[m];
    for (k=0; k<ga->inEdges[i].nels; k++) {
      edgeid = ga->inEdges[i].els[k];
      a = endpoints[edgeid].a;
      /*-- no need for b, because i = b --*/
      if (e->sampled.els[edgeid] && !e->hidden_now.els[edgeid] &&
          !d->hidden_now.els[a] && !d->hidden_now.els[i])
      {
        ga->nInEdgesVisible.els[i]++;
      }
    }
    for (k=0; k<ga->outEdges[i].nels; k++) {
      edgeid = ga->outEdges[i].els[k];
      b = endpoints[edgeid].b;
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
#endif

void
hide_inEdge (gint i, PluginInstance *inst)
{
  gint k, edgeid, a;
  graphactd *ga = graphactFromInst (inst);
  GGobiData *d = ga->d;
  GGobiData *e = ga->e;
  ggobid *gg = inst->gg;
  gint nd = g_slist_length (gg->d);
  endpointsd *endpoints;

  if (e == NULL) {
    quick_message ("You haven't designated a set of edges.", false);
/**/return;
  }
  endpoints = resolveEdgePoints(e, d);
  if (endpoints == NULL) {
    g_printerr ("failed to resolve edges. d: %s, e: %s\n", d->name, e->name);
/**/return;
  }

  for (k=0; k<ga->inEdges[i].nels; k++) {
    edgeid = ga->inEdges[i].els[k];
    a = endpoints[edgeid].a;
    /*-- no need for b, because i = b --*/

    e->hidden_now.els[edgeid] = e->hidden.els[edgeid] = true;
    d->hidden_now.els[i] = d->hidden.els[i] = true;

    if (!gg->linkby_cv && nd > 1) {
      symbol_link_by_id (true, i, d, gg);
      symbol_link_by_id (true, edgeid, e, gg);
    }
    /*
    ga->nInEdgesVisible.els[i] = 0;   
    ga->nOutEdgesVisible.els[a]--;
    */
  }
}
void
hide_outEdge (gint i, PluginInstance *inst)
{
  gint k, edgeid, b;
  graphactd *ga = graphactFromInst (inst);
  GGobiData *d = ga->d;
  GGobiData *e = ga->e;
  ggobid *gg = inst->gg;
  gint nd = g_slist_length (gg->d);
  endpointsd *endpoints;

  if (e == NULL) {
    quick_message ("You haven't designated a set of edges.", false);
/**/return;
  }
  endpoints = resolveEdgePoints(e, d);
  if (endpoints == NULL) {
    g_printerr ("failed to resolve edges. d: %s, e: %s\n", d->name, e->name);
/**/return;
  }

  for (k=0; k<ga->outEdges[i].nels; k++) {
    edgeid = ga->outEdges[i].els[k];
    b = endpoints[edgeid].b;

    e->hidden_now.els[edgeid] = e->hidden.els[edgeid] = true;
    d->hidden_now.els[i] = d->hidden.els[i] = true;

    if (!gg->linkby_cv && nd > 1) 
      symbol_link_by_id (true, i, d, gg);
    /*
    ga->nOutEdgesVisible.els[i] = 0;   
    ga->nInEdgesVisible.els[b]--;
    */
  }
}

void
ga_leaf_hide_cb (GtkWidget *btn, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  graphactd *ga = graphactFromInst (inst);
  GGobiData *d = ga->d;
  GGobiData *e = ga->e;
  gboolean changing;
  gboolean need_to_link_p = false;
  gint i, m;
  endpointsd *endpoints;

  if (e == NULL) {
    quick_message ("You haven't designated a set of edges.", false);
/**/return;
  }
  endpoints = resolveEdgePoints(e, d);
  if (endpoints == NULL) {
    g_printerr ("failed to resolve edges. d: %s, e: %s\n", d->name, e->name);
/**/return;
  }

/*
  count_visible_edges (inst);

  while (changing) {
    changing = false;
    nvisible = 0;
    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot.els[m];
      if (ga->nInEdgesVisible.els[i] + ga->nOutEdgesVisible.els[i] == 1) {
        *-- leaf node; hide the node and the edge;  decrease a counter --*
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
*/

  changing = true;
  while (changing) {
    gint ida, a, idb, b;
    changing = false;
    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot.els[m];
      if (!d->hidden_now.els[i]) {
        if (ga->inEdges[i].nels == 0 && ga->outEdges[i].nels == 0) {
          ;
        } else if (ga->inEdges[i].nels <= 1 && ga->outEdges[i].nels <= 1) {
          /* is this the same edge in the other direction? */
          if (ga->inEdges[i].nels == 1 && ga->outEdges[i].nels == 1) {
            ida = ga->inEdges[i].els[0];
            a = endpoints[ida].a;
            idb = ga->outEdges[i].els[0];
            b = endpoints[idb].b;
            if (a == b) {
              if (e->sampled.els[ida] && !e->hidden_now.els[ida] &&
                 !d->hidden_now.els[a])
              {
                hide_inEdge (i, inst);
                changing = true;
              }
              if (e->sampled.els[idb] && !e->hidden_now.els[idb] &&
                 !d->hidden_now.els[b])
              {
                hide_outEdge (i, inst);
                changing = true;
              }
            }
          /* or a singleton in one direction or the other? */
  	  } else if (ga->inEdges[i].nels == 1) {
	    hide_inEdge (i, inst);
            changing = true;
          } else if (ga->outEdges[i].nels == 1) {
	    hide_outEdge (i, inst);
            changing = true;
          }
        }
	if (changing) need_to_link_p = true;  
      }
    }
  }

  if (need_to_link_p) {
  }

  displays_tailpipe (FULL, gg);
}

void
ga_orphans_hide_cb (GtkWidget *btn, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  graphactd *ga = graphactFromInst (inst);
  GGobiData *d = gg->current_display->d;
  GGobiData *e = gg->current_display->e;
  gint m, i, k, edgeid, a, b;
  gboolean included;
  endpointsd *endpoints;
  gint nd = g_slist_length (gg->d);

  if (e == NULL) {
    quick_message ("You haven't designated a set of edges.", false);
/**/return;
  }
  endpoints = resolveEdgePoints(e, d);
  if (endpoints == NULL) {
    g_printerr ("failed to resolve edges. d: %s, e: %s\n", d->name, e->name);
/**/return;
  }

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot.els[m];
    included = false;
    for (k=0; k<ga->inEdges[i].nels; k++) {
      edgeid = ga->inEdges[i].els[k];
      a = endpoints[edgeid].a;
      /*-- no need for b, because i = b --*/
      if (e->sampled.els[edgeid] && !e->excluded.els[edgeid] &&
          !e->hidden.els[edgeid] &&
          !d->excluded.els[a] && !d->hidden.els[a] && !d->excluded.els[i])
      {
        included = true;
        break;
      }
    }
    if (!included) {
      for (k=0; k<ga->outEdges[i].nels; k++) {
        edgeid = ga->outEdges[i].els[k];
        b = endpoints[edgeid].b;
        /*-- no need for a, because i = a --*/
        if (e->sampled.els[edgeid] && !e->excluded.els[edgeid] &&
            !e->hidden.els[edgeid] &&
            !d->excluded.els[b] && !d->hidden.els[b] && !d->excluded.els[i])
        {
          included = true;
          break;
        }
      }
    }
    if (!included) {
      d->hidden_now.els[i] = d->hidden.els[i] = true;
      if (!gg->linkby_cv && nd > 1)
        symbol_link_by_id (true, i, d, gg);
    }
  }
  displays_tailpipe (FULL, gg);
}

void
ga_nodes_show_cb (GtkWidget *btn, PluginInstance *inst)
{
  graphactd *ga = graphactFromInst (inst);
  GGobiData *d = ga->d;
  GGobiData *e = ga->e;
  gint i;
  ggobid *gg = inst->gg;
  gint nd = g_slist_length (gg->d);

  for (i=0; i<d->nrows; i++) {
    d->hidden_now.els[i] = d->hidden.els[i] = d->hidden_prev.els[i] = false;
    if (!gg->linkby_cv && nd > 1)
      symbol_link_by_id (true, i, d, gg);
  }
  for (i=0; i<e->nrows; i++) {
    e->hidden_now.els[i] = e->hidden.els[i] = e->hidden_prev.els[i] = false;
    if (!gg->linkby_cv && nd > 1)
      symbol_link_by_id (true, i, e, gg);
  }

  displays_tailpipe (FULL, gg);
}

/*---------------------------------------------------------------------*/
/*                    Neighbors routines                               */
/*---------------------------------------------------------------------*/

void neighborhood_depth_cb (GtkWidget *w, gpointer cbd)
{
  PluginInstance *inst = (PluginInstance *) g_object_get_data(G_OBJECT (w),
    "PluginInst");
  graphactd *ga = (graphactd *) inst->data;
  ga->neighborhood_depth = GPOINTER_TO_INT (cbd) + 1;
}

void ga_all_hide (GGobiData *d, GGobiData *e, PluginInstance *inst)
{
  gint i;

  for (i=0; i<d->nrows; i++)
    d->hidden_now.els[i] = d->hidden.els[i] = true;
  for (i=0; i<e->nrows; i++)
    e->hidden_now.els[i] = e->hidden.els[i] = true;
}

void
show_neighbors (gint nodeid, gint edgeid, gint depth,
  GGobiData *d, GGobiData *e, PluginInstance *inst)
{
  gint a, b, neighbor, k, eid;
  graphactd *ga = (graphactd *) inst->data;
  endpointsd *endpoints;

  if (e == NULL) {
    quick_message ("You haven't designated a set of edges.", false);
/**/return;
  }
  endpoints = resolveEdgePoints(e, d);
  if (endpoints == NULL) {
    g_printerr ("failed to resolve edges. d: %s, e: %s\n", d->name, e->name);
/**/return;
  }

  edge_endpoints_get (edgeid, &a, &b, d, endpoints, e);

  e->hidden_now.els[edgeid] = e->hidden.els[edgeid] = false;
  d->hidden_now.els[a] = d->hidden.els[a] = false;
  d->hidden_now.els[b] = d->hidden.els[b] = false;


  if (depth-1) {
    neighbor = (nodeid == a) ? b : a;
    for (k=0; k<ga->inEdges[neighbor].nels; k++) {
      eid = ga->inEdges[neighbor].els[k];
      if (eid != edgeid)
        show_neighbors (neighbor, eid, depth-1, d, e, inst);
    }
    for (k=0; k<ga->outEdges[neighbor].nels; k++) {
      eid = ga->outEdges[neighbor].els[k];
      if (eid != edgeid)
        show_neighbors (neighbor, eid, depth-1, d, e, inst);
    }
  }
}

/*
 * Find the neighbors of node 'index' and show them; hide
 * all others.
*/
void show_neighbors_sticky_cb (ggobid *gg, gint index, gint state,
  GGobiData *d, void *data)
{
  PluginInstance *inst = (PluginInstance *)data;
  graphactd *ga = (graphactd *) inst->data;
  GGobiData *e = ga->e;
  gint k, edgeid;
  gint nd = g_slist_length (gg->d);
  gint i;
  endpointsd *endpoints;
  /*GGobiData *d = ga->d;*/
  enum {GRAPH_VIEW, EDGE_DATA_VIEW} idview = GRAPH_VIEW;
  displayd *display = gg->current_display;

  if (e == NULL) {
    quick_message ("You haven't designated a set of edges.", false);
/**/return;
  }

  /* 
     If I'm in the graph (map) view, display->d = ga->d
     and display->e = ga->e.

     If I'm in the scatterplot of the edge data (variogram cloud),
     then display->d = ga->e.  Don't do anything with this one for
     now.
 */
  if (display->d == ga->d) {
    idview = GRAPH_VIEW;
  } else if (display->d == ga->e) {
    idview = EDGE_DATA_VIEW;
  }
  if (idview == EDGE_DATA_VIEW)
/**/return;

  endpoints = resolveEdgePoints(e, d);
  if (endpoints == NULL) {
    g_printerr ("failed to resolve edges. d: %s, e: %s\n", d->name, e->name);
/**/return;
  }

  if (index == -1)
    return;


/*
 * This is now being executed in the same way whether 'index' is
 * becoming sticky or unsticky.
*/

  ga_all_hide (d, e, inst);

  for (k=0; k<ga->inEdges[index].nels; k++) {
    edgeid = ga->inEdges[index].els[k];
    show_neighbors (index, edgeid, ga->neighborhood_depth, d, e, inst);
  }
  for (k=0; k<ga->outEdges[index].nels; k++) {
    edgeid = ga->outEdges[index].els[k];
    show_neighbors (index, edgeid, ga->neighborhood_depth, d, e, inst);
  }

  /*-- <now> do the linking --*/
  if (!gg->linkby_cv && nd > 1) {
    for (i=0; i<d->nrows; i++)
      symbol_link_by_id (true, i, d, gg);
  }
  if (!gg->linkby_cv && nd > 2) {
   for (i=0; i<e->nrows; i++)
      symbol_link_by_id (true, i, e, gg);
  }

  displays_tailpipe (FULL, gg);
}

void
show_neighbors_toggle_cb (GtkToggleButton *button, PluginInstance *inst)
{
  graphactd *ga = (graphactd *) inst->data;

  if (ga->neighbors_find_p) {
    g_signal_handlers_disconnect_by_func(G_OBJECT(inst->gg),
      G_CALLBACK (show_neighbors_sticky_cb), inst);
    ga->neighbors_find_p = false;
  } else {
    g_signal_connect (G_OBJECT(inst->gg),
      "sticky_point_added", G_CALLBACK(show_neighbors_sticky_cb), inst);
    g_signal_connect (G_OBJECT(inst->gg),
      "sticky_point_removed", G_CALLBACK(show_neighbors_sticky_cb), inst);
    ga->neighbors_find_p = true;
  }
}

/*--------- Tidy -------------*/
void
ga_edge_tidy_cb (GtkWidget *w, PluginInstance *inst)
{
  graphactd *ga = (graphactd *) inst->data;
  GGobiData *d = ga->d;
  GGobiData *e = ga->e;
  endpointsd *endpoints;
  gint a, b, k;

  /* Loop over edges.  If either endpoint is hidden, hide the edge */

  if (e == NULL) {
    quick_message ("You haven't designated a set of edges.", false);
/**/return;
  }
  endpoints = resolveEdgePoints(e, d);
  if (endpoints == NULL) {
    g_printerr ("failed to resolve edges. d: %s, e: %s\n", d->name, e->name);
/**/return;
  }

  for (k=0; k<e->edge.n; k++) {
    edge_endpoints_get (k, &a, &b, d, endpoints, e);
    if (d->hidden_now.els[a] || d->hidden_now.els[b]) {
      e->hidden_now.els[k] = true;
    }
  }
  displays_tailpipe (FULL, inst->gg);
}
