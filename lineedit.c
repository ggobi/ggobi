/* lineedit.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/* --------------------------------------------------------------- */
/*                   Dynamic allocation section                    */
/* --------------------------------------------------------------- */

/*
 * The brushing arrays are handled in brush_init.c, and the
 * segments and arrowheads arrays are handled in splot.c
*/

void
segments_alloc (gint nsegs)
{
  xg.segment_endpoints = (endpointsd *)
    g_realloc (xg.segment_endpoints, nsegs * sizeof (endpointsd));
}

void
segments_free ()
{
  g_free ((gpointer) xg.segment_endpoints);
}

/* --------------------------------------------------------------- */
/*               End of dynamic allocation section                 */
/* --------------------------------------------------------------- */

void
segments_create ()
{
  gint i;

  xg.nsegments = xg.nrows - 1;
  xg.segment_endpoints = (endpointsd *)
    g_realloc ((gpointer) xg.segment_endpoints,
    xg.nsegments * sizeof (endpointsd));

  for (i=0; i<xg.nsegments; i++) {
    xg.segment_endpoints[i].a = i+1;
    xg.segment_endpoints[i].b = i+2;
  }
}
