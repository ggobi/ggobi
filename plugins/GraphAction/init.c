#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include "plugin.h"

#include "graphact.h"

void
graphact_init (graphactd *ga) {

  ga->nnodes = -1;

  ga->d = NULL;
  ga->e = NULL;

  ga->inEdges = NULL;
  ga->outEdges = NULL;

  vectori_init_null (&ga->nOutEdgesVisible);
  vectori_init_null (&ga->nInEdgesVisible);

  /*-- finding neighborhoods --*/
  ga->neighbors_find_p = false;
  ga->neighborhood_depth = 1;
}

void
init_edge_vectors (gboolean reinit, PluginInstance *inst)
{
  /*ggobid *gg = inst->gg;*/
  graphactd *ga = graphactFromInst (inst);
  datad *d = ga->d;
  datad *e = ga->e;
  GList **inEdgeList, **outEdgeList, *l;
  gint a, b, i, k, n;
  endpointsd *endpoints;


  if (reinit && ga->nnodes > 0) {
    for (i=0; i<ga->nnodes; i++) {
      vectori_free (&ga->inEdges[i]);
      vectori_free (&ga->outEdges[i]);
    }
    g_free (ga->inEdges);
    g_free (ga->outEdges);
  }

  ga->nnodes = d->nrows;
  ga->nedges = e->nrows;

  ga->inEdges = (vector_i *) g_malloc (ga->nnodes * sizeof (vector_i));
  ga->outEdges = (vector_i *) g_malloc (ga->nnodes * sizeof (vector_i));
  for (i=0; i<ga->nnodes; i++) {
    vectori_init_null (&ga->inEdges[i]);
    vectori_init_null (&ga->outEdges[i]);
  }

  inEdgeList = (GList **) g_malloc (ga->nnodes * sizeof (GList *));
  outEdgeList = (GList **) g_malloc (ga->nnodes * sizeof (GList *));
  for (i=0; i<ga->nnodes; i++) {
    inEdgeList[i] = NULL;
    outEdgeList[i] = NULL;
  }

/*
for (i=0; i<ga->nnodes; i++)
g_printerr ("id %d %d idv %d %d\n",
d->rowid.id.els[i], d->rowid.id.els[i],
d->rowid.idv.els[i], d->rowid.idv.els[i]);
*/

  endpoints = e->edge.endpoints;
  for (i=0; i<ga->nedges; i++) {
/*
g_printerr ("endpoints %d %d idv %d %d\n",
endpoints[i].a, endpoints[i].b,
d->rowid.idv.els[endpoints[i].a], d->rowid.idv.els[endpoints[i].b]);
*/

    a = d->rowid.idv.els[endpoints[i].a];
    b = d->rowid.idv.els[endpoints[i].b];

    /*-- could use a,b to populate inNodeList, outNodeList --*/
    inEdgeList[b] = g_list_append (inEdgeList[b], GINT_TO_POINTER (i));
    outEdgeList[a] = g_list_append (outEdgeList[a], GINT_TO_POINTER (i));
/*
g_printerr ("length of inEdgeList[%d] = %d\n",
b, g_list_length(inEdgeList[b]));
g_printerr ("length of outEdgeList[%d] = %d\n",
a, g_list_length(outEdgeList[a]));
*/
  }

/*
for (i=0; i<ga->nnodes; i++) {
  g_printerr ("inEdgeList[%d] ", i);
  if (inEdgeList[i] != NULL) {
    g_printerr ("n=%d: ",  g_list_length(inEdgeList[i]));
    for (l=inEdgeList[i]; l; l=l->next)
      g_printerr ("%d ", GPOINTER_TO_INT (l->data));
  } else g_printerr ("n=0 ");

  g_printerr ("outEdgeList[%d] ", i);
  if (outEdgeList[i] != NULL) {
    g_printerr ("n=%d: ",  g_list_length(outEdgeList[i]));
    for (l=outEdgeList[i]; l; l=l->next)
      g_printerr ("%d ", GPOINTER_TO_INT (l->data));
  } else g_printerr ("n=0 ");

  g_printerr ("\n");
}
*/

  for (i=0; i<ga->nnodes; i++) {
    if ((n = g_list_length (inEdgeList[i])) > 0) {
      vectori_alloc (&ga->inEdges[i], n);
      k = 0;
      for (l=inEdgeList[i]; l; l=l->next) {
        ga->inEdges[i].els[k++] = GPOINTER_TO_INT (l->data);
      }
    }

    if ((n = g_list_length (outEdgeList[i])) > 0) {
      vectori_alloc (&ga->outEdges[i], n);
        k = 0;
      for (l=outEdgeList[i]; l; l=l->next) {
        ga->outEdges[i].els[k++] = GPOINTER_TO_INT (l->data);
      }
    }
  }

/*
for (i=0; i<ga->nnodes; i++) {
  g_printerr ("node %d (inEdges n=%d) ", d->rowid.id.els[i], ga->inEdges[i].nels);
  for (k=0; k<ga->inEdges[i].nels; k++)
    g_printerr ("%d ", ga->inEdges[i].els[k]);
  g_printerr ("(outEdges n=%d) ", ga->outEdges[i].nels);
  for (k=0; k<ga->outEdges[i].nels; k++)
    g_printerr ("%d ", ga->outEdges[i].els[k]);
  g_printerr ("\n");
}
*/

  for (i=0; i<ga->nnodes; i++) {
    g_list_free (inEdgeList[i]);
    g_list_free (outEdgeList[i]);
  }
  g_free (inEdgeList);
  g_free (outEdgeList);
}
