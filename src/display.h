/*-- display.h --*/
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

#ifndef DISPLAY_H
#define DISPLAY_H

#include "defines.h"
#include "cpanel.h"
#include "splot.h"
#include "ggobi-data.h"

#include "session.h"

void       displays_tailpipe (RedrawStyle, GGobiSession *);


#ifdef __cplusplus
extern "C" {
#endif

struct _GGobiSession; 


#define GGOBI_TYPE_DISPLAY	 (ggobi_display_get_type ())
#define GGOBI_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_DISPLAY, displayd))
#define GGOBI_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_DISPLAY, GGobiDisplayClass))
#define GGOBI_IS_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_DISPLAY))
#define GGOBI_IS_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_DISPLAY))
#define GGOBI_DISPLAY_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_DISPLAY, GGobiDisplayClass))

GType ggobi_display_get_type();

enum { TOUR_STEP_SIGNAL, 
       MAX_GGOBI_DISPLAY_SIGNALS};


typedef struct _GGobiDisplayClass
{
  GtkVBoxClass parent_class;
  guint signals[MAX_GGOBI_DISPLAY_SIGNALS];

} GGobiDisplayClass;

struct _displayd {
  GtkVBox vbox;

/*
 * Used by all displays
*/

 GtkWidget *menubar;
 GtkUIManager *menu_manager;
 guint imode_merge_id, pmode_merge_id;
 
 /*-- for scatterplots, where edge menus need to be rebuilt on the fly --*/
 guint edge_merge, edge_option_merge;
 GtkActionGroup *edgeset_action_group;

 cpaneld cpanel;

 GList *splots;          /*-- doubly linked list of splots --*/
 splotd *current_splot;  /*-- multi-plot displays need this notion --*/

 GGobiStage *d;  /*-- pointer to a particular gg->d[] --*/
 GGobiStage *e;  /*-- pointer to a particular gg->d[] --*/
 
 /*-- --*/

/*
 * Actually, this might need to be a pair of vectors
 * or linked lists, corresponding to the number of plots in
 * the display.  But let's be lazy for the moment.
*/
 GtkWidget *hrule, *vrule;

 DisplayOptions options;

/*
 * For an individual scatterplot
*/
  fcoords drag_start;

/*
 * Scatterplot matrix display
*/
 GtkWidget *table;

/*
 * Parallel coordinates display
*/
  gint p1d_orientation;

/*
 * Manipulation Vars
*/
  array_d tc1_manbasis, tc2_manbasis, t1d_manbasis;
  gint tc1_manip_var, tc2_manip_var, t1d_manip_var;
  gint tc1_pos_old, tc1_pos, tc2_pos_old, tc2_pos, t1d_pos_old, t1d_pos;
  /*gint tc_manip_mode;*/
  gboolean tc1_manipvar_inc, tc2_manipvar_inc, t1d_manipvar_inc;
  gfloat tc1_phi, tc2_phi, t1d_phi;

  gint t2d_manip_var, t2d_manipvar_inc;
  gint t2d_pos1_old, t2d_pos1, t2d_pos2_old, t2d_pos2;
  /*gint t2d_manip_mode;*/
  array_d t2d_Rmat1, t2d_Rmat2;
  array_d t2d_manbasis;
  array_d t2d_mvar_3dbasis;
  gboolean t2d_no_dir_flag;
  gfloat t2d_rx, t2d_ry;

/*-- 1d tour --*/
 tour t1d;
 gboolean t1d_axes;
 gboolean t1d_video;

/*-- rotation: 2d tour, constrained to 3 variables --*/
 tour t2d3;
 gboolean t2d3_axes;
 gint t2d3_manip_var;
 array_d t2d3_manbasis, t2d3_mvar_3dbasis;
 array_d t2d3_Rmat1, t2d3_Rmat2;
 gint t2d3_pos1_old, t2d3_pos1, t2d3_pos2_old, t2d3_pos2;
 gfloat t2d3_rx, t2d3_ry;
 gboolean t2d3_no_dir_flag;
 gboolean t2d3_manipvar_inc;

/*-- 2d tour --*/
 tour t2d;
 gboolean t2d_axes;
 gboolean t2d_video;

/*-- corr tour --*/
 tour tcorr1, tcorr2;
 gboolean tcorr_axes;
 gboolean tourcorr_video;

/* projection pursuit */
 GtkWidget *t1d_pplabel, *t2d_pplabel;
 GtkWidget *t1d_ppda, *t2d_ppda;
 GdkPixmap *t1d_pp_pixmap, *t2d_pp_pixmap;
 GtkWidget *t1d_window, *t2d_window;
 GtkWidget *t1d_control_frame, *t2d_control_frame;
 GtkWidget *t1d_mbar, *t2d_mbar;
 GtkAccelGroup *t1d_pp_accel_group, *t2d_pp_accel_group;
 optimize0_param t1d_pp_op, t2d_pp_op;
 pp_param t1d_pp_param, t2d_pp_param;
 gfloat t2d_ppindx_mat[100], t1d_ppindx_mat[100];
 gfloat t2d_indx_min, t2d_indx_max, t1d_indx_min, t1d_indx_max;
 gint t2d_ppindx_count, t1d_ppindx_count;

 struct _GGobiSession *ggobi;

};  /* displayd; */




#define GGOBI_TYPE_EMBEDDED_DISPLAY	 (ggobi_embedded_display_get_type ())
#define GGOBI_EMBEDDED_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_EMBEDDED_DISPLAY, displayd))
#define GGOBI_EMBEDDED_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_EMBEDDED_DISPLAY, GGobiEmbeddedDisplayClass))
#define GGOBI_IS_EMBEDDED_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_EMBEDDED_DISPLAY))
#define GGOBI_IS_EMBEDDED_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_EMBEDDED_DISPLAY))
#define GGOBI_EMBEDDED_DISPLAY_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_EMBEDDED_DISPLAY, GGobiEmbeddedDisplayClass))

GType ggobi_embedded_display_get_type();

typedef struct _GGobiEmbeddedDisplayClass
{
    GGobiDisplayClass parent_class;

} GGobiEmbeddedDisplayClass;

typedef struct _embeddedDisplayd {
   displayd display;
} embeddedDisplayd;





#define GGOBI_TYPE_WINDOW_DISPLAY	 (ggobi_window_display_get_type ())
#define GGOBI_WINDOW_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_WINDOW_DISPLAY, windowDisplayd))
#define GGOBI_WINDOW_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_WINDOW_DISPLAY, GGobiWindowDisplayClass))
#define GGOBI_IS_WINDOW_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_WINDOW_DISPLAY))
#define GGOBI_IS_WINDOW_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_WINDOW_DISPLAY))
#define GGOBI_WINDOW_DISPLAY_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_WINDOW_DISPLAY, GGobiWindowDisplayClass))

GType ggobi_window_display_get_type();
displayd *ggobi_window_display_new(gint type, gboolean missing_p, GGobiStage *d, GGobiSession *gg);

typedef struct 
{
    GGobiDisplayClass parent_class;

} GGobiWindowDisplayClass;


typedef struct _windowDisplayd {
 displayd dpy;

 GtkWidget *window;
 gboolean useWindow;

} windowDisplayd;

gboolean isEmbeddedDisplay(displayd *dpy);



/*
 This is used as a trivial class for its type information so that we
 can detect whether we have one of the new style classes.  We might
 remove it when we are finished the construction.
*/

#define GGOBI_TYPE_EXTENDED_DISPLAY	 (ggobi_extended_display_get_type ())
#define GGOBI_EXTENDED_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_EXTENDED_DISPLAY, extendedDisplayd))
#define GGOBI_EXTENDED_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_EXTENDED_DISPLAY, GGobiExtendedDisplayClass))
#define GGOBI_IS_EXTENDED_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_EXTENDED_DISPLAY))
#define GGOBI_IS_EXTENDED_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_EXTENDED_DISPLAY))
#define GGOBI_EXTENDED_DISPLAY_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_EXTENDED_DISPLAY, GGobiExtendedDisplayClass))

GType ggobi_extended_display_get_type();

typedef struct 
{
    GGobiWindowDisplayClass parent_class;

    gboolean supports_edges_p;  /* only true for scatterplots? */
    gboolean show_edges_p; /* used in splot_draw_to_pixmap0_unbinned by scatmat and scatterplot (only) */

    gboolean binning_ok; /* see binning_permitted in brush.c */
    gboolean (*binningPermitted)(displayd *dpy);

    gboolean allow_reorientation; /* see p1d_varsel for changing vertical/horizontal orientation */
    gboolean options_menu_p; /* whether this supports an option menu in the control panel. Default is yes! */


    gboolean loop_over_points; 	/* See splot_draw_to_pixmap0_unbinned. */


    gchar * treeLabel;
    gchar const * (*tree_label)(displayd *dpy);

    gchar * titleLabel;
    gchar const *  (*title_label)(displayd *dpy);

    displayd *(*create)(gboolean missing_p, splotd *sp, GGobiStage *d, GGobiSession *gg);
    displayd *(*createWithVars)(gboolean missing_p, gint nvars, gint *vars, GGobiStage *d, GGobiSession *gg);

    gboolean (*variable_select)(GtkWidget *, displayd *, splotd *, gint jvar, gint toggle, gint mouse, cpaneld *cpanel, GGobiSession *gg);

    gint  (*variable_plotted_p)(displayd *dpy, GSList *cols, GGobiStage *d);

    gboolean (*cpanel_set)(displayd *dpy, cpaneld *cp, GGobiSession *gg);

	const gchar *(*mode_ui_get)(displayd *dpy);
	
    void (*display_unset)(displayd *dpy);
    void (*display_set)(displayd *dpy, GGobiSession *gg);

    gboolean (*build_symbol_vectors)(cpaneld *, GGobiStage *, GGobiSession *);

    void (*ruler_ranges_set)(gboolean, displayd *, splotd *, GGobiSession *);

    void (*varpanel_refresh)(displayd *dpy, splotd *sp, GGobiStage *d);

  gboolean (*handles_projection)(displayd *dpy, ProjectionMode);
  gboolean (*handles_interaction)(displayd *dpy, InteractionMode);

	/* Probably should arrange for displayd to come first and no need to pass the splots. */
    void (*xml_describe)(xmlNodePtr node, GList *splots, displayd *dpy);

    void (*varpanel_tooltips_set)(displayd *dpy, GGobiSession *gg, GtkWidget *wx, GtkWidget *wy, GtkWidget *wz, GtkWidget *label);

    gint (*plotted_vars_get)(displayd *display, gint *cols, GGobiStage *d, GGobiSession *gg);

    GtkWidget *(*imode_control_box)(displayd *, gchar **modeName, GGobiSession *gg);

  GtkWidget *(*menus_make)(displayd *dpy, GGobiSession *gg);

  gboolean (*event_handlers_toggle)(displayd *dpy, splotd *sp, gboolean state, ProjectionMode, InteractionMode);

    gint (*splot_key_event_handler)(displayd *dpy, splotd *sp, gint keval);
  /* new - dfs */
  gint (*splot_key_event_handled)(GtkWidget *, displayd *, splotd *, GdkEventKey *, GGobiSession *);

    void (*add_plot_labels)(displayd *dpy, splotd *sp, GdkDrawable *, GGobiStage *, GGobiSession *);

  gboolean (*varpanel_highd)(displayd *dpy);

  void (*move_points_motion_cb)(displayd *, splotd *, GtkWidget *w, GdkEventMotion *event, GGobiSession *);
  void (*move_points_button_cb)(displayd *, splotd *, GtkWidget *w, GdkEventButton *event, GGobiSession *);

/* XXX duncan and dfs: you need to sort this out
    void (*world_to_raw)(displayd *, splotd *, gint, GGobiStage *, GGobiSession *);
*/

  /* time will tell which of these we need -- dfs */
  void (*viewmode_set)(displayd *, GGobiSession *);
  void (*pmode_set)(ProjectionMode, displayd *, GGobiSession *);
  /* */

  gboolean (*varcircle_draw)(displayd *, gint jvar, GdkPixmap *da_pix, GGobiSession *gg);
  void (*select_X)(GtkWidget *, displayd *, gint, GGobiSession *);

  void (*tour1d_realloc)(displayd *, gint, GGobiStage *);
  void (*tour2d3_realloc)(displayd *, gint, GGobiStage *);
  void (*tour2d_realloc)(displayd *, gint, GGobiStage *);
  void (*tourcorr_realloc)(displayd *, gint, GGobiStage *);

  void (*set_show_axes_option)(displayd *, gboolean);
  void (*set_show_axes_label_option)(displayd *, gboolean);
  void (*set_show_axes_values_option)(displayd *, gboolean);

} GGobiExtendedDisplayClass;


typedef struct {
   windowDisplayd dpy;
   gchar * titleLabel;
   GtkWidget *cpanelWidget;
} extendedDisplayd;

void display_set_values(displayd *display, GGobiStage *d, GGobiSession *gg);

/* These functions are no longer const, because this is dangerous, given
   that the input is a pointer and the data it points to could change,
   fooling the compiler - mfl */
const gchar * /*const*/ ggobi_display_tree_label(displayd *dpy);
const gchar * /*const*/ ggobi_display_title_label(displayd *dpy);


displayd *ggobi_display_new(gboolean missing_p, GGobiStage *d, GGobiSession *gg);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif
