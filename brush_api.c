#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


void
moveBrush (gint ulx, gint uly)  /*-- move brush and paint --*/
{
  splotd *sp = xg.current_splot;
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  icoords pos;

  pos.x = ulx;
  pos.y = uly;

  brush_motion (&pos, true, false, cpanel);
}

void
sizeBrush (gint width, gint height)  /*-- resize brush without painting --*/
{
  splotd *sp = xg.current_splot;

  xg.app.brush_pos.x2 = xg.app.brush_pos.x1 + width;
  xg.app.brush_pos.y2 = xg.app.brush_pos.y1 + height;

  splot_redraw (sp, QUICK);  
}

