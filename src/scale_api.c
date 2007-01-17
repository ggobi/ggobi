/* scale_api.c: some api scaling routines */
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

/*
   I didn't do these right, I don't think, because pan takes
   incremental steps, while zoom resets the scale value in an
   absolute way.  Maybe the way I did the zooming is the way one
   would like.  If that's so, it could be renamed splot_zoom_set
   and left as it is.  splot_pan could be renamed splot_pan_set
   and reworked.
*/

/*
void
splot_pan (splotd *sp, gint xstep, gint ystep, GGobiSession *gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  displayd *display = gg->current_display;

  if (cpanel->scale_pan_opt == P_OBLIQUE ||
      cpanel->scale_pan_opt == P_HORIZ)
  {
  }

  if (cpanel->scale_pan_opt == P_OBLIQUE ||
      cpanel->scale_pan_opt == P_VERT)
  {
  }

  splot_plane_to_screen (display, cpanel, sp, gg);
  ruler_ranges_set (false, gg->current_display, sp, gg);
  splot_redraw (sp, FULL, gg);
}
*/

/*-- Set the current scale:  xsc, ysc are on (SCALE_MIN, ...) */
void
splot_zoom (splotd * sp, gfloat xsc, gfloat ysc)
{
  GGobiSession *gg = GGobiFromSPlot(sp);
  displayd *display = gg->current_display;
  /*gint projection = projection_get (gg); */
  icoords mid;
  /*  gfloat *scale_x = (projection == TOUR2D) ? &sp->tour_scale.x : &sp->scale.x;
     gfloat *scale_y = (projection == TOUR2D) ? &sp->tour_scale.y : &sp->scale.y; */
  gfloat *scale_x = &sp->scale.x;
  gfloat *scale_y = &sp->scale.y;
  gfloat scalefac_x = xsc / *scale_x;
  gfloat scalefac_y = ysc / *scale_y;

  mid.x = sp->max.x / 2;
  mid.y = sp->max.y / 2;

  if (xsc > SCALE_MIN && *scale_x * scalefac_x >= SCALE_MIN) {
    *scale_x = xsc;
  }
  if (scalefac_y > SCALE_MIN && *scale_y * scalefac_y >= SCALE_MIN) {
    *scale_y = ysc;
  }

  splot_plane_to_screen (display, &display->cpanel, sp, gg);
  ruler_ranges_set (false, gg->current_display, sp, gg);
  splot_redraw (sp, FULL, gg);
}
