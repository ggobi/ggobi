/*-- session.h --*/
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#ifndef GGOBI_H
#define GGOBI_H

#ifdef WIN32
#define GGOBI_EXPORT __declspec(dllexport)
#else
#define GGOBI_EXPORT 
#endif



struct _GGobiSession;

#include <libxml/parser.h>

#include "defines.h"
#include "types.h"
#include "brushing.h"
#include "display.h"
#include "display_tree.h"
#include "data-factory.h"
#include "input-source.h"
#include "read_init.h"
#include "ggobi-pipeline-factory.h"
#include "input-source-factory.h"

#include "fileio.h"

#include "colorscheme.h"

/*
 These are hooks for other applications (e.g. R) to 
 facilitate callbacks at a higher level that GTK events/signals.
 This one is used for responding to identifying points.
 */
typedef void (*IdentifyProc) (void *user_data, gint id, splotd * sp,
                              GtkWidget * w, GGobiSession * gg);

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
typedef gboolean(*KeyEventHandlerFunc) (guint keyval, GtkWidget * w,
                                        GdkEventKey * event,
                                        cpaneld * cpanel, splotd * sp,
                                        GGobiSession * gg, void *userData);

typedef void (*ReleaseData) (void *userData);
  /* This is the variable that stores the registered handler */
typedef struct {
  KeyEventHandlerFunc handlerRoutine;
  void *userData;
  char *description;
  ReleaseData *releaseData;
  ProgrammingLanguage language;
} KeyEventHandler;


typedef struct {
/*-- ggobi --*/

  struct _GGobiSession *thisGG;

  GtkAccelGroup *sp_accel_group; /*-- sp = scatterplot here --*/

} GGobiMenus;

typedef struct _PrintOptions PrintOptions;



GType ggobi_session_get_type(void);

#define GGOBI_TYPE_SESSION (ggobi_session_get_type())
#define GGOBI_SESSION(obj)	G_TYPE_CHECK_INSTANCE_CAST((obj), ggobi_session_get_type(), GGobiSession)
#define GGOBI_SESSION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_SESSION, GGobiSessionClass))
#define GGOBI_IS_SESSION(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_SESSION))
#define GGOBI_IS_SESSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_SESSION))
#define GGOBI_SESSION_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_SESSION, GGobiSessionClass))

typedef struct _GGobiSessionClass {
  GObjectClass parent_class;
} GGobiSessionClass;


/**
  @defgroup GGobiSession the GGobiSession instance structure.
  @brief This is the top-level structure representing a 
  ggobi instance.
 */
struct _GGobiSession {

  GObject object;

    /** 
       A tree 
     */
  DisplayTree display_tree;
  GList *displays;
  displayd *current_display;
  splotd *current_splot;
  gint buttondown;/*-- can be 0, 1, 2, or 3; could be useful in drawing --*/

  GGobiMenus app;

  GSList *d;                    /* Datasets (datad elements) */
            /*-- first is default: cases, nodes; second might be edges --*/

  /* main_ui */
  GtkWidget *current_control_panel;

  GList *control_panels;

  GtkWidget *main_window, *main_menubar;
  GtkUIManager *main_menu_manager;
  GtkWidget *display_menu; /*-- menu labelled 'Window' --*/
  GtkAccelGroup *main_accel_group, *pmode_accel_group, *imode_accel_group;
  GtkWidget *pmode_item, *imode_item;
  GtkWidget *imode_frame;  /* this should be cpanel_frame, actually */
  GtkTooltips *tips;
  gboolean firsttime;
  guint mode_merge_id;

  /* status bar in main console window */
  void (*status_message_func) (gchar *, GGobiSession *);
  gboolean statusbar_p;

  gboolean close_pending;
  IdentifyHandler identify_handler;

/*--------------------------------------------------------------------*/
/*                      reading in the data                           */
/*--------------------------------------------------------------------*/
  
  GGobiInputSource *data_source;/* Information about input files for the current
                                   data source, such as the display name, uri, etc
                                 */

/*----------------------- pipeline ---------------------------------*/

  ProjectionMode pmode, pmode_prev;
  InteractionMode imode, imode_prev;

  struct _VarTableUI {
    GtkWidget *window;
    GtkWidget *notebook;
  } vartable_ui;
  gboolean lims_use_visible;


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
  GdkGC *rectangle_GC;
  GdkColor mediumgray, lightgray, darkgray;  /* for 3d rectangles */
  GdkColor vcirc_freeze_color, vcirc_manip_color; /* for variable circles */
  gshort color_id, color_0;     /* 0:ncolors-1 */
  gboolean mono_p;

  struct _Color_UI {
    GtkWidget *symbol_window;
    GtkWidget *symbol_display, *line_display;

    GtkWidget *colorseldlg;
    GtkWidget *bg_da, *accent_da, *hidden_da, *fg_da[MAXNCOLORS], *current_da;

