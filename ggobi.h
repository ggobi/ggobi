/*-- ggobi.h --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#ifndef GGOBI_H
#define GGOBI_H

#include "defines.h"
#include "types.h"
#include "display.h"
#include "display_tree.h"

#include "varseldata.h"

#include "fileio.h"

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


/*
 This is a typedef that is used for registering a routine for handling
 presses of the numbered keys 0, 1, ..., 9.
 It gets the events from the regular event handler and also the key identifier.
 See scatterplot_event_handled() in splot.c.
 The idea is that people can register routines in a programming
 language interface to ggobi such as Rggobi, or the Perl or Python
 interfaces, and provide customized callbacks/handlers for the
 numbered keys.
 Note that if you register a handler for any of these keys, you have to handle
 them all (i.e. 0, ..., 9).  You can discard the ones you are not interested in.
 In the future, we may set it up so that one can refuse to handle an event
 and return false and have the scatterplot_event_handled() then process it
 in the default way. Also, one can register an intermediate handler which 
 looks up a table for key-specific handlers for each the 0, .., 9 keys.
 Then one could register that function with the general ggobi mechanism
 and provide an interface
 */
typedef gboolean (*KeyEventHandlerFunc)(guint keyval, GtkWidget *w, GdkEventKey *event,  cpaneld *cpanel, splotd *sp, ggobid *gg, void *userData);

typedef void (*ReleaseData)(void *userData);
  /* This is the variable that stores the registered handler */
typedef struct {
  KeyEventHandlerFunc handlerRoutine;
  void *userData;
  char *description;
  ReleaseData *releaseData;
  ProgrammingLanguage language;
} KeyEventHandler;


struct _ggobid;

