#ifndef TYPES_H
#define TYPES_H

#include <gtk/gtk.h>
/* types.h */

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

/* for gg.raw_data, tform1, tform2 */
typedef struct {
  gfloat **data;
  gint nrows, ncols;
} array_f;
/* for gg.missing */
typedef struct {
  gshort **data;
  gint nrows, ncols;
} array_s;
/* for gg.missing */
typedef struct {
  glong **data;
  gint nrows, ncols;
} array_l;

/*-- vectors --*/
typedef struct {
  gfloat *data;
  gint nels;
} vector_f;
/*-- vectors --*/
typedef struct {
  gint *data;
  gint nels;
} vector_i;


typedef struct {  /*-- used for obtaining ranks --*/
  gfloat f;
  gint indx;
} paird;

/* column-wise data that will appear in the variable table */
typedef struct {
 gint groupid_ori, groupid;
 gchar *collab, *collab_tform;
 gint nmissing;

 /*-- unadjusted, unaffected by vgroups or imputation --*/
 gfloat mean, median;

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
  gfloat (*domain_adj) (gfloat x);
  gfloat (*inv_domain_adj) (gfloat x);
  gint tform1;
  gfloat param;
  gint tform2;

  /*-- jittering --*/
  gfloat jitter_factor;

 /*-- and doubtless more, as we go along --*/

} vardatad;


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
  glong id;
  gulong *els;
  glong nels;
  gboolean included;   /* for linked brushing */
  gboolean sampled;  /* for subsetting */
} rgroupd;




/*
 * display options
*/
typedef struct {
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
} DisplayOptions;

extern DisplayOptions DefaultDisplayOptions;

#define TYPES_H


#endif
