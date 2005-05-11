/* cpanel.c : control panel */
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
  cpanel_t2d3_init (cpanel, gg);
  cpanel_t2d_init (cpanel, gg);
  cpanel_t1d_init (cpanel, gg);
  cpanel_tcorr_init (cpanel, gg);

  cpanel_brush_init (cpanel, gg);
  cpanel_scale_init (cpanel, gg);
  cpanel_edgeedit_init (cpanel, gg);
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
parcoords_cpanel_init (cpaneld* cpanel, ggobid *gg) 
{
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
   displaytype_known = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(display))->cpanel_set(display, cpanel, gg);
  }

  if (cpanel->viewmode < COTOUR) cpanel->projection = cpanel->viewmode;

  if (displaytype_known)
    viewmode_set (cpanel->viewmode, gg);
}
