/* lineedit.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/* --------------------------------------------------------------- */
/*                   Dynamic allocation section                    */
/* --------------------------------------------------------------- */

/*
 * The brushing arrays are handled in brush_init.c, and the
 * edges and arrowheads arrays are handled in splot.c
*/

void
edges_alloc (gint nsegs, datad *d, ggobid *gg)
{
  gg->edge_endpoints = (endpointsd *)
    g_realloc (gg->edge_endpoints, nsegs * sizeof (endpointsd));
}

void
edges_free (datad *d, ggobid *gg)
{
  g_free ((gpointer) gg->edge_endpoints);
}

/* --------------------------------------------------------------- */
/*               End of dynamic allocation section                 */
/* --------------------------------------------------------------- */

void
edges_create (datad *d, ggobid *gg)
{
  gint i;

  gg->nedges = d->nrows - 1;
  gg->edge_endpoints = (endpointsd *)
    g_realloc ((gpointer) gg->edge_endpoints,
    gg->nedges * sizeof (endpointsd));

  for (i=0; i<gg->nedges; i++) {
    gg->edge_endpoints[i].a = i+1;
    gg->edge_endpoints[i].b = i+2;
  }
}
