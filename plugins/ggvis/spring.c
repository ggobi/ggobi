/* molecule.c: run as part of xgvis ML 2/92. */
/* molecule.c: find layout with attraction/repulsion.
/* dists.c: called by netpad: writes out distance matrix.
/* ML: 11/91. */

/* Includes. */
#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"
#include "ggvis.h"

#include <sys/types.h>
#include <math.h>

/* Functions. */
void do_attract();          /* For working with vectors. */
void do_repel();
gfloat distance();

/* Macros. */
#define DOUBLE(x) ((gdouble)(x))
#define FLOAT(x) ((gfloat)(x))

void
spring_once (gint ndims, datad *d, datad *e, array_d *dist, array_d *pos)
{
  gint nNodes = d->nrows;
  gint nEdges = e->edge.n;
  gint dim, iter, i, j, k, a, b;
  gdouble **dv = dist->vals;
  gdouble dx, dy, dst, xf, yf, f, el;
  endpointsd *endpoints = e->edge.endpoints;

  gdouble *fx = (gdouble *) g_malloc0 (nNodes * sizeof (gdouble));
  gdouble *fy = (gdouble *) g_malloc0 (nNodes * sizeof (gdouble));

  /* 
      Lee Wilkinson:
      These parameters are critical and sensitive (based on extensive
      Monte Carlo testing):
          stiffness: resiliency of spring representing edge
          repulsion: force separating nodes
          stepsize: scale factor for gradient
          edgelength: zero-energy length of an edge
  */
  gdouble stiffness = .75;
  gdouble repulsion = .15;
  gdouble stepsize = .125;
  gdouble edgelength = .05/sqrt((gdouble)nNodes);

  for (dim = 1; dim < ndims; dim++) {

  for (iter = 0; iter < 10; iter++) {

      /* compute attractions between adjacent nodes using
         spring parameters */

    for (i = 0; i < nEdges; i++) {
      a = endpoints[i].a;
      b = endpoints[i].b;

      /*-- this does the first two dimensions only; test this first --*/
      dx = pos->vals[b][dim-1] - pos->vals[a][dim-1];
      dy = pos->vals[b][dim] - pos->vals[a][dim];

      dst = sqrt(dx*dx + dy*dy);
      xf = 0.;
      yf = 0.;
      if (dst > 0.) {
        el = edgelength;
/*
    We don't keep track of the node's degree
        if (indEdgeLengths>= 0)
          el = data[indEdgeLengths][i];
        else if (nodes[e.from].degree > 2 && nodes[e.to].degree > 2)
          el *= 1.5;
        else if (nodes[e.from].degree == 1 || nodes[e.to].degree == 1)
          el = 0.; 
*/
        f = stiffness * (el - dst) / dst;
        xf = dx * f;
        yf = dy * f;
        fx[b] += xf;
        fy[b] += yf;
        fx[a] -= xf;
        fy[a] -= yf;
      }
      else {        // jitter overlapping points
        gdouble FUZZ = .2;  /* Lee didn't specify this */
        xf += FUZZ*(randvalue() - .5);
        yf += FUZZ*(randvalue() - .5);
      }
    }

    /* compute repulsions for all pairs of nodes */
                
    for (i = 0; i < nNodes; i++) {
      xf = 0.;
      yf = 0.;
      for (j = 0 ; j < nNodes; j++) {
        if (i == j)
          continue;
        dx = pos->vals[i][dim-1] - pos->vals[j][dim-1];
        dy = pos->vals[i][dim] - pos->vals[j][dim];
        dst = dx*dx + dy*dy;
        if (dst > 0.) {
          xf += (dx / dst);
          yf += (dy / dst);
        }
        else {        // jitter
          gdouble FUZZ = .2;  /* Lee didn't specify this */
          xf += FUZZ*(randvalue() - .5);
          yf += FUZZ*(randvalue() - .5);
        }
      }
      dst = xf*xf + yf*yf;
      if (dst > 0.) {
        dst = sqrt(dst) / repulsion;
        fx[i] += (xf / dst);
        fy[i] += (yf / dst);
      }
    }

    /* move nodes and retain half of force-vector for
       next iteration */

    for (i = 0; i < nNodes; i++) {
      fx[i] = MAX(MIN(fx[i], 2.), -2.);
      fy[i] = MAX(MIN(fy[i], 2.), -2.);
      pos->vals[i][dim-1] += stepsize*fx[i];
      pos->vals[i][dim] += stepsize*fy[i];
      fx[i] /= 2.;
      fy[i] /= 2.;
    }
  }
  }

  g_free (fx);
  g_free (fy);

}

