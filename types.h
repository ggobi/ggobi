/*-- types.h --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#ifndef TYPES_H
#define TYPES_H

#include <gtk/gtk.h>

typedef enum {ascii_data, binary_data, Sprocess_data, xml_data, mysql_data, unknown_data, num_data_modes} DataMode;

typedef struct {
    glong x, y;
} lcoords;
typedef struct {
    gint x, y;
} icoords;
typedef struct {
    gfloat x, y;
} fcoords;
typedef struct {
    gfloat min, max;
} lims;
typedef struct {
    gint a, b;
} endpointsd;

/*-- arrays --*/
/*-- floating point: for gg.raw_data, tform1, tform2 --*/
typedef struct {
  gfloat **vals;
  guint nrows, ncols;
} array_f;
/*-- short: for gg.missing --*/
typedef struct {
  gshort **vals;
  guint nrows, ncols;
} array_s;
/*-- long: for gg.world, jitdata --*/
typedef struct {
  glong **vals;
  guint nrows, ncols;
} array_l;

/*-- vectors --*/
typedef struct {
  gfloat *els;
  guint nels;
} vector_f;
typedef struct {
  gint *els;
  guint nels;
} vector_i;
typedef struct {
  gshort *els;
  guint nels;
} vector_s;
typedef struct {
  gboolean *els;
  guint nels;
} vector_b;

typedef struct {  /*-- used for obtaining ranks --*/
  gfloat f;
  gint indx;
} paird;



/*-- used to keep track of history in moving points --*/
typedef struct {
  gint i, j;
  gfloat val;
} celld;

/*-- tour elements --*/
typedef struct {
  gint nvars;
  vector_i vars;
  array_f u0, u1, u, uold, v0, v1, v, uvevec, tv;
  vector_f lambda, tau, tinc;
  gfloat dv, delta;
  gint idled;
  gboolean get_new_target;
  gint nsteps, stepcntr;
} tour;

/*
 * display options
*/
typedef struct {
 gboolean points_show_p;              /* scatterplot, scatmat, parcoords */
 gboolean edges_directed_show_p;      /* scatterplot, scatmat */
 gboolean edges_undirected_show_p;    /* scatterplot, scatmat */
 gboolean edges_show_p;               /* parcoords */
 gboolean missings_show_p;            /* scatterplot, scatmat, parcoords */

 gboolean axes_show_p;                /* scatterplot, scatmat, parcoords */
 gboolean axes_center_p;              /* scatterplot */

 gboolean double_buffer_p;            /* parcoords */

 gboolean link_p;                     /* scatterplot, scatmat, parcoords */
} DisplayOptions;

extern DisplayOptions DefaultDisplayOptions;



#define TYPES_H

#endif
