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

 gint mode, prev_mode;
 gint projection;

/*
 * 1d plotting
*/
 gint p1d_type;
 gint nbins, nASHes;

/*
 * rotation
*/
 gint ro_type, ro_axis, ro_direction;
 gboolean ro_paused_p;

/*
 * brushing
*/
 gboolean brush_on_p;
 gint br_mode, br_scope, br_target;

/*
 * scaling
*/
  gint scale_style;       /* DRAG or CLICK */
  gint scale_click_opt;   /* PAN or ZOOM */
  gint scale_pan_opt;     /* P_OBLIQUE, P_HORIZ, P_VERT */
  gint scale_zoom_opt;    /* Z_OBLIQUE, Z_ASPECT, Z_HORIZ, Z_VERT */

/*
 * line editing
*/

/*
 * parallel coordinates
*/
 gint parcoords_selection_mode;
 gint parcoords_arrangement;  /* arrange plots in a row or a column */

/*
 * time series
*/

 gint tsplot_selection_mode;
 gint tsplot_arrangement;  /* arrange plots in a row or column*/

/*
 * scatterplot matrix
*/
 gint scatmat_selection_mode;

/*
 * 2d touring
*/
 gboolean t2d_paused;
 gboolean t2d_local_scan;
 gboolean t2d_stepping;
 gboolean t2d_backtracking;
 gfloat t2d_step; 
 gint t2d_ls_dir;
 gfloat t2d_path_len;

/*
 * 1d tour
*/
 gboolean t1d_paused;
 gfloat t1d_step; 
 gint t1d_nbins, t1d_nASHes;
 gboolean t1d_vert;
 gint t1d_pp_indx;
 gfloat t1d_pp_indxval;

/*
 * corr tour
*/
 gboolean tcorr1_paused;
 gfloat tcorr1_step; 
 gboolean tcorr2_paused;
 gfloat tcorr2_step; 

/* tour variables are in display.h */

} cpaneld;

#endif
