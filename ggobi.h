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
#include "brushing.h"
#include "display.h"
#include "display_tree.h"
#include "read_init.h"

#include "fileio.h"

#include "colorscheme.h"

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

 /*-- viewmode menus --*/
 GtkWidget *scatmat_mode_menu;
 GtkWidget *scatterplot_mode_menu;

 GtkAccelGroup *sp_accel_group;  /*-- sp = scatterplot here --*/

} GGobiApp;

typedef struct _PrintOptions PrintOptions;

/**
  @defgroup ggobid the ggobid instance structure.
  @brief This is the top-level structure representing a 
  ggobi instance.
 */
struct _ggobid {

    /** 
       A tree 
     */
 DisplayTree display_tree;
 GList *displays;
 displayd *current_display;
 splotd *current_splot;
 gint buttondown; /*-- can be 0, 1, 2, or 3; could be useful in drawing --*/

 /*-- signals --*/
#ifdef DATAD_ADDED_SIGNAL_IMPLEMENTED
 guint signal_datad_added;
#endif
 guint signal_variable_added;

 GGobiApp app;

 GSList *d; /* Datasets (datad elements) */ 
            /*-- first is default: cases, nodes; second might be edges --*/

 /* main_ui */
 GtkWidget *control_panel[NMODES];
 GtkWidget *main_window, *main_menubar;
 GtkItemFactory *main_menu_factory;
 GtkWidget *display_menu_item, *display_menu;  /*-- menu labelled 'Window' --*/
 GtkAccelGroup *main_accel_group;
 GtkWidget *viewmode_frame, *viewmode_item;
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

 PipelineMode viewmode, prev_viewmode;
 PipelineMode projection, prev_projection;

 struct _VarTableUI {
   GtkWidget *window;
   GtkWidget *notebook;
 } vartable_ui;
 gboolean lims_use_visible;


/*----------------------- missing values ---------------------------*/

/*---------------- deleting the hidden points; subsetting ----------*/

/*--------------- clusters: hiding, excluding ----------------------*/

 struct _ClusterUI {
   GtkWidget *window;
   GtkWidget *notebook;
 } cluster_ui;
 
/*---------------- brushing by categorical variable ----------------*/

  gboolean linkby_cv;

/*--------------------------------------------------------------------*/
/*                         color                                      */
/*--------------------------------------------------------------------*/

 gint ncolors;
 gchar **colorNames;
 GdkColor *color_table;      /* brushing colors */
 GdkColor bg_color;          /* background color */
 GdkColor accent_color;      /* color for axes and labels */
 GdkColor vcirc_freeze_color, vcirc_manip_color;  /* for variable circles */
 gshort color_id, color_0;   /* 0:ncolors-1 */
 gboolean mono_p;

 struct _Color_UI {
   GtkWidget *symbol_window;
   GtkWidget *symbol_display, *line_display;

   GtkWidget *colorseldlg;
   GtkWidget *bg_da, *accent_da, *fg_da[MAXNCOLORS], *current_da;

   gint spacing;
   gint margin;  /* between glyphs in the symbol_display */
 } color_ui;

/*---------------------- graphics contexts -----------------------------*/

 GdkGC *plot_GC;
 GdkGC *selvarfg_GC, *selvarbg_GC;     /* white background, thick lines */
 GdkGC *unselvarfg_GC, *unselvarbg_GC; /* grey background, thin lines */
 GdkGC *manipvarfg_GC;              /* white background, thin purple line */

/*--------------------------- jittering --------------------------------*/

 struct _JitterUI {
   GtkWidget *window;
 } jitter_ui;

/*------------------------- writing out data ---------------------------*/

 struct _Save {
   gint format, stage, row_ind, column_ind, missing_ind;
   gboolean jitter_p, lines_p;
 } save;

/*---------------------- 1d plotting -----------------------------------*/

 struct _P1D {
   GtkObject *cycle_speed_adj;
   gboolean cycle_p;
   gint cycle_id;
   gint cycle_dir;
   /*-- texture --*/
   gfloat *gy;
 } p1d;

/*-------------------- 2d plotting -----------------------------------*/

 struct _XYPlot {
   gint cycle_id;
   GtkAdjustment *cycle_delay_adj;
 } xyplot;

/*---------------------- touring -------------------------------------*/

 struct _Tour2d {
   gint idled;
   gboolean fade_vars;
 } tour2d;

 struct _Tour1d {
   gint idled; 
   gboolean fade_vars;
 } tour1d;

 struct _TourCorr {
   gint idled; 
   gboolean fade_vars;
 } tourcorr;

/*-------------------- parallel coordinates --------------------------*/