    gint spacing;
    gint margin;                /* between glyphs in the symbol_display */
  } color_ui;

/*---------------------- graphics contexts -----------------------------*/

  GdkGC *plot_GC;
  GdkGC *selvarfg_GC, *selvarbg_GC;     /* white background, thick lines */
  GdkGC *unselvarfg_GC, *unselvarbg_GC; /* grey background, thin lines */
  GdkGC *manipvarfg_GC;         /* white background, thin purple line */

/*--------------------------- jittering --------------------------------*/

  struct _JitterUI {
    GtkWidget *window;
  } jitter_ui;

/*------------------------- writing out data ---------------------------*/

  struct _Save {
    GtkWidget *tree_view;
    gint format, stage, row_ind, column_ind, missing_ind;
    gboolean jitter_p, edges_p;
  } save;

/*---------------------- 1d plotting -----------------------------------*/

  struct _P1D {
   /*-- cycling --*/
    gint cycle_id;
    GtkAdjustment *cycle_delay_adj;
   /*-- texture --*/
    gdouble *gy;
  } p1d;

/*-------------------- 2d plotting -----------------------------------*/

  struct _XYPlot {
   /*-- cycling --*/
    gint cycle_id;
    GtkAdjustment *cycle_delay_adj;
  } xyplot;

/*---------------------- touring -------------------------------------*/

#ifndef NEW_TOUR_CLASSES
  struct _Tour2d3 {
    gint idled;
  } tour2d3;

  struct _Tour2d {
    gint idled;
    gboolean fade_vars;
    gboolean all_vars;
  } tour2d;

  struct _Tour1d {
    gint idled;
    gboolean fade_vars;
    gboolean all_vars;
  } tour1d;

  struct _TourCorr {
    gint idled;
    gboolean fade_vars;
  } tourcorr;
#else

#endif
/*-------------------- parallel coordinates --------------------------*/

  struct _Parcoords {
    GtkAccelGroup *accel_group;
    GtkWidget *arrangement_box;
  } parcoords;

/*---------------------time series------------------------------------*/

/*XX */
  struct _TSPLOT {
    GtkAccelGroup *accel_group;
    GtkWidget *arrangement_box;
  } tsplot;



/*------------------------ brushing ----------------------------------*/

  glyphd glyph_id, glyph_0;

  struct _Brush_UI {
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

  struct _Main_MiscMenu {
    GtkWidget *options_item, *options_menu;
  } menus;

/*-------------------- transformation --------------------------------*/

  GtkWidget *transform_gui;
  
  struct _Sphere {
    GtkWidget *window;
    GtkWidget *scree_da;
    GdkPixmap *scree_pixmap;

    GtkObject *npcs_adj;
    GtkWidget *stdized_entry, *variance_entry, *condnum_entry;
    GtkWidget *apply_btn, *restore_btn;
    GtkWidget *tree_view;

   /*-- a pointer to be compared with current_display->d --*/
    GGobiStage *d;
  } sphere_ui;

/*-------------------- subsetting ------------------------------------*/

  struct _SubsetUI {
    GtkWidget *window;
    GtkWidget *notebook;
  } subset_ui;

/*---------------- color scheme selection -------------------------------*/

  struct _SchemeChooser {
    GtkWidget *window, *entry_preview, *entry_applied, *da;
    GdkPixmap *pix;
    colorschemed *scheme; /*-- current color scheme --*/
    GdkGC *GC;
    gdouble *pct;
    gint npct;
  } svis;

/*---------------- brushing by weights -------------------------------*/

  struct _WeightedVis {
    GtkWidget *window, *da;
    GdkPixmap *pix;
    GdkGC *GC;

    gdouble *pct;
    gint npct;
    gint *n;   /*-- number of points that will take on each color --*/
    gint nearest_color;

    gint motion_notify_id;
    icoords mousepos;
    gint binning_method;
    gint update_method;
  } wvis;


/*-------------------- moving points ---------------------------------*/

  struct _MovePts {
    gboolean cluster_p;
    enum directiond direction;
    gcoords eps;
  } movepts;


/*----------------- variable selection panel -------------------------*/

  struct _Varpanel_ui {
    GtkWidget *notebook;
    gboolean layoutByRow;
  } varpanel_ui;

  KeyEventHandler *NumberedKeyEventHandler;

  PrintOptions *printOptions;

  GList *colorSchemes;
  colorschemed *activeColorScheme;

  GSList *pmodeRadioGroup;
  GSList *imodeRadioGroup;

  void *userData;/** A place to hang data for a host application, plugin, etc. 
                     Since plugins, etc. may also use this, we might want 
                     a hashtable here similar to pthread's thread-specific data. */

  GTimeVal time;  
  
  GGobiPipelineFactory *pipeline_factory;
  
