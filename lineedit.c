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
  gg.segment_endpoints = (endpointsd *)
    g_realloc (gg.segment_endpoints, nsegs * sizeof (endpointsd));
}

void
segments_free ()
{
  g_free ((gpointer) gg.segment_endpoints);
}

/* --------------------------------------------------------------- */
/*               End of dynamic allocation section                 */
/* --------------------------------------------------------------- */

void
segments_create ()
{
  gint i;

  gg.nsegments = gg.nrows - 1;
  gg.segment_endpoints = (endpointsd *)
    g_realloc ((gpointer) gg.segment_endpoints,
    gg.nsegments * sizeof (endpointsd));

  for (i=0; i<gg.nsegments; i++) {
    gg.segment_endpoints[i].a = i+1;
    gg.segment_endpoints[i].b = i+2;
  }
}
