/*-- brush-api.c --*/
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/


#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*-- move brush and paint --*/
void ggobi_moveBrush (gint ulx, gint uly, GGobiSession * gg)
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
void ggobi_sizeBrush (gint width, gint height, splotd * sp, GGobiSession * gg)
{
  brush_coords *brush_pos = &sp->brush_pos;

  brush_pos->x2 = brush_pos->x1 + width;
  brush_pos->y2 = brush_pos->y1 + height;

  splot_redraw (sp, QUICK, gg);
}
