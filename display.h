#ifndef DISPLAY_H
#define DISPLAY_H

#ifndef DEFINES_H
#include "defines.h"
#endif

#ifndef CPANEL_H
#include "cpanel.h"
#endif

#ifndef SPLOT_H
#include "splot.h"
#endif

typedef struct {

/*
 * Used by all displays
*/
 GtkWidget *window;
 cpaneld cpanel;
 enum displaytyped displaytype;
 GList *splots;  /* doubly linked list of splots */

/* Missing values */
 gboolean missing_p;  /* false by default */

/*
 * Actually, this is going to need to be a pair of vectors
 * or linked lists, corresponding to the number of plots in
 * the display.  But let's be lazy for the moment.
*/
 GtkWidget *hrule, *vrule;

/*
 * display options
*/

 gboolean points_show_p;              /* scatterplot, scatmat, parcoords */
 gboolean segments_directed_show_p;   /* scatterplot, scatmat */
 gboolean segments_undirected_show_p; /* scatterplot, scatmat */
 gboolean segments_show_p;            /* parcoords */
 gboolean missings_show_p;            /* scatterplot, scatmat, parcoords */

 gboolean gridlines_show_p;           /* scatterplot, scatmat, parcoords */
 gboolean axes_show_p;                /* scatterplot, scatmat, parcoords */
 gboolean axes_center_p;              /* scatterplot */

 gboolean double_buffer_p;            /* parcoords */

 gboolean link_p;                     /* scatterplot, scatmat, parcoords */


/*
 * For an individual scatterplot
*/


/*
 * Scatterplot matrix display
*/
 GList *scatmat_cols, *scatmat_rows;
 GtkWidget *table;


/*
 * Parallel coordinates display
*/
  gint p1d_orientation;

} displayd;


#endif
