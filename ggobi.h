#ifndef GGOBI_H
#define GGOBI_H

#include "defines.h"
#include "types.h"
#include "display.h"
#include "display_tree.h"

typedef enum {ascii, Sprocess, binary, xml} DataMode;
typedef enum {read_all, read_block, draw_sample} FileReadType;

struct _ggobid;

typedef struct {

 struct _ggobid *thisGG;

  /* main_ui */
  GtkWidget *menubar;
  GtkAccelGroup *main_accel_group;

  /* brush_ui */
  GtkWidget *brush_reset_menu;
  GtkWidget *brush_link_menu;

  /* brush */
  brush_coords brush_pos ;  

  /* color_ui.c */
  gint spacing;

  /* identify_ui.c */
  GtkWidget *identify_link_menu;
  icoords cursor_pos;

  /* texture */
  gfloat *gy;

  /* tour2d_ui */
  GtkWidget *tour2d_io_menu;

  /* parcoords_ui */
  GtkWidget *parcoords_mode_menu;

  /* scatmat_ui */
  GtkWidget *scatmat_mode_menu;

  /* scatterplot_ui */
  GtkWidget *scatterplot_mode_menu;

  /* display_tree */
  DisplayTree display_tree;

  /* rotate_ui */
  GtkWidget *rotation_io_menu;

  /* scale_ui.c */
  GtkWidget *scale_reset_menu;

} GGobiApp;

typedef struct _ggobid {

 gchar *data_in;

 GdkGC *plot_GC;
 GdkGC *selvarfg_GC, *selvarbg_GC;     /* white background, thick lines */
 GdkGC *unselvarfg_GC, *unselvarbg_GC; /* grey background, thin lines */

 GList *displays;
 displayd *current_display;
 splotd *current_splot; 

 icoords mousepos, mousepos_o;
 gboolean mono_p;

 GtkWidget *control_panel[NMODES];

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
 /*-- used in reading in associated row-wise files --*/
 glong *file_rows_sampled; /* of length file_sample_size */

 gint ncols, nrows;

 array_f raw, tform1, tform2;
 array_l world, jitter;

 /*-- Scaling --*/
 fcoords scale0, tour_scale0;

/* Missing values */
 gint nmissing;
 array_s missing;  /*-- array of shorts --*/
 array_l missing_jitter, missing_world;
 gfloat missing_jitter_factor;
 lims missing_lim;  /*-- usually 0,1, but potentially wider --*/
/* */

 /* Deleting the hidden points; subsetting */
 gboolean delete_hidden_pts;
 gint *rows_in_plot;
 gint nrows_in_plot;
 gboolean *sampled;

 /* Row grouping */

 glong nrgroups, nrgroups_in_plot;
 glong *rgroup_ids;
 rgroupd *rgroups;

 /* Hiding/excluding the hidden points */
 clusterd *clusv;
 gint nclust;
 vector_i clusterid;
 gboolean *included;


/* Line groups */
 glong nlgroups;
 glong *lgroup_ids;
 rgroupd *lgroups;  /* id, nels, *els */

/* standardization options */
 gint std_type;  /* Can be 0, 1 or 2 */

/* for connecting points with segments */
 gint nsegments;
 endpointsd *segment_endpoints;

 vardatad *vardata;

/* row labels */
 gchar **rowlab;

/* gboolean: does this data contain only one variable? False by default */
 gboolean single_column;

/*
 * brushing
*/
 gint ncolors;
 GdkColor *default_color_table, *color_table;  /* brushing colors */
 GdkColor bg_color;          /* background color */
 GdkColor accent_color;      /* color for axes and labels */
 gushort color_id, color_0;  /* 0:ncolors-1 */
 /* point brushing */
 gboolean *under_new_brush;
 gushort *color_ids, *color_now, *color_prev;  /* 0:ncolors-1 */
 glyphv glyph_id, glyph_0;
 glyphv *glyph_ids, *glyph_now, *glyph_prev;
 gboolean *hidden, *hidden_now, *hidden_prev;
 /* line brushing */
 gushort *xed_by_new_brush;
 gushort *line_color_ids, *line_color_now, *line_color_prev;  /* 0:ncolors-1 */
 /* binning */
 gint br_nbins;
 bin_struct **br_binarray;
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

 GGobiApp app;

} ggobid;


GGOBI_ ggobid gg;

#define GGOBI_H

#endif