  GSList *data_factories;
  GSList *input_source_factories;
  
}; /*  GGobiSession; */

/* the keys for the built-in stages. eventually these might become
   functions that return a static GQuark, if we move to a GQuark-based ID system */

#define GGOBI_MAIN_STAGE_DISPLAY "ggobi-main-display"
#define GGOBI_MAIN_STAGE_DOMAIN_ADJ "ggobi-main-domain-adj"
#define GGOBI_MAIN_STAGE_FILTER "ggobi-main-filter"
#define GGOBI_MAIN_STAGE_FREEZE "ggobi-main-freeze"
#define GGOBI_MAIN_STAGE_IMPUTE "ggobi-main-impute"
#define GGOBI_MAIN_STAGE_JITTER "ggobi-main-jitter"
#define GGOBI_MAIN_STAGE_RANDOMIZE "ggobi-main-randomize"
#define GGOBI_MAIN_STAGE_STANDARDIZE "ggobi-main-standardize"
#define GGOBI_MAIN_STAGE_SUBSET "ggobi-main-subset"
#define GGOBI_MAIN_STAGE_TRANSFORM "ggobi-main-transform"

#include "read_init.h"

typedef enum { GGOBI_SILENT, GGOBI_CHATTY, GGOBI_VERBOSE } GGobiOutputLevel;

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
  GGobiOutputLevel verbose;

    /**
       @ingroup SessionOptions
       The default format for the data being read.
     */
  DataMode data_mode;

  gchar *data_type;

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

  gchar *restoreFile;

  GSList *pluginFiles;

    /** Directory in which the GGobi files are located.
      */
  gchar *ggobiHome;

  gdouble defaultTourSpeed;
  gdouble defaultTour1dSpeed;

  gboolean useRadioMenuItems;

  /**
    @ingroup SessionOptions
    If true, run timing tests.
    */
  gboolean timingp;

} GGobiOptions;


/**
  Information about move and identify events in ggobi reported
  to listeners.
 */
typedef struct {
  GGobiStage *d;
  int id;
} GGobiPointMoveEvent;

void start_ggobi(GGobiSession * gg, gboolean init_data, gboolean createPlot);
void process_initialization_files();

extern GGobiOptions *sessionOptions;

/**
  Identifiers for the different signal types generated by ggobi
 */
typedef enum { DATAD_ADDED_SIGNAL,
  VARIABLE_ADDED_SIGNAL,         
  VARIABLE_LIST_CHANGED_SIGNAL,  /*-- this works for variable lists --*/
  SPLOT_NEW_SIGNAL,
  BRUSH_MOTION_SIGNAL, POINT_MOVE_SIGNAL, IDENTIFY_POINT_SIGNAL,
  VARIABLE_SELECTION_SIGNAL,
  STICKY_POINT_ADDED_SIGNAL, STICKY_POINT_REMOVED_SIGNAL,
  CLUSTERS_CHANGED_SIGNAL,       /*-- ggvis wants this --*/
  DISPLAY_NEW_SIGNAL,
  DISPLAY_SELECTED_SIGNAL,
  MAX_GGOBI_SIGNALS
} GGobiSignalType;

/**
  Registered signal identifiers for the ggobi signals.
  Indexed by the enum above.
 */
extern guint GGobiSignals[MAX_GGOBI_SIGNALS];


/**
  Should be in edges.h, if there were one.
 */
GGobiStage *setDisplayEdge(displayd * dpy, GGobiStage * e);


gchar *getOptValue(const char *const name, const char *const value);
const char *getCommandLineArgValue(const char *name);
void showHelp();

void globals_init(GGobiSession * gg);

guint getGGobiSignal(GGobiSignalType);

GSList *ggobi_getExtendedDisplayTypes();

void gtk_marshal_NONE__INT_POINTER_POINTER_POINTER(GtkObject * object,
                                                   GtkSignalFunc func,
                                                   gpointer func_data,
                                                   GtkArg * args);


extern GSList *ExtendedDisplayTypes;
typedef GType(*GTypeLoad) (void);

gchar* ggobi_find_data_file(const gchar *name);
gchar* ggobi_find_config_file(const gchar *name);

GList *getInputPluginSelections(GGobiSession *gg);

extern const gchar *DefaultUnknownInputModeName;
	
GSList*    load_data (const gchar *, const gchar *modeName, GGobiSession *);
GSList*    load_data_source (GGobiInputSource *, GGobiSession *);

GGobiInputSource *create_input_source(const gchar *uri, const gchar *mode);
GGobiDataFactory *create_data_factory(GGobiSession *gg, GGobiInputSource *source);

void ggobi_session_register_data_factory(GGobiSession *session, GGobiDataFactory *factory);
void ggobi_session_register_input_source_factory(GGobiSession *session, 
  GGobiInputSourceFactory *factory);

#include "GGobiEvents.h"

#endif
