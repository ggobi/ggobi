/* identify.c */

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

gint
find_nearest_point (icoords *cursor_pos, splotd *sp)
{
/*
 * Returns index of nearest un-erased point
*/
  gint i, k, sqdist, near, xdist, ydist, npoint;

  npoint = -1;

  near = 20*20;  /* If nothing is close, don't show any label */

  for (i=0; i<xg.nrows_in_plot; i++) {
    if (!xg.erased_now[ k=xg.rows_in_plot[i] ]) {
      xdist = sp->screen[k].x - cursor_pos->x;
      ydist = sp->screen[k].y - cursor_pos->y;
      sqdist = xdist*xdist + ydist*ydist;
      if (sqdist < near) {
        near = sqdist;
        npoint = k;
      }
    }
  }
  return (npoint);
}
