#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>
#include <math.h>

#include "plugin.h"
#include "ggvis.h"

/*
 * Initialize a couple of values for each node.
*/
void
initLayout (ggobid *gg, ggvisd *ggv, datad *d, datad *e) {
  gint i, k, nn;
  noded *na, *nb;
  radiald *radial = ggv->radial;
  gint nnodes = d->nrows_in_plot;
  gint nnodessq = nnodes * nnodes;
  gint nedges = e->edge.n;
  noded *nodes;
  endpointsd *endpoints = e->edge.endpoints;
  gint a, b;

  if (radial != NULL) {
    nn = sizeof (radial->nodes) / sizeof (radial);
    for (i=0; i < nn; i++) {
      g_list_free (radial->nodes[i].edges);
      g_list_free (radial->nodes[i].connectedNodes);
    }
    g_free (radial->nodes);
  }

  radial = (radiald *) g_malloc (sizeof (radiald));
  radial->nodes = (noded *) g_malloc (nnodes * sizeof (noded));
  nodes = radial->nodes;

  for (i = 0; i <d->nrows_in_plot; i++) {
    k = d->rows_in_plot[i];
    nodes[k].edges = NULL;
    nodes[k].subtreeSize = 0;
    nodes[k].nChildren = 0;
    nodes[k].nStepsToCenter = nnodessq;
    nodes[k].i = k;

    if (nedges <= 1) {
      nodes[k].nStepsToLeaf = 0;
      nodes[k].nChildren = 0;
    } else {
      nodes[k].nStepsToLeaf = nnodessq;
    }
  }

  /*-- initialize the linked lists of edges and nodes --*/

  /*-- loop over the edges --*/
  /*
   * Do I check both hidden and sampled?  Suppose I check sampled
   * here, so that the lists are no larger than the current subset,
   * but I save checking hidden for later, when running the layout
   * algorithms.
  */
  for (i = 0; i <e->edge.n; i++) {
    if (e->sampled.els[i]) {
      a = d->rowid.idv.els[endpoints[i].a];
      b = d->rowid.idv.els[endpoints[i].b];
      if (d->sampled.els[a] && d->sampled.els[b]) {
        /*-- add b to edges for a, and vice versa --*/
        /*-- alternative:  add a pointer to the endpoints object --*/
        na->edges = g_list_append (na->edges, GINT_TO_POINTER (b));
        nb->edges = g_list_append (nb->edges, GINT_TO_POINTER (a));

        /*-- add b to connectedNodes for a, and vice versa --*/
        na = &ggv->radial->nodes[a];
        nb = &ggv->radial->nodes[b];
        na->connectedNodes = g_list_append (na->connectedNodes, nb);
        nb->connectedNodes = g_list_append (nb->connectedNodes, na);
      }
    }
  }
}

void
setNStepsToCenter (noded *n, noded *prevNeighbor) {
  noded *n1;
  gint nsteps = n->nStepsToCenter + 1;
  GList *l;

  for (l = n->connectedNodes; l; l = l->next) {
    n1 = (noded *) l->data;
    if (prevNeighbor != NULL && n1->i == prevNeighbor->i)
      continue;

    if (nsteps < n1->nStepsToCenter) {
      n1->nStepsToCenter = nsteps;
      n1->parentNode = n;
      setNStepsToCenter (n1, n);
    }
  }
}


/*
 * Work out from the center and determine the value of
 * nStepsToCenter and parent node for each node.
*/
void
setParentNodes (ggvisd *ggv, datad *d) {
  gint i;
  noded *n;

  noded *centerNode = &ggv->radial->centerNode;

  centerNode->nStepsToCenter = 0;
  centerNode->parentNode = NULL;
  setNStepsToCenter (centerNode, NULL);

  // find the maximum number of steps from the center
  ggv->radial->nStepsToCenter = 0;
  for (i=0; i<d->nrows_in_plot; i++) {
    n = &ggv->radial->nodes[i];
    if (n->nStepsToCenter > ggv->radial->nStepsToCenter) {
      ggv->radial->nStepsToCenter = n->nStepsToCenter;
    }
    g_printerr ("node %d parent %d\n", n->i, n->parentNode->i);
  }
}