typedef struct /*-- ggobi --*/ {

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

 GSList *d;  /*-- first is default: cases, nodes; second might be edges --*/

 /* main_ui */
 GtkWidget *control_panel[NMODES];
 GtkWidget *main_window, *main_menubar;
  GtkItemFactory *main_menu_factory;
 GtkWidget *display_menu_item, *display_menu;  /*-- menu labelled 'Window' --*/
 GtkAccelGroup *main_accel_group;
 GtkWidget *mode_frame, *mode_item;
 GtkTooltips *tips;
 gboolean firsttime;  

 gboolean close_pending;
 IdentifyHandler identify_handler;

/*--------------------------------------------------------------------*/
/*                      reading in the data                           */
/*--------------------------------------------------------------------*/

 InputDescription *input;   /* Information about input files for the default
                               data source, such as the name of the
                               file, directory, data mode, extension, etc.
                             */

/*----------------------- pipeline ---------------------------------*/

 gint mode, prev_mode;
 gint projection, prev_projection;

 struct _VarTableUI {
   GtkWidget *window;
   GtkWidget *notebook;
   GtkWidget *umin, *umax;   /*-- text entry widgets for setting ranges --*/
 } vartable_ui;

/*----------------------- missing values ---------------------------*/

/*---------------- deleting the hidden points; subsetting ----------*/

/*--------------- clusters: hiding, excluding ----------------------*/

 struct _ExclusionUI {
   GtkWidget *window;
   GtkWidget *notebook;
 } exclusion_ui;
 
/*----------------------- row grouping -----------------------------*/

/*--------------------------------------------------------------------*/
/*                         color                                      */
/*--------------------------------------------------------------------*/

 gint ncolors;
 gchar **colorNames;
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

 struct _JitterUI {
   GtkWidget *window;

   /*-- a pointer to be compared with current_display->d --*/
   datad *d;
 } jitter_ui;

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

/*-------------------- 2d plotting -----------------------------------*/

 struct _XYPlot {
   gboolean cycle_p;
   gint direction;
 } xyplot;

/*---------------------- touring -------------------------------------*/

 struct _Tour2d {
   GtkWidget *io_menu;
   gint idled; 
 } tour2d;

 struct _Tour1d {
   GtkWidget *io_menu;
   gint idled; 
 } tour1d;

 struct _TourCorr {
   GtkWidget *io_menu;
   gint idled; 
 } tourcorr;

/*-------------------- parallel coordinates --------------------------*/

 struct _Parcoords {
   GtkAccelGroup *pc_accel_group;
   GtkWidget *arrangement_box;
   GtkWidget *mode_menu;
 } parcoords;

/*---------------------time series------------------------------------*/  

 struct _TSPLOT {
   GtkAccelGroup *pc_accel_group;
   GtkWidget *arrangement_box;
   GtkWidget *mode_menu;
 } tsplot;

/*------------------------ brushing ----------------------------------*/

 glyphv glyph_id, glyph_0;

 struct _Brush_UI {
   GtkWidget *reset_menu;
   GtkWidget *link_menu;
   GtkWidget *mode_opt;
   GtkWidget *cg_opt;
   GtkWidget *scope_opt;
   GtkWidget *brush_on_btn;

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

/*---------------------- identification ------------------------------*/

 struct _Identify {
/*
   GSList *sticky_ids;
*/
   GtkWidget *link_menu;
 } identify;

/*--------------------- submenu management ---------------------------*/

 struct _Mode_SubMenu {
   GtkWidget *reset_item;
   GtkWidget *link_item;
   GtkWidget *io_item;
 
   gboolean firsttime_reset;
   gboolean firsttime_link;
   gboolean firsttime_io;
  
 } mode_menu;

/*-------------------- transformation --------------------------------*/

/*
 gint std_type;
*/

 struct _Transformation {
   GtkWidget *window;
   GtkWidget *stage0_opt, *stage1_opt, *stage2_opt;
   GtkAdjustment *boxcox_adj;
   GtkWidget *entry_a, *entry_b;
 } tform_ui;

 struct _Sphere {
   GtkWidget *window;
   GtkWidget *scree_da;
   GdkPixmap *scree_pixmap;

   GtkObject *npcs_adj;
   GtkWidget *stdized_entry, *variance_entry, *condnum_entry;
   GtkWidget *apply_btn, *restore_btn;
   GtkWidget *clist;

   /*-- a pointer to be compared with current_display->d --*/
   datad *d;
 } sphere_ui;

/*-------------------- subsetting ------------------------------------*/

 struct _SubsetUI {
   gboolean rescale_p;
   GtkWidget *window;
   GtkWidget *notebook;
   /*-- entry widgets from which to get values for sample, rowlab --*/
   GtkWidget *random_entry, *nrows_entry, *rowlab_entry;
   /*-- spinners --*/
   GtkWidget *bstart, *bsize;
   GtkWidget *bstart_incr, *bsize_incr;
   GtkWidget *estart, *estep;

   /*-- a pointer to be compared with current_display->d --*/
   datad *d;
 } subset_ui;

/*-------------------- scaling ---------------------------------------*/

 struct _Scale {
   GtkWidget *scale_reset_menu;
   /*-- widgets whose sensitivity needs to be turned on and off --*/
   GtkWidget *pan_opt, *zoom_opt, *pan_radio, *zoom_radio;
   rectd click_rect;
 } scale;

/*-------------------- imputation ------------------------------------*/

 struct _Impute {
   gboolean rescale_p, bgroup_p;
   gint whichvars;
   GtkWidget *window;
   GtkWidget *notebook;
   /*-- the entry widgets from which to get values to impute --*/
   GtkWidget *entry_above, *entry_below, *entry_val;
 } impute;

/*-------------------- moving points ---------------------------------*/

 struct _MovePts {
   gboolean cluster_p;
   enum directiond direction;
   lcoords eps;
/*
   GSList *history;
*/
 } movepts;


/*----------------- variable selection panel -------------------------*/

 struct _Varpanel_ui {
   GtkWidget *notebook;
   /*GtkAccelGroup *varpanel_accel_group;*/
   GtkTooltips *tips;
   gboolean layoutByRow;
 } varpanel_ui;


 KeyEventHandler *NumberedKeyEventHandler;

}; /*  ggobid; */


typedef struct {

  gboolean verbose;

  DataMode data_mode;

  gchar *data_in;

  char **cmdArgs;
  int numArgs;
} GGobiOptions;

extern GGobiOptions *sessionOptions;

#endif

