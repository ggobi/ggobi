/* scale_click.c: pan and zoom for the CLICK interaction style */
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

void
pan_step (splotd *sp, gint pan_opt, ggobid *gg)
{
  greal dx, dy;
  greal scale_x, scale_y;
  /*cpaneld *cpanel = &gg->current_display->cpanel;*/
  greal precis = (greal) PRECISION1;

  if (pan_opt == P_OBLIQUE || pan_opt == P_HORIZ) {
    dx = (greal) (sp->mousepos.x - sp->max.x/2);
    /*    scale_x = (greal)
	  ((cpanel->projection == TOUR2D) ? sp->tour_scale.x : sp->scale.x);*/
    scale_x = (greal) sp->scale.x;
    scale_x /= 2;
    sp->iscale.x = (greal) sp->max.x * scale_x;
    sp->pmid.x -= (dx * precis / sp->iscale.x);
  }


  if (pan_opt == P_OBLIQUE || pan_opt == P_VERT) {
    dy = (greal) (sp->mousepos.y - sp->max.y/2);
    /*    scale_y = (greal)
	  ((cpanel->projection == TOUR2D) ? sp->tour_scale.y : sp->scale.y);*/
    scale_y = (greal) sp->scale.y;
    scale_y /= 2;
    sp->iscale.y = (greal) sp->max.y * scale_y;
    sp->pmid.y += (dy * precis / sp->iscale.y);
  }
}

void
zoom_step (splotd *sp, gint zoom_opt, gint in_or_out, rectd *rect, ggobid* gg)
{
/*
  gint projection = projection_get (gg);
  gfloat *scale_x = (projection == TOUR2D) ? &sp->tour_scale.x : &sp->scale.x;
  gfloat *scale_y = (projection == TOUR2D) ? &sp->tour_scale.y : &sp->scale.y;
*/
  gfloat *scale_x = &sp->scale.x;
  gfloat *scale_y = &sp->scale.y;
  gfloat scalefac_x = 1.0, scalefac_y = 1.0;
  icoords mid;

  mid.x = sp->max.x / 2;
  mid.y = sp->max.y / 2;

  switch (zoom_opt) {
    case Z_OBLIQUE:
    case Z_ASPECT:
      scalefac_x = (in_or_out == ZOOM_IN) ?
        (gfloat) mid.x / (gfloat) (mid.x - rect->x) :
        (gfloat) (mid.x - rect->x) / (gfloat) mid.x;
      scalefac_y = (in_or_out == ZOOM_IN) ?
        (gfloat) mid.y / (gfloat) (mid.y - rect->y) :
        (gfloat) (mid.y - rect->y) / (gfloat) mid.y;
    break;
    case Z_HORIZ:
      scalefac_x = (in_or_out == ZOOM_IN) ?
        (gfloat) mid.x / (gfloat) (mid.x - rect->x) :
        (gfloat) (mid.x - rect->x) / (gfloat) mid.x;
    break;
    case Z_VERT:
      scalefac_y = (in_or_out == ZOOM_IN) ?
        (gfloat) mid.y / (gfloat) (mid.y - rect->y) :
        (gfloat) (mid.y - rect->y) / (gfloat) mid.y;
    break;
  }

/*-- Make sure that the scale doesn't get too small --*/
  if (*scale_x * scalefac_x >= SCALE_MIN) {
    *scale_x = *scale_x * scalefac_x;
  }
  if (*scale_y * scalefac_y >= SCALE_MIN) {
    *scale_y = *scale_y * scalefac_y;
  }
}

