/* scale-drag.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*
 * scale_style == DRAG and button 1 is pressed; we are panning.
 * The mouse has moved to gg.mousepos from gg.mousepos_o. 
 * Change shift_wrld appropriately
*/
void
pan_by_drag (splotd *sp, ggobid *gg)
{
  sp->ishift.x += (gg->mousepos.x - gg->mousepos_o.x);
  sp->ishift.y += (gg->mousepos.y - gg->mousepos_o.y);
}

/*
 * scale_style == DRAG and button 2 is pressed; we are zooming. 
 * The mouse has moved to gg->mousepos from gg->mousepos_o and the center
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
  if (gg->mousepos_o.x - sp->ishift.x > npix ||
      sp->ishift.x - gg->mousepos_o.x > npix)
  {
    *scale_x *= ((gfloat) (gg->mousepos.x - sp->ishift.x) /
                 (gfloat) (gg->mousepos_o.x - sp->ishift.x));
  }

  if (gg->mousepos_o.y - sp->ishift.y > npix ||
      sp->ishift.y - gg->mousepos_o.y > npix)
  {
    *scale_y *= ((gfloat) (gg->mousepos.y - sp->ishift.y) /
                 (gfloat) (gg->mousepos_o.y - sp->ishift.y));
  }

  /* Restore if too small. */
  *scale_x = MAX (SCALE_MIN, *scale_x);
  *scale_y = MAX (SCALE_MIN, *scale_y);
}

