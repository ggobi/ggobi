/* scale_click.c: pan and zoom for the CLICK interaction style */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

void
pan_step (splotd *sp, gint pan_opt, ggobid *gg)
{
  if (pan_opt == P_OBLIQUE || pan_opt == P_HORIZ)  /* pan horizontally */
    sp->ishift.x += (sp->mousepos.x - sp->max.x/2);

  if (pan_opt == P_OBLIQUE || pan_opt == P_VERT)  /* pan vertically */
    sp->ishift.y += (sp->mousepos.y - sp->max.y/2);
}

void
zoom_step (splotd *sp, gint zoom_opt, gint in_or_out, rectd *rect, ggobid* gg)
{
  gint projection = projection_get (gg);
  gfloat *scale_x = (projection == TOUR2D) ? &sp->tour_scale.x : &sp->scale.x;
  gfloat *scale_y = (projection == TOUR2D) ? &sp->tour_scale.y : &sp->scale.y;
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
/*
 * Reset ishift in response to changes in scale -- in click-style
 * zooming, we are scaling out of the center of the window, 
 * {mid.x/2, mid.y/2}, not out of the center of the data.
*/
  if (*scale_x * scalefac_x >= SCALE_MIN) {
    sp->ishift.x = mid.x + (gint) (scalefac_x * (gfloat) (sp->ishift.x - mid.x));
    *scale_x = *scale_x * scalefac_x;
  }
  if (*scale_y * scalefac_y >= SCALE_MIN) {
    sp->ishift.y = mid.y + (gint) (scalefac_y * (gfloat) (sp->ishift.y - mid.y));
    *scale_y = *scale_y * scalefac_y;
  }
}

