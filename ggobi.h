#ifndef GGOBI_H
#define GGOBI_H

#include "defines.h"
#include "types.h"
#include "display.h"
#include "display_tree.h"

#include "varseldata.h"

/*
 These are hooks for other applications (e.g. R) to 
 facilitate callbacks at a higher level that GTK events/signals.
 This one is used for responding to identifying points.
 */
typedef void (*IdentifyProc)(void *user_data, gint id, splotd *sp,
  GtkWidget *w, ggobid *gg);
typedef struct {
  IdentifyProc handler;
  void *user_data;
} IdentifyHandler;

typedef struct {
 gint ncols, nrows;
 vardatad *vardata;
 GArray *rowlab;

 gboolean single_column;

 array_f raw, tform1, tform2;
 array_l world, jitdata;

 /*----------------------- missing values ---------------------------*/

 gint nmissing;
 array_s missing;  /*-- array of shorts --*/
 array_l missing_jitter, missing_world;
 gfloat missing_jitter_factor;
 lims missing_lim;  /*-- usually 0,1, but potentially wider --*/

 /*---------------- deleting the hidden points; subsetting ----------*/

 gint *rows_in_plot;
 gint nrows_in_plot;
 gboolean *sampled;

 /*--------------- clusters: hiding, excluding ----------------------*/
 
 struct _Clusterd {
   gint nclusters;
   clusterd *clusv;
   vector_i clusterid;
   gboolean *included;
 } cluster;

 /*----------------------- row grouping -----------------------------*/

 gint nrgroups, nrgroups_in_plot;
 gint *rgroup_ids;
 rgroupd *rgroups;

 /*--------------------------- jittering --------------------------------*/

 struct _Jitterd {
   gfloat factor;
   gboolean type;
   gboolean vgroup;
   gboolean convex;
   gfloat *jitfacv;
 } jitter;

/*-------------------------- brushing ----------------------------------*/

 gint npts_under_brush;
 gboolean *pts_under_brush;
 gshort *color_ids, *color_now, *color_prev;  /* 0:ncolors-1 */
 glyphv glyph_id, glyph_0;
 glyphv *glyph_ids, *glyph_now, *glyph_prev;
 gboolean *hidden, *hidden_now, *hidden_prev;
 bin_struct **binarray;

/*------------------------ identification ------------------------------*/

 GSList *sticky_ids;

/*---------------------- moving points ---------------------------------*/

 GSList *movepts_history;  /*-- a list of elements of type celld --*/

/*------------------- variable selection panel -------------------------*/

 struct _Varpaneld {
   GtkWidget **da, **label;
   gint vnrows, vncols, nvars;
 } varpanel;

} datad;

struct _ggobid;

typedef struct {

 struct _ggobid *thisGG;

 /*-- used in identification, line editing, and point motion --*/
 gint nearest_point, nearest_point_prev; 

 GtkWidget *scatmat_mode_menu;
 GtkWidget *scatterplot_mode_menu;
 GtkWidget *rotation_io_menu;
 GtkAccelGroup *sp_accel_group;

} GGobiApp;

struct _ggobid {

 DisplayTree display_tree;
 GList *displays;
 displayd *current_display;
 splotd *current_splot; 

 GGobiApp app;

 datad d;
 datad edged;

 icoords mousepos, mousepos_o;

 /* main_ui */
 GtkWidget *control_panel[NMODES];
 GtkWidget *main_window, *main_menubar;
 GtkAccelGroup *main_accel_group;
 GtkWidget *mode_frame, *mode_item;
 GtkTooltips *tips;
 gboolean firsttime;  

 gboolean close_pending;
 IdentifyHandler identify_handler;

/*----------------------------------------------------------------------*/
/*                        reading in the data                           */
/*----------------------------------------------------------------------*/

 DataMode data_mode;
 gchar *data_in;
 gchar *filename;      /* full file name, including path */
 gchar *fname;         /* file name without suffix: .dat, .missing */

 /* gboolean: does this data contain only one variable? False by default */
 gboolean single_column;

/*----------------------- pipeline ---------------------------------*/

 gint ncols, nrows;
 vardatad *vardata;
 GArray *rowlab;

 gint mode, prev_mode;
 gint projection, prev_projection;

 array_f raw, tform1, tform2;
 array_l world, jitdata;

 struct _Vartable {
   GtkWidget *window;
   GtkWidget *clist;
 } vartable;

/*----------------------- missing values ---------------------------*/

 gint nmissing;
 array_s missing;  /*-- array of shorts --*/
 array_l missing_jitter, missing_world;
 gfloat missing_jitter_factor;
 lims missing_lim;  /*-- usually 0,1, but potentially wider --*/

/*---------------- deleting the hidden points; subsetting ----------*/

