/* wvis.c */

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


/**
 Initialize basic colors for this ggobid.
 */
void
wvis_init (ggobid  *gg)
{
  gg->wvis.window = NULL;
  gg->wvis.npct = 0;
  gg->wvis.n = NULL;
  gg->wvis.nearest_color = -1;
  gg->wvis.motion_notify_id = 0;
  gg->wvis.mousepos.x = -1;
  gg->wvis.mousepos.y = -1;
  gg->wvis.pix = NULL;

  gg->wvis.binning_method = WVIS_EQUAL_WIDTH_BINS;
  gg->wvis.update_method = WVIS_UPDATE_ON_MOUSE_UP;

  gg->wvis.scheme = NULL;
  gg->wvis.GC = NULL;
}
