/* weightedvis_ui.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


void
wvis_init (ggobid  *gg)
{
  GdkColormap *cmap = gdk_colormap_get_system ();
  gboolean writeable = false, best_match = true, success;

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

  gg->wvis.gray1.red = gg->wvis.gray1.blue = gg->wvis.gray1.green =
    (guint16) (.3*65535.0);
  success = gdk_colormap_alloc_color (cmap, &gg->wvis.gray1, writeable,
    best_match);
  gg->wvis.gray2.red = gg->wvis.gray2.blue = gg->wvis.gray2.green =
    (guint16) (.5*65535.0);
  success = gdk_colormap_alloc_color (cmap, &gg->wvis.gray2, writeable,
    best_match);
  gg->wvis.gray3.red = gg->wvis.gray3.blue = gg->wvis.gray3.green =
    (guint16) (.7*65535.0);
  success = gdk_colormap_alloc_color (cmap, &gg->wvis.gray3, writeable,
    best_match);
}
