/* scale-drag.c */
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

/*
 * scale_style == DRAG and button 1 is pressed; we are panning.
 * The mouse has moved to sp.mousepos from sp.mousepos_o. 
 * Change shift_wrld appropriately
*/
void
pan_by_drag (splotd *sp, ggobid *gg)
{
  gint dx, dy;
  gfloat scale_x, scale_y;
  cpaneld *cpanel = &gg->current_display->cpanel;

  dx = sp->mousepos.x - sp->mousepos_o.x;
  dy = sp->mousepos.y - sp->mousepos_o.y;

  scale_x = (cpanel->projection == TOUR2D) ? sp->tour_scale.x : sp->scale.x;
  scale_y = (cpanel->projection == TOUR2D) ? sp->tour_scale.y : sp->scale.y;

  scale_x /= 2;
  sp->iscale.x = (glong) ((gfloat) sp->max.x * scale_x);
  scale_y /= 2;
  sp->iscale.y = (glong) (-1 * (gfloat) sp->max.y * scale_y);

  sp->pmid.x -= ((dx * PRECISION1) / sp->iscale.x);
  sp->pmid.y -= ((dy * PRECISION1) / sp->iscale.y);
}

/*
 * scale_style == DRAG and button 2 is pressed; we are zooming. 
 * The mouse has moved to sp->mousepos from sp->mousepos_o and the center
 * of the figure is at sp->mid.  Change sp->scale by the
 * appropriate amounts.
*/
void
zoom_by_drag (splotd *sp, ggobid *gg)
{
  gint projection = projection_get (gg);
  gfloat *scale_x = (projection == TOUR2D) ? &sp->tour_scale.x : &sp->scale.x;
  gfloat *scale_y = (projection == TOUR2D) ? &sp->tour_scale.y : &sp->scale.y;
  gint npix = 10;  /*-- number of pixels from the crosshair required --*/

  icoords mid;
  fcoords scalefac;

  mid.x = sp->max.x / 2;
  mid.y = sp->max.y / 2;
  scalefac.x = scalefac.y = 1.0;

  if (ABS(sp->mousepos.x - mid.x) < npix)
    return;

  /*-- making the behavior identical to click zooming --*/
  scalefac.x = 
    (gfloat) (sp->mousepos.x - mid.x) / (gfloat) (sp->mousepos_o.x - mid.x);
  scalefac.y =
    (gfloat) (sp->mousepos.y - mid.y) / (gfloat) (sp->mousepos_o.y - mid.y);

  if (*scale_x * scalefac.x >= SCALE_MIN) {
    *scale_x = *scale_x * scalefac.x;
  }
  if (*scale_y * scalefac.y >= SCALE_MIN) {
    *scale_y = *scale_y * scalefac.y;
  }

}

