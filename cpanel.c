/* cpanel.c : control panel */

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

void
scatterplot_cpanel_init (cpaneld *cpanel, gint initial_mode, ggobid *gg) {
  cpanel->mode = initial_mode;

  /*-- scatterplot only, so far --*/
  cpanel->projection = initial_mode;

  /*-- 1d plots --*/
  cpanel->p1d_type = TEXTURE;
  cpanel_p1d_init (cpanel, gg);

  cpanel_rotation_init (cpanel, gg);
  cpanel_brush_init (cpanel, gg);
  cpanel_tour_init (cpanel, gg);  /*-- tour_init, or tour2d_init? --*/
}

void
scatmat_cpanel_init (cpaneld* cpanel, ggobid *gg) {
  cpanel->mode = SCATMAT;
  cpanel->projection = XYPLOT;  /*-- does it need a projection? --*/

  /*-- testing; dfs --*/
  cpanel->parcoords_selection_mode = VAR_REPLACE;

  /*-- 1d plots --*/
  cpanel->p1d_type = ASH;
  cpanel_p1d_init (cpanel, gg);

  cpanel_brush_init (cpanel, gg);
}

void
parcoords_cpanel_init (cpaneld* cpanel, ggobid *gg) {
  cpanel->mode = PCPLOT;
  cpanel->projection = P1PLOT;  /*-- does it need a projection? --*/

  /*-- 1d plots --*/
  cpanel->p1d_type = DOTPLOT;
  cpanel_p1d_init (cpanel, gg);

  cpanel->parcoords_selection_mode = VAR_REPLACE;
  cpanel->parcoords_arrangement = ARRANGE_ROW;

  cpanel_brush_init (cpanel, gg);
}

void
cpanel_set (displayd *display, ggobid *gg) {
  cpaneld *cpanel = &display->cpanel;

  switch (display->displaytype) {
    case scatterplot:
      cpanel_p1d_set (cpanel, gg);
      cpanel_rotation_set (cpanel, gg);
      cpanel_brush_set (cpanel, gg);
      break;
    case scatmat:
      cpanel_brush_set (cpanel, gg);
      break;
    case parcoords:
      /*cpanel_p1d_set (cpanel);  write cpanel_parcoords_set (cpanel) */
      cpanel_brush_set (cpanel, gg);
      break;
  }

  mode_set (cpanel->mode, gg);
}
