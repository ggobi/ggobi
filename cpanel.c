/* cpanel.c : control panel */
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
scatterplot_cpanel_init (cpaneld *cpanel, PipelineMode initial_mode,
  ggobid *gg)
{
  cpanel->viewmode = initial_mode;

  /*-- scatterplot only, so far --*/
  cpanel->projection = initial_mode;

  /*-- 1d plots --*/
  cpanel->p1d.type = ASH;
  cpanel_p1d_init (cpanel, gg);
  cpanel_xyplot_init (cpanel, gg);
#ifdef ROTATION_IMPLEMENTED
  cpanel_t2d3_init (cpanel, gg);
#endif
  cpanel_t2d_init (cpanel, gg);
  cpanel_t1d_init (cpanel, gg);
  cpanel_tcorr_init (cpanel, gg);

  cpanel_brush_init (cpanel, gg);
  cpanel_scale_init (cpanel, gg);
#ifdef EDIT_EDGES_IMPLEMENTED
  cpanel_edgeedit_init (cpanel, gg);
#endif
  cpanel_identify_init (cpanel, gg);
}

void
scatmat_cpanel_init (cpaneld* cpanel, ggobid *gg) {
  cpanel->viewmode = SCATMAT;
  cpanel->projection = XYPLOT;  /*-- does it need a projection? --*/

  cpanel->scatmat_selection_mode = VAR_REPLACE;

  /*-- 1d plots --*/
  cpanel->p1d.type = ASH;
  cpanel_p1d_init (cpanel, gg);

  /*-- available modes --*/
  cpanel_brush_init (cpanel, gg);
  cpanel_identify_init (cpanel, gg);
}

void
parcoords_cpanel_init (cpaneld* cpanel, ggobid *gg) {
  cpanel->viewmode = PCPLOT;
  cpanel->projection = P1PLOT;  /*-- does it need a projection? --*/

  /*-- 1d plots --*/
  cpanel->p1d.type = DOTPLOT;
  cpanel_p1d_init (cpanel, gg);

  cpanel->parcoords_selection_mode = VAR_REPLACE;
  cpanel->parcoords_arrangement = ARRANGE_ROW;

  /*-- available modes --*/
  cpanel_brush_init (cpanel, gg);
  cpanel_identify_init (cpanel, gg);
}


void
cpanel_set (displayd *display, ggobid *gg)
{
  cpaneld *cpanel = &display->cpanel;
  gboolean displaytype_known = true;

  if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
   displaytype_known = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass)->cpanel_set(display, cpanel, gg);
  }

  if (cpanel->viewmode < COTOUR) cpanel->projection = cpanel->viewmode;

  if (displaytype_known)
    viewmode_set (cpanel->viewmode, gg);
}
