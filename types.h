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
/* types.h */

typedef enum {ascii, binary, Sprocess, xml, mysql} DataMode;
typedef enum {PLUS=1, X, OR, FR, OC, FC, DOT,UNKNOWN_GLYPH} GlyphType;

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
typedef struct {
    gint x1, y1, x2, y2;
} brush_coords;

/*-- arrays --*/
/*-- floating point: for gg.raw_data, tform1, tform2 --*/
typedef struct {
  gfloat **vals;
  gint nrows, ncols;
} array_f;
/*-- short: for gg.missing --*/
typedef struct {
  gshort **vals;
  gint nrows, ncols;
} array_s;
/*-- long: for gg.world, jitdata --*/
typedef struct {
  glong **vals;
  gint nrows, ncols;
} array_l;

/*-- vectors --*/
typedef struct {
  gfloat *vals;
  glong nels;
} vector_f;
typedef struct {
  gint *vals;
  gint nels;
} vector_i;
typedef struct {
  gshort *vals;
  gint nels;
} vector_s;
typedef struct {
  gboolean *vals;
  gint nels;
} vector_b;


typedef struct {  /*-- used for obtaining ranks --*/
  gfloat f;
  gint indx;
} paird;

/* column-wise data that will appear in the variable table */
typedef struct {
 gint groupid_ori, groupid;
 gchar *collab, *collab_tform;
 gint nmissing;

 /*-- reference variable:  jref=-1 except for cloned variables --*/
 gint jref;

 /*-- unadjusted, unaffected by imputation --*/
 gfloat mean, median;

 /*
  * If the user has supplied limits, lim_specified_p = true
  * and the limits are stored in lim_specified.{min,max}
 */
 gboolean lim_specified_p;
 lims lim_specified;

 /*
  * lim contains the limits in use: the first use use min/max scaling
 */
 lims lim_raw;            /*-- range of the raw data          --*/
 lims lim_raw_gp;         /*-- range of the raw data, grouped --*/
 lims lim_tform;          /*-- range of tform2                --*/
 lims lim_tform_gp;       /*-- range of tform2, grouped       --*/
 lims lim;                /*-- limits in use, maybe not min/max  --*/

 gboolean selected;  /*-- temporary?  I'll use this for transformation --*/

 /*-- transformations --*/
 gint tform0;
 gfloat domain_incr;  /*-- stage 0 --*/
 gfloat (*domain_adj) (gfloat x, gfloat incr);
 gfloat (*inv_domain_adj) (gfloat x, gfloat incr);
 gint tform1;
 gfloat param;
 gint tform2;

 /*-- jittering --*/
 gfloat jitter_factor;

} vartabled;


/* these data types seem to be missing in gdk */
typedef struct {
  gint x, y;
  gint width, height;
} arcd; 
typedef struct {
  gint x, y;
  gint width, height;
} rectd; 

/* cluster; to be used in Group Exclusion tool */
typedef struct {
  glong n;  /*-- Can I know the number of elements in this cluster? --*/
  gint glyphtype, glyphsize;
  gshort color;
  gboolean hidden, included;
  GtkWidget *da, *lbl, *hide_tgl, *exclude_tgl;
} clusterd;

/* glyph vectors */
typedef struct {
  gint type;
  gint size;
} glyphv;

/* structure for binning the plot window */
typedef struct {
  gulong *els;
  gint nels;
  gint nblocks;  /* how many blocks have been allocated for els */
} bin_struct;

/* row groups */
typedef struct {
  gint id, nels, *els;
  gboolean included;  /* for linked brushing */
  gboolean sampled;   /* for subsetting */
} rgroupd;


/*-- used to keep track of history in moving points --*/
typedef struct {
  gint i, j;
  gfloat val;
} celld;

/*
 * display options
*/
typedef struct {
 gboolean points_show_p;              /* scatterplot, scatmat, parcoords */
 gboolean edges_directed_show_p;      /* scatterplot, scatmat */
 gboolean edges_undirected_show_p;    /* scatterplot, scatmat */
 gboolean edges_show_p;               /* parcoords */
 gboolean missings_show_p;            /* scatterplot, scatmat, parcoords */

 gboolean gridlines_show_p;           /* scatterplot, scatmat, parcoords */
 gboolean axes_show_p;                /* scatterplot, scatmat, parcoords */
 gboolean axes_center_p;              /* scatterplot */

 gboolean double_buffer_p;            /* parcoords */

 gboolean link_p;                     /* scatterplot, scatmat, parcoords */
} DisplayOptions;

extern DisplayOptions DefaultDisplayOptions;



#define TYPES_H

#endif
