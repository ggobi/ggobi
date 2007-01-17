/* schemes.c */

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


/**
 Initialize basic colors for this GGobiSession.
 */
void
svis_init (GGobiSession * gg)
{
  gg->svis.window = NULL;
  gg->svis.npct = 0;
  gg->svis.pix = NULL;
  gg->svis.scheme = NULL;
  gg->svis.GC = NULL;
}
