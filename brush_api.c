#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


void
GGOBI(moveBrush) (gint ulx, gint uly, ggobid *gg)  /*-- move brush and paint --*/
{
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  icoords pos;

  pos.x = ulx;
  pos.y = uly;

  brush_motion (&pos, true, false, cpanel, gg);
}

void
GGOBI(sizeBrush) (gint width, gint height, ggobid *gg)  /*-- resize brush without painting --*/
{
  splotd *sp = gg->current_splot;
  brush_coords *brush_pos = &gg->brush.brush_pos;

  brush_pos->x2 = brush_pos->x1 + width;
  brush_pos->y2 = brush_pos->y1 + height;

  splot_redraw (sp, QUICK, gg);  
}

