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
  sp->ishift.x += (sp->mousepos.x - sp->mousepos_o.x);
  sp->ishift.y += (sp->mousepos.y - sp->mousepos_o.y);
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

  /*-- Scale the scaler if far enough from center --*/
  if (sp->mousepos_o.x - sp->ishift.x > npix ||
      sp->ishift.x - sp->mousepos_o.x > npix)
  {
    *scale_x *= ((gfloat) (sp->mousepos.x - sp->ishift.x) /
                 (gfloat) (sp->mousepos_o.x - sp->ishift.x));
  }

  if (sp->mousepos_o.y - sp->ishift.y > npix ||
      sp->ishift.y - sp->mousepos_o.y > npix)
  {
    *scale_y *= ((gfloat) (sp->mousepos.y - sp->ishift.y) /
                 (gfloat) (sp->mousepos_o.y - sp->ishift.y));
  }

  /* Restore if too small. */
  *scale_x = MAX (SCALE_MIN, *scale_x);
  *scale_y = MAX (SCALE_MIN, *scale_y);
}

