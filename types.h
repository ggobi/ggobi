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

typedef enum {ascii_data, binary_data, Sprocess_data, xml_data, mysql_data, url_data, unknown_data, num_data_modes} DataMode;

typedef enum {NONE, EXPOSE, QUICK, BINNED, FULL, FULL_1PIXMAP} RedrawStyle;


typedef struct {
  gint type, size;
} glyphd;

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
typedef struct {
  glyphd *els;
  guint nels;
} vector_g;

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
  gint target_basis_method;
  gint index; /* this is for counting planes passed */
  gfloat ppval; /* for projection pursuit */
} tour;

/* Sigbert's code for pp */
typedef struct {
  gint min_neighbour, max_neighbour, dim, data_step, neighbour_step;
  /* temporary space */
  gfloat *dist, *mean, *nmean, *fv1, *fv2, *ew, *ev, *cov;
  gint *index;
} subd_param;

typedef struct {
  gint *group, *ngroup, groups;
  /* temporary space */
  gfloat *cov, *mean, *ovmean, *a, *work;
  gint *kpvt;

} discriminant_param;

typedef struct {
  gint *ngroup, *group, groups;

  /* temporary space */
  gint *nright, *index;
  gfloat *x;

} cartgini_param;

typedef struct {
  gint *ngroup, *group, groups;

  /* temporary space */
  gint *nright, *index;
  gfloat *x;

} cartentropy_param;

typedef struct {
  gfloat *y;

  /* temporary space */
  gfloat *x;
  gint *index;

} cartvariance_param;

typedef struct
{ gfloat temp_start, temp_end, cooling, heating, temp, index_best;
  gint restart, maxproj, success;
  array_f proj_best, data;
} optimize0_param; 
/* end Sigbert's code */

/* pp */
typedef struct
{
  gint nrows, ncols;
  gfloat *h0, *h1;
  gfloat acoefs;
  gfloat **derivs;
  gfloat *alpha, *beta;
} holes_param;
/* end pp */

/*
 * display options
*/
typedef struct {
 gboolean points_show_p;              /* scatterplot, scatmat, parcoords */
 
 /*-- two options here may be plenty --*/
 gboolean edges_undirected_show_p;    /* scatterplot */
 gboolean edges_arrowheads_show_p;    /* scatterplot */
 gboolean edges_directed_show_p;      /* scatterplot: both of the above */
 /*-- --*/

 gboolean whiskers_show_p;            /* parcoords, time series */

 gboolean missings_show_p;            /* scatterplot, scatmat, parcoords */

 gboolean axes_show_p;                /* scatterplot, scatmat, parcoords */
 gboolean axes_center_p;              /* scatterplot */

 gboolean double_buffer_p;            /* parcoords */

 gboolean link_p;                     /* scatterplot, scatmat, parcoords */
} DisplayOptions;

extern DisplayOptions DefaultDisplayOptions;



#define TYPES_H

#endif
