#include <gtk/gtk.h>
#include "vars.h"

gboolean
xyplot_varsel (splotd *sp, gint jvar, gint *jvar_prev, gint button)
{
  gboolean redraw = true;

  if (button == 1) {
    if (jvar == sp->xyvars.x)
      redraw = false;
    else if (jvar == sp->xyvars.y) {
      sp->xyvars.y = sp->xyvars.x;
      *jvar_prev = sp->xyvars.x;
    } else {
      *jvar_prev = sp->xyvars.x;
    }
    sp->xyvars.x = jvar;
  } else if (button == 2 || button == 3) {
    if (jvar == sp->xyvars.y)
      redraw = false;
    else if (jvar == sp->xyvars.x) {
      sp->xyvars.x = sp->xyvars.y;
      *jvar_prev = sp->xyvars.y;
    } else {
      *jvar_prev = sp->xyvars.y;
    }
    sp->xyvars.y = jvar;
  }

  return redraw;
}

void
xy_reproject (splotd *sp, glong **world_data)
{
/*
 * Project the data down from the ncols_used-dimensional world_data[]
 * to the 2-dimensional array planar[].
*/
  gint i, m;
  gint jx = sp->xyvars.x;
  gint jy = sp->xyvars.y;

  for (i=0; i<xg.nrows; i++) {
    m = xg.rows_in_plot[i];
    sp->planar[i].x = world_data[i][jx];  /*-- regular or missings --*/
    sp->planar[i].y = world_data[i][jy];
  }
}

