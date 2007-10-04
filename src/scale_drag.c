/* scale_drag.c */
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
#include <math.h>
#include "vars.h"
#include "externs.h"

/*
 * scale_style == DRAG and button 1 is pressed; we are panning.
 * The mouse has moved to sp.mousepos from sp.mousepos_o. 
 * Change shift_wrld appropriately
*/
void
pan_by_drag (splotd * sp, GGobiSession * gg)
{
  gdouble dx, dy;
  gdouble scale_x, scale_y;

  dx = (gdouble) (sp->mousepos.x - sp->mousepos_o.x);
  dy = (gdouble) (sp->mousepos.y - sp->mousepos_o.y);

  scale_x = (gdouble) sp->scale.x;
  scale_y = (gdouble) sp->scale.y;

/*
 * This section is a bit puzzling, because I don't know what
 * would change this -- maybe resizing the plot window?
*/
  scale_x /= 2;
  sp->iscale.x = (gdouble) sp->max.x * scale_x;
  scale_y /= 2;
  sp->iscale.y = -1 * (gdouble) sp->max.y * scale_y;

  sp->pmid.x -= (dx / sp->iscale.x);
  sp->pmid.y -= (dy / sp->iscale.y);
}

/*
 * scale_style == DRAG and button 2 (or 3) is pressed; we are zooming. 
 * The mouse has moved to sp->mousepos from sp->mousepos_o and the center
 * of the figure is at sp->mid.  Change sp->scale by the
 * appropriate amounts.
*/
void
zoom_by_drag (splotd * sp, GGobiSession * gg)
{
  gdouble *scale_x = &sp->scale.x;
  gdouble *scale_y = &sp->scale.y;
  gint npix = 20;  /*-- number of pixels from the crosshair required --*/
  displayd *dsp = sp->displayptr;
  cpaneld *cpanel = &dsp->cpanel;

  icoords mid;
  fcoords scalefac;

  mid.x = sp->max.x / 2;
  mid.y = sp->max.y / 2;
  scalefac.x = scalefac.y = 1.0;

  if ((ABS (sp->mousepos.x - mid.x) >= npix) &&
      (ABS (sp->mousepos.y - mid.y) >= npix)) {
    /*-- making the behavior identical to click zooming --*/
    scalefac.x =
      (gdouble) (sp->mousepos.x - mid.x) / (gdouble) (sp->mousepos_o.x - mid.x);
    scalefac.y =
      (gdouble) (sp->mousepos.y - mid.y) / (gdouble) (sp->mousepos_o.y - mid.y);


    if (cpanel->scale.fixAspect_p) {
      gdouble fac = MAX (scalefac.x, scalefac.y);
      *scale_x = *scale_x * fac;
      *scale_y = *scale_y * fac;

    }
    else {
      if (*scale_x * scalefac.x >= SCALE_MIN)
        *scale_x = *scale_x * scalefac.x;
      if (*scale_y * scalefac.y >= SCALE_MIN)
        *scale_y = *scale_y * scalefac.y;
    }
  }
}
