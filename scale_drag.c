/* scale-drag.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*
 * scale_style == DRAG and button 1 is pressed; we are panning.
 * The mouse has moved to mousepos from mousepos_o. 
 * Change shift_wrld appropriately
*/
void
pan_by_drag (splotd *sp)
{
  sp->ishift.x += (mousepos.x - mousepos_o.x);
  sp->ishift.y += (mousepos.y - mousepos_o.y);
}

/*
 * scale_style == DRAG and button 2 is pressed; we are zooming. 
 * The mouse has moved to mousepos from mousepos_o and the center
 * of the figure is at sp->mid.  Change sp->scale by the
 * appropriate amounts.
*/
void
zoom_by_drag (splotd *sp)
{
  gint projection = projection_get ();
  gfloat *scale_x = (projection == GRTOUR) ? &sp->tour_scale.x : &sp->scale.x;
  gfloat *scale_y = (projection == GRTOUR) ? &sp->tour_scale.y : &sp->scale.y;
  gint npix = 10;  /*-- number of pixels from the crosshair required --*/

  /*-- Scale the scaler if far enough from center --*/
  if (mousepos_o.x - sp->ishift.x > npix || sp->ishift.x - mousepos_o.x > npix)
    *scale_x *= ((gfloat) (mousepos.x - sp->ishift.x) /
                 (gfloat) (mousepos_o.x - sp->ishift.x));

  if (mousepos_o.y - sp->ishift.y > npix || sp->ishift.y - mousepos_o.y > npix)
    *scale_y *= ((gfloat) (mousepos.y - sp->ishift.y) /
                 (gfloat) (mousepos_o.y - sp->ishift.y));

  /* Restore if too small. */
  *scale_x = MAX (SCALE_MIN, *scale_x);
  *scale_y = MAX (SCALE_MIN, *scale_y);
}

