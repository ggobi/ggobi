#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>
#include <math.h>

#include "plugin.h"
#include "ggvis.h"


/*-----------------------------------------------------------------*/
/*                   callbacks                                     */
/*-----------------------------------------------------------------*/

void radial_cb (GtkButton *button, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  ggvisd *ggv = GGVisFromInst (inst);
  datad *d = gg->current_display->d;
  datad *e = gg->current_display->e;

  if (d == NULL || e == NULL)
    return;

  initLayout (gg, ggv, d, e);

  /*-- initial default:  let the first node be the center node --*/
  ggv->radial->centerNode = &ggv->radial->nodes[0];
  ggv->radial->centerNode->i = 0;

  setParentNodes (ggv, d);
  setNChildren (ggv, d);
  setSubtreeSize (ggv->radial->centerNode, ggv, d);

  setSubtreeSpans (ggv, d);  /*-- ok --*/

  setNodePositions (ggv, d);

/*-- add two variables and put in the new values --*/
  {
    gint i, k;
    gdouble *x = g_malloc0 (d->nrows * sizeof (gdouble));
    gdouble *y = g_malloc0 (d->nrows * sizeof (gdouble));
    gchar *name;

    /*-- here's a problem: if nrows != nrows_in_plot, this won't
         do the right thing --*/
    for (i=0; i<d->nrows; i++) {
      x[i] = ggv->radial->nodes[i].pos.x;
      y[i] = ggv->radial->nodes[i].pos.y;
    }

    name = g_strdup_printf ("x");
    newvar_add_with_values (x, d->nrows, name, d, gg);
    g_free (name);
    g_free (x);
    name = g_strdup_printf ("y");
    newvar_add_with_values (y, d->nrows, name, d, gg);
    g_free (name);
    g_free (y);
  }
}

/*-- this should be in radial.c or radial_ui.c --*/
void highlight_edges_cb (GtkButton *button, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  ggvisd *ggv = GGVisFromInst (inst);
  datad *d = gg->current_display->d;
  datad *e = gg->current_display->e;

/*
for each sticky label index k in d
  for each edge
    if a == k || b == k
      highlight the edge in e -- for starters
    else unhighlight the edge

in the long term, there could be a nicer way to do this:
  let's add two events to ggobi: edge becomes sticky and
  edge becomes unsticky
*/
}

/*-----------------------------------------------------------------*/