 gint *rows_in_plot;
 gint nrows_in_plot;
 gboolean *sampled;

/*--------------- clusters: hiding, excluding ----------------------*/
 
 clusterd *clusv;
 gint nclust;
 vector_i clusterid;
 gboolean *included;

/*----------------------- row grouping -----------------------------*/

 gint nrgroups, nrgroups_in_plot;
 gint *rgroup_ids;
 rgroupd *rgroups;

/*---------------------- transformation --------------------------------*/

 gint std_type;  /* Can be 0, 1 or 2 */

 struct _Transformation {
   GtkWidget *stage0_opt, *stage1_opt, *stage2_opt;
   GtkAdjustment *boxcox_adj;
 } tform;

 struct _Sphere {
   /* sphering transformation */
   GtkWidget *window;
   GtkAdjustment *npcs_adj;
   GtkWidget *totvar_entry, *condnum_entry;
   GtkWidget *sphere_apply_btn;

   gint nspherevars;
   gint *spherevars;
   gint sphere_npcs;

   gfloat *eigenval;
   gfloat **eigenvec;
   gfloat **vc;
   gfloat *tform1_mean;
 } sphere;

/*---------------- segments; edges ---------------------------------*/

 /*-- data on edges; edge pipeline --*/
 struct _EdgeData {
   datad d;

   gint n, n_in_plot, ncols;  /*-- number of edges and edge variables --*/
   gboolean *sampled;
   vardatad *vardata;  /*-- include this in gg.vartable? --*/
   GArray *lbl;        /*-- we certainly could have edge labels --*/
   array_f raw, tform1, tform2;
   array_l world, jitdata;

/* this isn't really part of the edgedata -- ie, it's not parallel
   to the regular data.  Rather it's part of the graph data.
*/
   vector_i head, tail;  /*-- vectors of length n --*/

 } edge;

 /*-- would these two be superseceded by the preceding? --*/
 gint nsegments;
 endpointsd *segment_endpoints;

 /*-- line brushing --*/
 struct _LineData {
   gint *nxed_by_brush;
   vector_b xed_by_brush;
   vector_s color, color_now, color_prev;
   vector_b hidden, hidden_now, hidden_prev;
 } line;

 /*-- line groups --*/
 gint nlgroups;
 gint *lgroup_ids;
 rgroupd *lgroups;  /* id, nels, *els */


/*----------------------------------------------------------------------*/
/*                           color                                      */
/*----------------------------------------------------------------------*/

 gint ncolors;
 GdkColor *default_color_table, *color_table;  /* brushing colors */
 GdkColor bg_color;          /* background color */
 GdkColor accent_color;      /* color for axes and labels */
 gshort color_id, color_0;   /* 0:ncolors-1 */
 gboolean mono_p;

 struct _Color_UI {
   GtkWidget *symbol_window;
   GtkWidget *symbol_display;

   GtkWidget *colorseldlg;
   GtkWidget *bg_da, *accent_da, *fg_da[NCOLORS], *current_da;

   gint spacing;
   gint margin;  /* between glyphs in the symbol_display */
 } color_ui;

/*---------------------- graphics contexts -----------------------------*/

 GdkGC *plot_GC;
 GdkGC *selvarfg_GC, *selvarbg_GC;     /* white background, thick lines */
 GdkGC *unselvarfg_GC, *unselvarbg_GC; /* grey background, thin lines */

/*--------------------------- jittering --------------------------------*/

 struct _Jitter {
   gfloat factor;
   gboolean type;
   gboolean vgroup;
   gboolean convex;
   gfloat *jitfacv;
 } jitter;

/*------------------------- writing out data ---------------------------*/

 struct _Save {
   gint format, stage, row_ind, column_ind, missing_ind;
   gboolean jitter_p, lines_p;
 } save;

/*---------------------- 1d plotting -----------------------------------*/

 struct _Ash {
   GtkWidget *type_opt;
   GtkObject *smoothness_adj;
 } ash;
 struct _P1D {
   GtkObject *cycle_speed_adj;
   gboolean cycle_p;
   /*-- texture --*/
   gfloat *gy;
 } p1d;

/*---------------------- 2d plotting -----------------------------------*/

 struct _XYPlot {
   gboolean cycle_p;
   gint direction;
 } xyplot;

/*------------------------ touring -------------------------------------*/

 struct _Tour2d {
   GtkWidget *io_menu;
   gint idled; 
 } tour2d;

/*---------------------- parallel coordinates --------------------------*/

 struct _Parcoords {
   GtkAccelGroup *pc_accel_group;
   GtkWidget *arrangement_box;
   GtkWidget *mode_menu;
 } parcoords;

/*-------------------------- brushing ----------------------------------*/

