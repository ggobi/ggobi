#ifndef DEFINES_H
#include "defines.h"
#endif

typedef enum {ascii, Sprocess, binary} DataMode;
typedef enum {read_all, read_block, draw_sample} FileReadType;

typedef struct {

/************************** Data variables *************************/

 DataMode data_mode;

 gchar *filename;      /* full file name, including path */
 gchar *fname;         /* file name without suffix: .dat, .missing */

 /*
  * Do we read in the entire file, or do we only read in some
  * block or sample of cases?
 */
 FileReadType file_read_type;
 glong file_start_row;     /* needed for block type */
 glong file_length;        /* needed for sample */
 glong file_sample_size;   /* needed for both */
 /*
  * To be used in reading in associated row-wise files
 */
 glong *file_rows_sampled; /* of length file_sample_size */


 array_f raw, tform1, tform2;
 array_l world, jitter;

 fcoords scale0;
 fcoords tour_scale0;

/* Missing values */
 gint nmissing;
 array_s missing;  /*-- array of shorts --*/
 array_l missing_jitter, missing_world;
 gfloat missing_jitter_factor;
 lims missing_lim;  /*-- usually 0,1, but potentially wider --*/
/* */

 gint ncols, nrows;

 /* Deleting the erased points; subsetting */
 gboolean delete_erased_pts;
 gint *rows_in_plot;
 gint nrows_in_plot;


/* Row grouping */
 glong nrgroups, nrgroups_in_plot;
 glong *rgroup_ids;
 rg_struct *rgroups;


 /* Deleting the erased points -- this notion will change to hidden/excluded */
 cluster *clusv;
 int nclust;
 gboolean *erased;
 gboolean *excluded; /* it's too slow to get this from rows_in_plot */

 gulong *senddata;
 gint nlinkable, nlinkable_in_plot;  /* used in sizing senddata */;

/* Line grouping */
 glong nlgroups;
 glong *lgroup_ids;
 rg_struct *lgroups;  /* id, nels, *els */

/* standardization options */
 gint std_type;  /* Can be 0, 1 or 2 */

/* for connecting points with segments */
 gint nsegments;
 endpointsd *segment_endpoints;


 /*
  * Vectors of min and max values - including current fixed or imputed data
  * lim_raw contains the min_max limits -- for the raw_data!
  * lim_tform contains the limits of the tform_data -- used in
  *   parallel coordinates in case the main window is displaying
  *   sphered data
  * lim contains the limits in use
 */

 vardatad *vardata;


/* row labels */
 gchar **rowlab;

/* gboolean: does this data contain only one variable? False by default */
 gboolean single_column;

/* link to S */
 gchar *Spath;

/*
 * brushing
*/
 glyphv glyph_id, glyph_0;
 glyphv *glyph_ids, *glyph_now, *glyph_prev;

 gint ncolors;
 GdkColor *default_color_table, *color_table;  /* brushing colors */
 GdkColor bg_color;      /* background color */
 GdkColor accent_color;  /* color for axes and labels */
 gushort color_id, color_0;  /* 0:ncolors-1 */
 /* point brushing */
 gboolean *under_new_brush;
 gushort *color_ids, *color_now, *color_prev;  /* 0:ncolors-1 */
 /* line brushing */
 gushort *xed_by_new_brush;
 gushort *line_color_ids, *line_color_now, *line_color_prev;  /* 0:ncolors-1 */

 /* binning */
 gint br_nbins;
 bin_struct **br_binarray;
 /*
  * These are initialized so that the first merge_brushbins()
  * call will behave reasonably.
 */
 icoords bin0, bin1;

 /*
  * jittering
 */
 gfloat jitter_factor;
 gboolean jitter_type;
 gboolean jitter_vgroup;
 gboolean jitter_convex;
 gfloat *jitfacv;

 /*
  * identification
 */
 GSList *sticky_ids;

 gint ntourvars;
 gint *tourvars;

 

/************************** Display variables *************************/

  GtkTooltips *tips;

/*
 * scaling
*/
 gint pan_or_zoom;


} xgobid;

#define XGOBI_H


