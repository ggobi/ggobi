#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

extern brush_coords brush_pos;  /*-- from brush.c --*/

void
moveBrush (gint ulx, gint uly)  /*-- move brush and paint --*/
{
  splotd *sp = current_splot;
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
  splotd *sp = current_splot;

  brush_pos.x2 = brush_pos.x1 + width;
  brush_pos.y2 = brush_pos.y1 + height;

  splot_redraw (sp, QUICK);  
}

