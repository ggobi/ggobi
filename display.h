/*-- display.h --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#ifndef DISPLAY_H
#define DISPLAY_H

#include "defines.h"
#include "cpanel.h"
#include "splot.h"
#include "datad.h"

#include "ggobi.h"

struct _ggobid; 


#define GTK_TYPE_GGOBI_DISPLAY	 (gtk_ggobi_display_get_type ())
#define GTK_GGOBI_DISPLAY(obj)	 (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_DISPLAY, displayd))
#define GTK_GGOBI_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_DISPLAY, GtkGGobiDisplayClass))
#define GTK_IS_GGOBI_DISPLAY(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_DISPLAY))
#define GTK_IS_GGOBI_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_DISPLAY))

GtkType gtk_ggobi_display_get_type();
displayd *gtk_ggobi_display_new(gint type, gboolean missing_p, datad *d, ggobid *gg);

typedef struct _GtkGGobiDisplayClass
{
    GtkVBoxClass parent_class;

} GtkGGobiDisplayClass;

/**

 */
struct _displayd {
  GtkVBox vbox;

/*
 * Used by all displays
*/
 enum displaytyped displaytype;


   /*-- for scatterplots, where edge menus need to be rebuilt on the fly --*/
 GtkWidget *menubar;
 GtkWidget *edge_item, *edge_menu;

 cpaneld cpanel;

 GList *splots;          /*-- doubly linked list of splots --*/
 splotd *current_splot;  /*-- multi-plot displays need this notion --*/

 datad *d;  /*-- pointer to a particular gg->d[] --*/
 datad *e;  /*-- pointer to a particular gg->d[] --*/

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
 GList *scatmat_cols, *scatmat_rows;
 GtkWidget *table;


/*
 * Parallel coordinates display
*/
  gint p1d_orientation;

/*
 * Tour display
  gint tour_idled;
*/

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

/*
 * 2d tour
*/
 tour t2d;
 gboolean t2d_axes;

/*
 * 1d tour
*/
 tour t1d;
 gboolean t1d_axes;

/*
 * corr tour
*/
 tour tcorr1, tcorr2;
 gboolean tcorr_axes;

/* projection pursuit */
 GtkWidget *t1d_pplabel, *t2d_pplabel;
 GtkWidget *t1d_ppda, *t2d_ppda;
 GdkPixmap *t1d_pp_pixmap, *t2d_pp_pixmap;
 GtkWidget *t1d_window, *t2d_window;
 GtkWidget *t1d_control_frame, *t2d_control_frame;
 GtkWidget *t1d_mbar, *t2d_mbar;
 GtkAccelGroup *t1d_pp_accel_group, *t2d_pp_accel_group;
 optimize0_param t1d_pp_op, t2d_pp_op;
 gint t2d_pp_indx, t1d_pp_indx;
 gfloat t2d_ppindx_mat[100], t1d_ppindx_mat[100];
 gfloat t2d_indx_min, t2d_indx_max, t1d_indx_min, t1d_indx_max;
 gint t2d_ppindx_count, t1d_ppindx_count;

 struct _ggobid *ggobi;

};  /* displayd; */


#define GTK_TYPE_GGOBI_WINDOW_DISPLAY	 (gtk_ggobi_window_display_get_type ())
#define GTK_GGOBI_WINDOW_DISPLAY(obj)	 (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_WINDOW_DISPLAY, windowDisplayd))
#define GTK_GGOBI_WINDOW_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_WINDOW_DISPLAY, GtkGGobiWindowDisplayClass))
#define GTK_IS_GGOBI_WINDOW_DISPLAY(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_WINDOW_DISPLAY))
#define GTK_IS_GGOBI_WINDOW_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_WINDOW_DISPLAY))

GtkType gtk_ggobi_window_display_get_type();
displayd *gtk_ggobi_window_display_new(gint type, gboolean missing_p, datad *d, ggobid *gg);

typedef struct 
{
    GtkGGobiDisplayClass parent_class;

} GtkGGobiWindowDisplayClass;


typedef struct _windowDisplayd {
 displayd dpy;

 GtkWidget *window;
} windowDisplayd;

gboolean isEmbeddedDisplay(displayd *dpy);



/**
 This is used as a trivial class for its type information so that we can detect whether 
 we have one of the new style classes.
 We might remove it when we are finished the construction.
*/

#define GTK_TYPE_GGOBI_EXTENDED_DISPLAY	 (gtk_ggobi_extended_display_get_type ())
#define GTK_GGOBI_EXTENDED_DISPLAY(obj)	 (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_EXTENDED_DISPLAY, extendedDisplayd))
#define GTK_GGOBI_EXTENDED_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_EXTENDED_DISPLAY, GtkGGobiExtendedDisplayClass))
#define GTK_IS_GGOBI_EXTENDED_DISPLAY(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_EXTENDED_DISPLAY))
#define GTK_IS_GGOBI_EXTENDED_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_EXTENDED_DISPLAY))

GtkType gtk_ggobi_extended_display_get_type();

typedef struct 
{
    GtkGGobiWindowDisplayClass parent_class;

    gchar const * treeLabel;
    gchar * const (*tree_label)(displayd *dpy);

} GtkGGobiExtendedDisplayClass;


typedef struct {

   windowDisplayd dpy;
   
} extendedDisplayd;

void display_set_values(displayd *display, enum displaytyped type, datad *d, ggobid *gg);

gchar * const gtk_display_tree_label(displayd *dpy);

#endif
