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
 * scatterplot matrix
*/
 gint scatmat_selection_mode;

/*
 * touring
*/
 gboolean tour_paused_p;
 gboolean tour_local_scan_p;
 gboolean tour_stepping_p;
 gboolean tour_backtracking_p;
 gfloat tour_step; 
 gint tour_ls_dir;
 gfloat tour_path_len;

} cpaneld;

#endif
