#ifndef GGOBI_H
#define GGOBI_H


#ifndef DEFINES_H
#include "defines.h"
#endif

#include "types.h"
#include "display.h"
#include "display_tree.h"

typedef enum {ascii, Sprocess, binary} DataMode;
typedef enum {read_all, read_block, draw_sample} FileReadType;


struct _xgobid;

typedef struct {

 gchar *_data_in;
 GtkWidget *_control_panel[NMODES];

 GdkGC *_plot_GC;
 GdkGC *_selvarfg_GC, *_selvarbg_GC;     /* white background, thick lines */
 GdkGC *_unselvarfg_GC, *_unselvarbg_GC; /* grey background, thin lines */

 struct _xgobid *_thisXg;
 GList *_displays;
 displayd *_current_display;

 /* The splot which has the mouse and keyboard focus */
 splotd *_current_splot; 

 icoords _mousepos, _mousepos_o;

 gboolean _mono_p;




  /* main_ui */
  GtkWidget *_menubar;
  GtkAccelGroup *_main_accel_group;

  /* brush_ui */
  GtkWidget *_brush_reset_menu;
  GtkWidget *_brush_link_menu;

  /* brush */
  brush_coords _brush_pos ;  

  /* color_ui.c */
  gint _spacing;

  /* identify_ui.c */
  GtkWidget *_identify_link_menu;
  icoords _cursor_pos;

  /* texture */
  gfloat *_gy;

  /* tour2d_ui */
  GtkWidget *_tour2d_io_menu;

  /* parcoords_ui */
  GtkWidget *_parcoords_mode_menu;

  /* scatmat_ui */
  GtkWidget *_scatmat_mode_menu;

  /* scatterplot_ui */
  GtkWidget *_scatterplot_mode_menu;

  /* display_tree */
  DisplayTree display_tree;

  /* rotate_ui */
  GtkWidget *_rotation_io_menu;

  /* scale_ui.c */
  GtkWidget *_scale_reset_menu;

} XGobiApp;



typedef struct _xgobid {

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
 gboolean *in_subset;


/* Row grouping */
 glong nrgroups, nrgroups_in_plot;
 glong *rgroup_ids;
 rgroupd *rgroups;


 /* Hiding/excluding the erased points */
 cluster *clusv;
 int nclust;
 gboolean *included; /* it's too slow to get this from rows_in_plot */

 gulong *senddata;
 gint nlinkable, nlinkable_in_plot;  /* used in sizing senddata */;

/* Line grouping */
 glong nlgroups;
 glong *lgroup_ids;
 rgroupd *lgroups;  /* id, nels, *els */

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
 gboolean *erased, *erased_now, *erased_prev;
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


 XGobiApp app;

} xgobid;





#define data_in xg.app._data_in
#define control_panel xg.app._control_panel
#define plot_GC xg.app._plot_GC
#define selvarfg_GC xg.app._selvarfg_GC
#define selvarbg_GC xg.app._selvarbg_GC
#define unselvarfg_GC xg.app._unselvarfg_GC
#define unselvarbg_GC xg.app._unselvarbg_GC

#define thisXg xg.app._thisXg
#define displays xg.app._displays
#define current_display xg.app._current_display

#define current_splot xg.app._current_splot
#define mousepos xg.app._mousepos
#define mousepos_o xg.app._mousepos_o
#define mono_p xg.app._mono_p


#define menubar xg.app._menubar
#define main_accel_group xg.app._main_accel_group

#define brush_reset_menu xg.app._brush_reset_menu
#define brush_link_menu xg.app._brush_link_menu

#define brush_pos xg.app._brush_pos

#define spacing xg.app._spacing

#define identify_link_menu xg.app._identify_link_menu
#define cursor_pos xg.app._cursor_pos

#define gy             xg.app._gy
#define tour2d_io_menu xg.app._tour2d_io_menu
#define parcoords_mode_menu xg.app._parcoords_mode_menu

#define scatmat_mode_menu xg.app._scatmat_mode_menu
#define scatterplot_mode_menu xg.app._scatterplot_mode_menu

#define rotation_io_menu xg.app._rotation_io_menu
#define scale_reset_menu xg.app._scale_reset_menu



XGOBI_ xgobid xg;


#define XGOBI_H




#endif

