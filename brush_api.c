#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*-- move brush and paint --*/
void
GGOBI(moveBrush) (gint ulx, gint uly, ggobid *gg) 
{
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  datad *d = gg->current_display->d;
  icoords pos;

  pos.x = ulx;
  pos.y = uly;

  brush_motion (&pos, true, false, cpanel, d, gg);
}

/*-- resize brush without painting --*/
void
GGOBI(sizeBrush) (gint width, gint height, ggobid *gg) 
{
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  brush_coords *brush_pos = &d->brush_pos;

  brush_pos->x2 = brush_pos->x1 + width;
  brush_pos->y2 = brush_pos->y1 + height;

  splot_redraw (sp, QUICK, gg);  
}

