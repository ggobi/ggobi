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

/* for xg.raw_data, tform1, tform2 */
typedef struct {
  gfloat **data;
  gint nrows, ncols;
} array_f;
/* for xg.missing */
typedef struct {
  gshort **data;
  gint nrows, ncols;
} array_s;
/* for xg.missing */
typedef struct {
  glong **data;
  gint nrows, ncols;
} array_l;


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
  gchar name[32];
  gint glyphtype, glyphsize;
  gulong color;
  gint glyphtype_prev, glyphsize_prev;
  gulong color_prev;
  gboolean hidden, excluded;
} cluster;

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
  gboolean excluded;  /* will use this for linked brushing */
  gboolean hidden;  /* may not use this, but add it just in case */
} rg_struct;

#define XGOBI_TYPES_H
