#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


void
GGOBI(moveBrush) (gint ulx, gint uly)  /*-- move brush and paint --*/
{
  splotd *sp = gg.current_splot;
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  icoords pos;

  pos.x = ulx;
  pos.y = uly;

  brush_motion (&pos, true, false, cpanel);
}

void
GGOBI(sizeBrush) (gint width, gint height)  /*-- resize brush without painting --*/
{
  splotd *sp = gg.current_splot;

  gg.app.brush_pos.x2 = gg.app.brush_pos.x1 + width;
  gg.app.brush_pos.y2 = gg.app.brush_pos.y1 + height;

  splot_redraw (sp, QUICK);  
}

