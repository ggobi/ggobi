#ifndef DISPLAY_H
#define DISPLAY_H

#include "defines.h"
#include "cpanel.h"
#include "splot.h"
#include "datad.h"

#include "ggobi.h"

struct _ggobid; 
typedef struct _ggobid ggobid;

struct _displayd {

/*
 * Used by all displays
*/
 GtkWidget *window;
 cpaneld cpanel;
 enum displaytyped displaytype;

 struct _displayd *embeddedIn;

 GList *splots;  /* doubly linked list of splots */

 datad *d;  /*-- pointer to a particular gg->d[] --*/

/* Missing values */
 gboolean missing_p;  /* false by default */

/*
 * Actually, this is going to need to be a pair of vectors
 * or linked lists, corresponding to the number of plots in
 * the display.  But let's be lazy for the moment.
*/
 GtkWidget *hrule, *vrule;

 DisplayOptions options;


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

/*
 * Tour display
 */
  gint ntour_vars, *tour_vars;
  gfloat **u0, **u1, **u, **uold, **v0, **v1;
  gfloat *tau, *tinc, dv, delta;
  gfloat **tv;
  gfloat ts[2], coss[2], sins[2];
  gint icoss[2], isins[2];

/*
 * Correlation Tour
*/
  gint ncorrvars_x, *corrvars_x, ncorrvars_y, *corrvars_y;

  struct _ggobid *ggobi;

};  /* displayd; */


gboolean isEmbeddedDisplay(displayd *dpy);


#endif