 gint npts_under_brush;
 gboolean *pts_under_brush;
 gshort *color_ids, *color_now, *color_prev;  /* 0:ncolors-1 */
 glyphv glyph_id, glyph_0;
 glyphv *glyph_ids, *glyph_now, *glyph_prev;
 gboolean *hidden, *hidden_now, *hidden_prev;

 struct _Brush_UI {
   GtkWidget *reset_menu;
   GtkWidget *link_menu;
   GtkWidget *mode_opt;
   GtkWidget *cg_opt;
   GtkWidget *scope_opt;
   GtkWidget *brush_on_btn;
   brush_coords brush_pos ;  

   /* binning */
   gint nbins;
   bin_struct **binarray;
   icoords bin0, bin1;

   gboolean firsttime;
 } brush;

 struct _Plot {
  /*
   * Part of binning: the corners of the bin to be copied from
   * pixmap0 to pixmap1.
   * They're defined in splot_draw_to_pixmap0_binned and used in
   * splot_pixmap0_to_pixmap1 when binned == true.
  */
  icoords bin0, bin1;
  icoords loc0, loc1;
 } plot;

/*------------------------ identification ------------------------------*/

 struct _Identify {
   GSList *sticky_ids;
   GtkWidget *link_menu;
 } identify;

/*----------------------- submenu management ---------------------------*/

 struct _Mode_SubMenu {
   GtkWidget *reset_item;
   GtkWidget *link_item;
   GtkWidget *io_item;
 
   gboolean firsttime_reset;
   gboolean firsttime_link;
   gboolean firsttime_io;
  
 } mode_menu;

/*---------------------- transformation --------------------------------*/

 gint std_type;  /* Can be 0, 1 or 2 */

 struct _Transformation {
   GtkWidget *stage0_opt, *stage1_opt, *stage2_opt;
   GtkAdjustment *boxcox_adj;
 } tform;

 struct _Sphere {
   /* sphering transformation */
   GtkWidget *window;
   GtkAdjustment *npcs_adj;
   GtkWidget *totvar_entry, *condnum_entry;
   GtkWidget *sphere_apply_btn;

   gint nspherevars;
   gint *spherevars;
   gint sphere_npcs;

   gfloat *eigenval;
   gfloat **eigenvec;
   gfloat **vc;
   gfloat *tform1_mean;
 } sphere;

/*---------------------- subsetting ------------------------------------*/

 struct _Subset {
   gboolean rescale_p;
   GtkWidget *window;
   GtkWidget *notebook;
   /*-- the entry widgets from which to get values for sample, rowlab --*/
   GtkWidget *random_entry, *rowlab_entry;
   /*-- the adjustments from which to get values for blocksize, everyn --*/
   GtkAdjustment *bstart_adj, *bsize_adj;
   GtkAdjustment *estart_adj, *estep_adj;
 } subset;

/*---------------------- scaling ---------------------------------------*/

 struct _Scale {
   GtkWidget *scale_reset_menu;
   /*-- widgets whose sensitivity needs to be turned on and off --*/
   GtkWidget *pan_opt, *zoom_opt, *pan_radio, *zoom_radio;
   rectd click_rect;
 } scale;

/*---------------------- imputation ------------------------------------*/

 struct _Impute {
   gboolean rescale_p, vgroup_p;
   gint whichvars;
   GtkWidget *window;
   GtkWidget *notebook;
   /*-- the entry widgets from which to get values to impute --*/
   GtkWidget *entry_above, *entry_below, *entry_val;
 } impute;

/*---------------------- moving points ---------------------------------*/

 struct _MovePts {
   gboolean cluster_p;
   enum directiond direction;
   lcoords eps;
   GSList *history;  /*-- a list of elements of type celld --*/
 } movepts;


/*------------------- variable selection panel -------------------------*/

 struct _Varpanel_ui {
   GtkWidget *varpanel;
   GtkWidget *scrolled_window;
   GtkAccelGroup *varpanel_accel_group;
   
   GtkWidget **da, **varlabel;
   gint vnrows, vncols, nvars;
 } varpanel_ui;

/*------------------- variable selection menus -------------------------*/

 struct {
  varseldatad vdata0, vdata1;
 } p1d_menu;

 struct {
  varseldatad vdata0, vdata1;
 } parcoords_menu;

 struct {
  varseldatad vdata0, vdata1;
 } xyplot_menu;

 struct {
  varseldatad vdata0, vdata1, vdata2;
 } rotation_menu;

 struct {
  varseldatad vdata0, vdata1, vdata2;
 } tour2d_menu;

 struct {
  varseldatad vdata0, vdata1, vdata2, vdata3;
 } scatmat_menu;

/*----------------------------------------------------------------------*/

}; /*  ggobid; */

#define GGOBI_H

#endif

