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
#include <math.h>
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
  greal dx, dy;
  greal scale_x, scale_y;
  cpaneld *cpanel = &gg->current_display->cpanel;
  greal precis = (greal) PRECISION1;

  dx = (greal) (sp->mousepos.x - sp->mousepos_o.x);
  dy = (greal) (sp->mousepos.y - sp->mousepos_o.y);

  /*  scale_x = (greal)
    ((cpanel->projection == TOUR2D) ? sp->tour_scale.x : sp->scale.x);
  scale_y = (greal)
  ((cpanel->projection == TOUR2D) ? sp->tour_scale.y : sp->scale.y);*/
  scale_x = (greal) sp->scale.x;
  scale_y = (greal) sp->scale.y;

/*
 * This section is a bit puzzling, because I don't know what
 * would change this -- maybe resizing the plot window?
*/
  scale_x /= 2;
  sp->iscale.x = (greal) sp->max.x * scale_x;
  scale_y /= 2;
  sp->iscale.y = -1 * (greal) sp->max.y * scale_y;

  sp->pmid.x -= (dx * precis / sp->iscale.x);
  sp->pmid.y -= (dy * precis / sp->iscale.y);
}

/*
 * scale_style == DRAG and button 2 (or 3) is pressed; we are zooming. 
 * The mouse has moved to sp->mousepos from sp->mousepos_o and the center
 * of the figure is at sp->mid.  Change sp->scale by the
 * appropriate amounts.
*/
void
zoom_by_drag (splotd *sp, ggobid *gg)
{
  gint projection = projection_get (gg);
  /*  gfloat *scale_x = (projection == TOUR2D) ? &sp->tour_scale.x : &sp->scale.x;
      gfloat *scale_y = (projection == TOUR2D) ? &sp->tour_scale.y : &sp->scale.y;*/
  gfloat *scale_x = &sp->scale.x;
  gfloat *scale_y = &sp->scale.y;
  gint npix = 5;  /*-- number of pixels from the crosshair required --*/

  icoords mid;
  fcoords scalefac;

  mid.x = sp->max.x / 2;
  mid.y = sp->max.y / 2;
  scalefac.x = scalefac.y = 1.0;

  if (ABS(sp->mousepos.x - mid.x) >= npix) {
    /*-- making the behavior identical to click zooming --*/
    scalefac.x = 
      (gfloat) (sp->mousepos.x - mid.x) / (gfloat) (sp->mousepos_o.x - mid.x);

/*
 * Trying to make it behave better around mid.x, but it doesn't 
 * seem to help much.
    scalefac.x = (gfloat) sqrt((gdouble)scalefac.x);
*/

    if (*scale_x * scalefac.x >= SCALE_MIN) {
      *scale_x = *scale_x * scalefac.x;
    }
  }

  if (ABS(sp->mousepos.y - mid.y) >= npix) {
    /*-- making the behavior identical to click zooming --*/
    scalefac.y =
      (gfloat) (sp->mousepos.y - mid.y) / (gfloat) (sp->mousepos_o.y - mid.y);
    if (*scale_y * scalefac.y >= SCALE_MIN) {
      *scale_y = *scale_y * scalefac.y;
    }
  }

}