/*
 * Initialize a couple of values for each node.
*/
void
initLayout (ggobid *gg, ggvisd *ggv, datad *d, datad *e) {
  gint i, k, nn;
  noded *na, *nb;
  gint nnodes = d->nrows_in_plot;
  gint nnodessq = nnodes * nnodes;
  gint nedges = e->edge.n;
  noded *nodes;
  endpointsd *endpoints = e->edge.endpoints;
  gint a, b;

  if (ggv->radial != NULL) {
    nn = sizeof (ggv->radial->nodes) / sizeof (ggv->radial);
    for (i=0; i < nn; i++) {
      g_list_free (ggv->radial->nodes[i].edges);
      g_list_free (ggv->radial->nodes[i].connectedNodes);
    }
    g_free (ggv->radial->nodes);
  }

  ggv->radial = (radiald *) g_malloc (sizeof (radiald));
  ggv->radial->nodes = (noded *) g_malloc (nnodes * sizeof (noded));
  nodes = ggv->radial->nodes;

  for (i = 0; i <nnodes; i++) {
    k = d->rows_in_plot[i];
    ggv->radial->nodes[i].edges = NULL;
    ggv->radial->nodes[i].subtreeSize = 0;
    ggv->radial->nodes[i].nChildren = 0;
    ggv->radial->nodes[i].nStepsToCenter = nnodessq;
    ggv->radial->nodes[i].i = k;
    ggv->radial->nodes[i].connectedNodes = NULL;
    ggv->radial->nodes[i].parentNode = NULL;

    if (nedges <= 1) {
      ggv->radial->nodes[i].nStepsToLeaf = 0;
      ggv->radial->nodes[i].nChildren = 0;
    } else {
      ggv->radial->nodes[i].nStepsToLeaf = nnodessq;
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
        /*
        na->edges = g_list_append (na->edges, GINT_TO_POINTER (b));
        nb->edges = g_list_append (nb->edges, GINT_TO_POINTER (a));
        */

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

  noded *centerNode = ggv->radial->centerNode;

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
  }
}


void setNChildren (ggvisd *ggv, datad *d)
{
  gint i;
  noded *n;

  for (i=0; i<d->nrows_in_plot; i++) {
    n = &ggv->radial->nodes[i];
    if (n->parentNode != NULL) {
      n->parentNode->nChildren++;
    }
  }

/*-- debug --*/
/*
  for (i=0; i<d->nrows_in_plot; i++) {
    n = &ggv->radial->nodes[i];
    g_printerr ("node %d children %d\n", n->i, n->nChildren);
  }
*/
}

/*
 * This is currently being computed three times; once ought
 * to be enough.
*/
static void
childNodes (GList **children, noded *n) {
  GList *l;
  noded *n1;

  for (l = n->connectedNodes; l; l = l->next) {
    n1 = (noded *) l->data;

    if (n1->parentNode != NULL && n1->parentNode->i == n->i)
      if (g_list_index (*children, n1) == -1)
        *children = g_list_append (*children, n1);
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
  noded *nchild;
  GList *l, *children = NULL;

  childNodes (&children, n);

  for (l = children; l; l = l->next) {
    nchild = (noded *) l->data;

    if (nchild->nChildren == 0)
      n->subtreeSize += 1;
    else
      n->subtreeSize += setSubtreeSize (nchild, ggv, d);
  }
  /*g_printerr ("node %d subtreeSize %d\n", n->i, n->subtreeSize);*/
  return (n->subtreeSize);
}

/*---------------------------------------------------------------------*/

static void
setChildSubtreeSpans (noded *n, ggvisd *ggv, datad *d)
{
  noded *nchild;
  GList *l, *children = NULL;

  childNodes (&children, n);

  for (l = children; l; l = l->next) {
    nchild = (noded *) l->data;

    nchild->span = n->span * nchild->subtreeSize / n->subtreeSize;

    if (nchild->nChildren > 0) {
      setChildSubtreeSpans (nchild, ggv, d);
    }
  }
}


void
setSubtreeSpans (ggvisd *ggv, datad *d) {
  ggv->radial->centerNode->span = 2*M_PI;
  setChildSubtreeSpans (ggv->radial->centerNode, ggv, d);
}

/*---------------------------------------------------------------------*/

  // Set the node positions for the 2nd and later rings.
static void
setChildNodePositions (noded *n, ggvisd *ggv, datad *d)
{
  gint i;
  noded *nchild;
  gdouble theta;
  GList *l, *children = NULL;

  // the initial value of theta is the angle of the boundary of the fan
  if (n->i == ggv->radial->centerNode->i) theta = 0;
  else if (n->nChildren == 1) theta = n->theta;
  else {
    theta = n->theta - n->span/2;
  }

  // Build an array of the child nodes 
  childNodes (&children, n);

  i = 0;
  for (l = children; l; l = l->next) {
    nchild = (noded *) l->data;

    if (i == 0) {
      nchild->theta = theta;

      if (nchild->span > 0)
        theta += nchild->span/2;
      else  // if it's a leaf node
        theta += .5 * (n->span)/(gdouble)(n->subtreeSize-1);

      i++;
    } else {
      nchild->theta = theta + nchild->span/2;

      if (nchild->span > 0)
        theta += nchild->span;
      else  // if it's a leaf node
        theta += (n->span)/(gdouble)(n->subtreeSize-1);
    }
      
    nchild->pos.x = nchild->nStepsToCenter * cos(nchild->theta);
    nchild->pos.y = nchild->nStepsToCenter * sin(nchild->theta);


    if (nchild->nChildren > 0)
      setChildNodePositions(nchild, ggv, d);
  }
}


void
setNodePositions (ggvisd *ggv, datad *d) {

  // Set the position of the center node
  ggv->radial->centerNode->pos.x = 0;
  ggv->radial->centerNode->pos.y = 0;
  ggv->radial->centerNode->theta = 0;

  setChildNodePositions (ggv->radial->centerNode, ggv, d);
}

