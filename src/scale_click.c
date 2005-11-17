/* scale_click.c: pan and zoom for the CLICK interaction style */
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

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <math.h>
#include "vars.h"
#include "externs.h"


void
pan_step_key (splotd *sp, guint keyval, ggobid *gg)
{
  greal dx, dy;
  greal scale_x, scale_y;
  greal precis = (greal) PRECISION1;
  greal diff;

  if (keyval == GDK_Left || keyval == GDK_Right) {
    dx = (greal) fabs(sp->mousepos.x - sp->max.x/2);
    scale_x = (greal) sp->scale.x;
    scale_x /= 2;
    sp->iscale.x = (greal) sp->max.x * scale_x;
    diff = (dx * precis / sp->iscale.x);
    if (keyval == GDK_Left)
      sp->pmid.x += diff;
    else sp->pmid.x -= diff;
  } else if (keyval == GDK_Down || keyval == GDK_Up) {
    dy = (greal) fabs(sp->mousepos.y - sp->max.y/2);
    scale_y = (greal) sp->scale.y;
    scale_y /= 2;
    sp->iscale.y = (greal) sp->max.y * scale_y;
    diff = (dy * precis / sp->iscale.y);
    if (keyval == GDK_Down)
      sp->pmid.y += diff;
    else sp->pmid.y -= diff;
  }

}

void
pan_step (splotd *sp, gint pan_opt, ggobid *gg)
{
  greal dx, dy;
  greal scale_x, scale_y;
  greal precis = (greal) PRECISION1;

  if (pan_opt == P_OBLIQUE || pan_opt == P_HORIZ) {
    dx = (greal) (sp->mousepos.x - sp->max.x/2);
    scale_x = (greal) sp->scale.x;
    scale_x /= 2;
    sp->iscale.x = (greal) sp->max.x * scale_x;
    sp->pmid.x -= (dx * precis / sp->iscale.x);
  }


  if (pan_opt == P_OBLIQUE || pan_opt == P_VERT) {
    dy = (greal) (sp->mousepos.y - sp->max.y/2);
    scale_y = (greal) sp->scale.y;
    scale_y /= 2;
    sp->iscale.y = (greal) sp->max.y * scale_y;
    sp->pmid.y += (dy * precis / sp->iscale.y);
  }
}

void
zoom_step (splotd *sp, gint zoom_opt, gint in_or_out, rectd *rect, ggobid* gg)
{
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

