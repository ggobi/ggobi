/*-- brush-api.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
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
  icoords pos;

  pos.x = ulx;
  pos.y = uly;

  brush_motion (&pos, true, false, cpanel, sp, gg);
}

/*-- resize brush without painting --*/
void
GGOBI(sizeBrush) (gint width, gint height, splotd *sp, ggobid *gg) 
{
  brush_coords *brush_pos = &sp->brush_pos;

  brush_pos->x2 = brush_pos->x1 + width;
  brush_pos->y2 = brush_pos->y1 + height;

  splot_redraw (sp, QUICK, gg);  
}

