#include <gtk/gtk.h>

#include "vars.h"

void
xy_reproject (splotd *sp)
{
/*
 * Project the data down from the ncols_used-dimensional world_data[]
 * to the 2-dimensional array planar[].
*/
  gint i;
  gint jx = sp->xyvars.x;
  gint jy = sp->xyvars.y;


  for (i=0; i<xg.nrows; i++) {
    sp->planar[i].x = xg.world_data[i][jx];
    sp->planar[i].y = xg.world_data[i][jy];
  }
}

