/* cpanel.c : control panel */

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

void
scatterplot_cpanel_init (cpaneld *cpanel, gint initial_mode) {
  cpanel->mode = initial_mode;

  /*-- scatterplot only, so far --*/
  cpanel->projection = initial_mode;

  /*-- 1d plots --*/
  cpanel->p1d_type = TEXTURE;
  cpanel_p1d_init (cpanel);

  cpanel_rotation_init (cpanel);
  cpanel_brush_init (cpanel);
  cpanel_tour_init (cpanel);  /*-- tour_init, or tour2d_init? --*/
}

void
scatmat_cpanel_init (cpaneld* cpanel) {
  cpanel->mode = SCATMAT;
  cpanel->projection = XYPLOT;  /*-- does it need a projection? --*/

  /*-- 1d plots --*/
  cpanel->p1d_type = ASH;
  cpanel_p1d_init (cpanel);

  cpanel_brush_init (cpanel);
}

void
parcoords_cpanel_init (cpaneld* cpanel) {
  cpanel->mode = PCPLOT;
  cpanel->projection = P1PLOT;  /*-- does it need a projection? --*/

  /*-- 1d plots --*/
  cpanel->p1d_type = DOTPLOT;
  cpanel_p1d_init (cpanel);

  cpanel->parcoords_selection_mode = VAR_REPLACE;
  cpanel->parcoords_arrangement = ARRANGE_ROW;

  cpanel_brush_init (cpanel);
}

void
cpanel_set (displayd *display, ggobid *gg) {
  cpaneld *cpanel = &display->cpanel;

  switch (display->displaytype) {
    case scatterplot:
      cpanel_p1d_set (cpanel, gg);
      cpanel_rotation_set (cpanel);
      cpanel_brush_set (cpanel);
      break;
    case scatmat:
      cpanel_brush_set (cpanel);
      break;
    case parcoords:
      /*cpanel_p1d_set (cpanel);  write cpanel_parcoords_set (cpanel) */
      cpanel_brush_set (cpanel);
      break;
  }

  mode_set (cpanel->mode, gg);
}
