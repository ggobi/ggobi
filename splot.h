#ifndef SPLOT_H
#define SPLOT_H

#include "defines.h"
typedef struct _displayd displayd;

typedef struct {

 displayd *displayptr;  /* a pointer to the enclosing display */

 GtkWidget *da;                 /* drawing_area */
 GdkPixmap *pixmap0, *pixmap1;  /* 2-stage drawing */

 gint redraw_style;

 /* Drawing area dimensions */
 icoords max;

 /*
  * line segments in scatterplot and scatmat
 */
 GdkSegment *arrowheads;
 GdkSegment *edges;

 /*
  * line segments in parallel coordinates plot
 */
 GdkSegment *whiskers;

 lcoords *planar;
 icoords *screen;

 /*
  * shift and scale
 */
 fcoords scale, tour_scale;
 lcoords iscale;
 icoords ishift;

/*
 * button and key event information
*/
 gint motion_id, press_id, release_id, key_press_id;

/*
 * plot1d  (used in parcoords as well as scatterplot)
*/
 gint p1dvar;
 gfloat *p1d_data;  /* the spreading data */
 lims p1d_lim;      /* limits of the spreading data */

/*
 * xyplot
*/
 icoords xyvars;

/*
 * rotation
*/
 struct {gint x, y, z;} spinvars;

/*
 * tour
*/

/*
 * correlation tour
*/

/*
 * brushing
*/

} splotd;

#endif
