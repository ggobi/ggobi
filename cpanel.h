#include <gtk/gtk.h>

#ifndef DEFINES_H
#include "defines.h"
#endif

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
 gint br_mode, br_scope, br_cg;

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
} cpaneld;

#define CPANEL_H
