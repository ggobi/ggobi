/*-- cpanel.h --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#ifndef CPANEL_H
#define CPANEL_H

#include <gtk/gtk.h>
#include "defines.h"  /*-- and defines includes cpanel.h?  weird --*/

/*
 * This is at the display, or window, level.  Every display type
 * has one.  It captures the state of the control panel -- but
 * not the variable circles.
*/

typedef struct {

 PipelineMode viewmode, prev_viewmode;
 PipelineMode projection;

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
 gboolean brush_on_p;
 gint br_mode;
 gint br_linkby;
 BrushTargetType br_point_targets, br_edge_targets;

 /*-- scaling --*/
 gint scale_style;       /* DRAG or CLICK */
 gint scale_click_opt;   /* PAN or ZOOM */
 gboolean scale_drag_aspect_p;   /* fix aspect ratio or don't */
 gint scale_pan_opt;     /* P_OBLIQUE, P_HORIZ, P_VERT */
 gint scale_zoom_opt;    /* Z_OBLIQUE, Z_ASPECT, Z_HORIZ, Z_VERT */

 /*-- identification --*/
 gint id_display_type;
 enum idtargetd id_target_type;  /* points or edges */

 /*-- edge editing -- including adding points --*/
 gboolean ee_adding_edges_p;
 gboolean ee_deleting_edges_p;  /*??*/
 gboolean ee_adding_points_p;

 /*-- parallel coordinates --*/
 gint parcoords_selection_mode;
 gint parcoords_arrangement;  /* arrange plots in a row or a column */

 /*-- time series --*/
/*XX*/
 gint tsplot_selection_mode;
 gint tsplot_arrangement;  /* arrange plots in a row or column*/

 /*-- barchart --*/
/*XX*/
 gint barchart_display_mode;


 /*-- scatterplot matrix --*/
 gint scatmat_selection_mode;

 /*-- 2d touring control pane --*/
 struct _Tour2DCPanel {
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
 } t2d;

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
 struct _TourCorrCpanel {
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
