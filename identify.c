/* identify.c */

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

gint
find_nearest_point (icoords *lcursor_pos, splotd *splot)
{
/*
 * Returns index of nearest un-hidden point
*/
  gint i, k, sqdist, near, xdist, ydist, npoint;

  npoint = -1;

  near = 20*20;  /* If nothing is close, don't show any label */

  for (i=0; i<xg.nrows_in_plot; i++) {
    if (!xg.hidden_now[ k=xg.rows_in_plot[i] ]) {
      xdist = splot->screen[k].x - lcursor_pos->x;
      ydist = splot->screen[k].y - lcursor_pos->y;
      sqdist = xdist*xdist + ydist*ydist;
      if (sqdist < near) {
        near = sqdist;
        npoint = k;
      }
    }
  }
  return (npoint);
}