 struct _Parcoords {
   GtkAccelGroup *accel_group;
   GtkWidget *arrangement_box;
   GtkWidget *mode_menu;
 } parcoords;

/*---------------------time series------------------------------------*/  

 struct _TSPLOT {
   GtkAccelGroup *accel_group;
   GtkWidget *arrangement_box;
   GtkWidget *mode_menu;
 } tsplot;

/*------------------------ brushing ----------------------------------*/

 glyphd glyph_id, glyph_0;

 struct _Brush_UI {
   gboolean updateAlways_p;
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

/*---------------------- edge editing --------------------------------*/

 struct _EdgeEdit {
   gint a;
 } edgeedit;

/*--------------------- submenu management ---------------------------*/

 struct _Mode_SubMenu {
   GtkWidget *options_item, *options_menu;
   GtkWidget *reset_item, *reset_menu;
   GtkWidget *io_item, *io_menu;
 } menus;

/*-------------------- transformation --------------------------------*/

 struct _Transformation {
   GtkWidget *window;
   GtkAdjustment *boxcox_adj;
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

 /*-- all the widgets here except the window should be removed
      from this file and retrieved by name instead.  --*/
 struct _SubsetUI {
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

/*---------------- brushing by weights -------------------------------*/

 struct _WeightedVis {
   GtkWidget *window;
   GtkWidget *da;
   GdkPixmap *pix;
   GdkColor gray1, gray2, gray3;

   colorschemed *scheme;  /*-- current color scheme --*/
   gint ncolors;    
   GdkColor *color_table, bg_color, accent_color;
   GdkGC *GC;

   gfloat *pct;
   gint npct;
   gint *n;    /*-- number of points that will take on each color --*/
   gint nearest_color;

   gint motion_notify_id;
   icoords mousepos;
 } wvis;

/*-------------------- scaling ---------------------------------------*/

 struct _Scale {
   rectd click_rect;
 } scale;

/*-------------------- imputation ------------------------------------*/

 struct _Impute {
   gboolean bgroup_p;
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
 } movepts;


/*----------------- variable selection panel -------------------------*/

 struct _Varpanel_ui {
   GtkWidget *notebook;
   gboolean layoutByRow;
 } varpanel_ui;


 KeyEventHandler *NumberedKeyEventHandler;

 PrintOptions *printOptions;
 GList *pluginInstances;

 GList *colorSchemes;
 colorschemed *activeColorScheme;
}; /*  ggobid; */

#ifdef USE_XML
#include "read_init.h"
#endif


/**
  @defgroup SessionOptions Session Options
  @brief This is used to store store values from the
   command line arguments which can be used in subsequent
   code.
 */
typedef struct {

  /**
    @ingroup SessionOptions
    Controls whether messages explaining what is being done internally
    are displayed.
    */
  gboolean verbose;

    /**
       @ingroup SessionOptions
       The default format for the data being read.
     */
  DataMode data_mode;

  /**
    @ingroup SessionOptions
    The name of the data file containing the data.
    */
  gchar *data_in;
    /**
       @ingroup SessionOptions
       The command line arguments used to start ggobi.
     */ 
  gchar **cmdArgs;
    /**
      @ingroup SessionOptions
      The number of command line arguments used to start ggobi.
     */
  gint numArgs;


    /**
       @ingroup SessionOptions
       A logical value controlling whether the control window
       of a ggobi instance is displayed. This can be used to hide
       the control window when ggobi is embedded in other applications.
     */
  gboolean showControlPanel;

#ifdef USE_XML
    /**
      @ingroup SessionOptions
      Data for the initialization settings.
     */
  struct _GGobiInitInfo *info;
    /**
      @ingroup SessionOptions
      The name of the initialization file from which to read any
      session parameters not specified on the command line.
     */
  gchar *initializationFile;

    /**
      @ingroup SessionOptions
      The collection of color schemes read
       available to ggobi, typically specified in the
       initialization file.
      
     */
  GList *colorSchemes;
    /**
     @ingroup SessionOptions
     The name of the active color scheme, indexing the 
     list of colorSchemes.
     */
  gchar *activeColorScheme;
#endif
} GGobiOptions;


gboolean read_input(InputDescription *desc, ggobid *gg);
void start_ggobi(ggobid *gg, gboolean init_data, gboolean createPlot);
void process_initialization_files();

extern GGobiOptions *sessionOptions;

enum {DATA_NEW_SIGNAL, SPLOT_NEW_SIGNAL, BRUSH_MOTION_SIGNAL,  MAX_GGOBI_SIGNALS};
extern guint GGobiSignals[MAX_GGOBI_SIGNALS];
#endif