void setNChildren (ggvisd *ggv, datad *d)
{
  gint i;
  noded *n;

  for (i=0; i<d->nrows_in_plot; i++) {
    n = &ggv->radial->nodes[i];
    if (n->parentNode != NULL)
      n->parentNode->nChildren++;
  }
}


/*
 * Once the parent node is irrevocably set (once setParentNodes and
 * setNChildren are through), then it's possible to compute subtreeSize
 * for each node.
 *
 * Work out from the center ...
*/
gint
setSubtreeSize (noded *n, ggvisd *ggv, datad *d) {
  gint i;
  noded *nchild;

  for (i=0; i<d->nrows_in_plot; i++) {
    n = &ggv->radial->nodes[i];

    if (nchild->nChildren == 0)
      n->subtreeSize += 1;
    else
      n->subtreeSize += setSubtreeSize (nchild, ggv, d);
  }
  // System.out.println (n +" " + n.subtreeSize);
  return (n->subtreeSize);
}

/*---------------------------------------------------------------------*/

static void
setChildSubtreeSpans (noded *n, ggvisd *ggv, datad *d)
{
  gint i;
  noded *nchild;

  for (i=0; i<d->nrows_in_plot; i++) {
    nchild = &ggv->radial->nodes[i];

    nchild->span = n->span * nchild->subtreeSize / n->subtreeSize;

    if (nchild->nChildren > 0) {
      setChildSubtreeSpans (nchild, ggv, d);
    }
  }
}


void
setSubtreeSpans (ggvisd *ggv, datad *d) {
  ggv->radial->centerNode.span = 2*M_PI;
  setChildSubtreeSpans (&ggv->radial->centerNode, ggv, d);
}

/*---------------------------------------------------------------------*/

static void
childNodes (GList *l, noded *n) {
  noded *n1;

  for (l = n->connectedNodes; l; l = l->next) {
    n1 = (noded *) l->data;

    if (n1->parentNode->i == n->i)
      if (g_list_index (l, n1) == -1)
        l = g_list_append (l, n1);
  }
}

  // Set the node positions for the 2nd and later rings.
static void
setChildPositions (noded *n, ggvisd *ggv, datad *d)
{
  gint i;
  noded *nchild;
  gdouble theta;
  GList *l, *children = NULL;

  /*-- scale the positions onto 1, 100? --*/

  gdouble y = 6.0;  /*-- where does this come from? --*/
  gdouble x =
    (gdouble) ((n->nStepsToCenter+1) * 50.0) /
    (gdouble) ggv->radial->nStepsToCenter;
  gdouble symbolSpan = atan (y/x);

  // theta is the boundary of the fan
  if (n->i == ggv->radial->centerNode.i) theta = 0;
  else if (n->nChildren == 1) theta = n->theta;
  else {
    // With 2*symbolSpan, I get the behavior I would expect to
    // see with 1*symbolSpan.  With 4*symbolSpan, I get a nice
    // space between fans, but then very small fans get too
    // compressed.  3 seems to be a reasonable compromise.
    gdouble span = n->span - 3*symbolSpan;
    theta = n->theta - span/2;
  }

  // Build an array of the child nodes -- for the purpose
  // of sorting them -- how about an alphabetical sorting
  // by label?  Do I have one?
  childNodes (children, n);

  for (l = children; l; l->next) {
    nchild = (noded *) l->data;

    nchild->theta = theta + nchild->span/2.0 ;

    nchild->pos.x = nchild->nStepsToCenter * cos(nchild->theta);
    nchild->pos.y = nchild->nStepsToCenter * sin(nchild->theta);

    if (nchild->span > 0)
      theta += nchild->span;
    else  // if it's a leaf node
      theta += (n->span - 3*symbolSpan)/(n->subtreeSize-1);

    if (nchild->nChildren > 0)
      setChildPositions(nchild, ggv, d);
  }
}


void
setPositions (ggvisd *ggv, datad *d) {

  // Set the position of the center node
  ggv->radial->centerNode.pos.x = 0;
  ggv->radial->centerNode.pos.y = 0;
  ggv->radial->centerNode.theta = 0;

  setChildPositions (&ggv->radial->centerNode, ggv, d);
}

