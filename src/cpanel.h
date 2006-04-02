/*-- cpanel.h --*/
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

#ifndef CPANEL_H
#define CPANEL_H

#include <gtk/gtk.h>
#include "defines.h"  /*-- and defines includes cpanel.h?  weird --*/


typedef struct _Tour2DCPanel {
   gboolean paused;
   gboolean local_scan;
   gboolean stepping;
   gboolean backtracking;
   gfloat step; 
   gint ls_dir;
   gfloat path_len;
   gint pp_indx;
   gfloat slidepos;
   gint manip_mode;

   TourPPIndex ppindex;
} Tour2DCPanel;

/*
 * This is at the display, or window, level.  Every display type
 * has one.  It captures the state of the control panel -- but
 * not the variable circles.
*/

typedef struct {

  ProjectionMode pmode;
  InteractionMode imode;

 /*-- 1d plotting --*/
 struct _P1DCpanel {
   gint type;
   gint nbins, nASHes;
   gboolean ASH_add_lines_p;
   /*-- cycling --*/
   gboolean cycle_p;
   gint cycle_dir;
   guint32 cycle_delay;
 } p1d;

 /*-- xy plotting --*/
 struct _XYPlotCpanel {
   /*-- cycling --*/
   gboolean cycle_p;
   gint cycle_axis;
   gint cycle_dir;
   guint32 cycle_delay;
 } xyplot;


 /*-- brushing --*/
  struct _BrushCpanel {
   gboolean updateAlways_p;
   gboolean brush_on_p;
   gint mode;
   gint linkby_row;
   BrushTargetType point_targets, edge_targets;
  } br;

 /*-- scaling --*/
  struct _ScaleCpanel {
    gboolean updateAlways_p;
    gboolean fixAspect_p;   /* fix aspect ratio or don't */
    struct {gdouble x, y;} zoomval;
    struct {gdouble x, y;} panval;
  } scale;

 /*-- identification --*/
 gint id_display_type;
 enum idtargetd id_target_type;  /* points or edges */

 /*-- edge editing -- including adding points --*/
 eeMode ee_mode;

 /*-- parallel coordinates --*/
 ParCoordsArrangeMode parcoords_arrangement;  /* arrange plots in a row or a column */

 /*-- time series --*/
/*XX*/
 gint tsplot_selection_mode;
 gint tsplot_arrangement;  /* arrange plots in a row or column*/

 /*-- barchart --*/
/*XX*/
 gint barchart_display_mode;

 /*-- 2d touring control pane --*/
 Tour2DCPanel t2d;

 /*-- rotation control pane --*/
 struct _Tour2D3CPanel {
   gboolean paused;
   gfloat step; 
   gfloat slidepos;
   gint manip_mode;
 } t2d3;

 /*-- 1d tour control panel --*/
 struct _Tour1DCpanel {
   gboolean paused;
   gfloat step; 
   gint nbins, nASHes;
   gboolean vert;
   gint pp_indx;
   gfloat slidepos;
   gfloat ASH_smooth;
   gfloat ASH_add_lines_p;
 } t1d;

 /*-- corr tour control panel --*/
 struct _TourCorrCpanel {s
   gfloat slidepos;
   gint manip_mode;
 } tcorr;
 struct _TourCorr1Cpanel {
   gboolean paused;
   gfloat step; 
 } tcorr1;
 struct _TourCorr2Cpanel {
   gboolean paused;
   gfloat step; 
 } tcorr2;

/* tour variables are in display.h */

} cpaneld;

#endif
