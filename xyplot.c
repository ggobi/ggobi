#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

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
xy_reproject (splotd *sp, glong **world_data, ggobid *gg)
{
/*
 * Project the data down from the ncols_used-dimensional world_data[]
 * to the 2-dimensional array planar[].
*/
  gint i, m;
  gint jx = sp->xyvars.x;
  gint jy = sp->xyvars.y;

  for (i=0; i<gg->nrows; i++) {
    m = gg->rows_in_plot[i];

    sp->planar[m].x = world_data[m][jx];  /*-- regular or missings --*/
    sp->planar[m].y = world_data[m][jy];
  }
}

