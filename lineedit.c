/* lineedit.c */

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
edges_alloc (gint nsegs, ggobid *gg)
{
  gg->edge_endpoints = (endpointsd *)
    g_realloc (gg->edge_endpoints, nsegs * sizeof (endpointsd));
}

void
edges_free (ggobid *gg)
{
  g_free ((gpointer) gg->edge_endpoints);
}

/* --------------------------------------------------------------- */
/*               End of dynamic allocation section                 */
/* --------------------------------------------------------------- */

void
edges_create (ggobid *gg)
{
  gint i;

  gg->nedges = gg->nrows - 1;
  gg->edge_endpoints = (endpointsd *)
    g_realloc ((gpointer) gg->edge_endpoints,
    gg->nedges * sizeof (endpointsd));

  for (i=0; i<gg->nedges; i++) {
    gg->edge_endpoints[i].a = i+1;
    gg->edge_endpoints[i].b = i+2;
  }
}
