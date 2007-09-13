/*-- types.h --*/
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

#ifndef TYPES_H
#define TYPES_H

#include <gtk/gtk.h>
#include "array.h"
#include "vector.h"

typedef struct {
  GtkWidget *w;
  gchar *name;
} modepaneld;

/*
 * greal is at several steps in the pipeline, in particular
 * for world, and planar, as we eliminate the conversion
 * to longs.  Defining 'greal' allows us to compare the behavior
 * of floats and doubles for speed and storage.
*/
typedef enum {Sprocess_data, xml_data, mysql_data, url_data, csv_data, unknown_data, num_data_modes} DataMode;

typedef enum {NONE, EXPOSE, QUICK, BINNED, FULL, FULL_1PIXMAP} RedrawStyle;

/* see varpanel_ui.c and especially varpanel_names. */
enum {VARSEL_X, VARSEL_Y, VARSEL_Z, VARSEL_LABEL}; 

/* For use in the sticky_point_added and sticky_point_removed events. */
typedef enum {STICKY, UNSTICKY} PointIdentifyState;

typedef enum {DOT_GLYPH=0, PLUS, X, OC, OR, FC, FR, UNKNOWN_GLYPH} GlyphType;

typedef enum {ADDING_EDGES=0, ADDING_POINTS} eeMode;

typedef enum {
  IMP_RANDOM,  IMP_FIXED, IMP_BELOW, IMP_ABOVE, IMP_MEAN, IMP_MEDIAN
} ImputeType;

typedef struct {
  GlyphType type;
  gint size;
} glyphd;
typedef struct {
  glong x, y;
} lcoords;
typedef struct {
  greal x, y;
} gcoords;
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
  /* 
   * by default, jpartner = -1, but if this edge is one of a
   * bidirectional pair, jpartner is the index of the edge going
   * in the other direction.
  */
  gint jpartner;
} endpointsd;

/* The symbolic edge description which keeps the endpoints as record ids */
typedef gchar *RecordKey;
typedef struct {
  RecordKey a;
  RecordKey b;
  gint jpartner;
} SymbolicEndpoints;
typedef struct {
  gchar *a;
  gchar *b;
  gint jcase;
} SortableEndpoints;
typedef struct {
  endpointsd *endpoints;
  GObject *data; /* GGobiStage pointer*/
} DatadEndpoints;




typedef struct {  /*-- used for obtaining ranks --*/
  gfloat f;
  gint indx;
} paird;


/*-- used to keep track of history in moving points --*/
typedef struct {
  gint i, j;
  gfloat val;
} celld;

typedef gint (*Tour_PPIndex_f)(array_f *pd, void *params, gfloat *val, gpointer userData);

typedef struct {
    gchar *ppIndexName;   /* a string that can be used in the GUI to describe this PP index. */
    Tour_PPIndex_f index_f; /* The C routine that calculates the PP index value.*/
    gboolean checkGroups; /* Whether we have to call compute_groups and calculate the index only if this returns false(). */
    gpointer userData;    /* arbitrary data object that is passed in the call to index_f to parameterize it. */
} TourPPIndex;

/*-- tour elements --*/
typedef struct {
  gint datadim, projdim;
  /*
   * the variables that are in the current subset, and represented by
   * circles or rectangles in the right-hand pane.
  */
  gint nsubset;
  vector_i subset_vars;
  vector_b subset_vars_p;
  /*
   * Of the variables in the current subset, these are the variables
   * that are currently touring.  Their meaning remains the same
   * despite the variable selection panel redesign.
  */
  gint nactive;
  vector_i active_vars;
  vector_b active_vars_p;
  /* */
  array_d Fa, Fz, F, Ga, Gz, G, Va, Vz, tv;
  vector_f lambda, tau, tinc;
  gfloat dist_az, delta, tang;
  gint target_selection_method;
  gint idled;
  gboolean get_new_target;
  gint index; /* this is for counting planes passed */
  gfloat ppval, oppval; /* for projection pursuit */

} tour;

typedef struct {
  vector_i ngroup, group; /* for class indices */
  gint numgroups; /* previously called groups in class code */
  array_d cov, tcov, mean; /* for lda, holes, cm */
  vector_d ovmean; /* for lda, holes, cm */
  vector_i nright, index; /* for gini, entropy */
  vector_d x; /* for gini, entropy */
} pp_param;

typedef struct
{ gfloat temp_start, temp_end, cooling, heating, temp, index_best;
  gint restart, maxproj, success;
  array_f proj_best, data, pdata;
} optimize0_param; 

/*
 * display options
*/
typedef struct {
 gboolean points_show_p;              /* scatterplot, scatmat, parcoords */
 gboolean axes_show_p;                /* scatterplot, scatmat, parcoords */
 gboolean axes_label_p;               /* scatterplot; tour */
 gboolean axes_values_p;              /* scatterplot; tour */
 
 gboolean edges_undirected_show_p;    /* scatterplot */
 gboolean edges_arrowheads_show_p;    /* scatterplot */
 gboolean edges_directed_show_p;      /* scatterplot: both of the above */

 gboolean whiskers_show_p;            /* parcoords, time series */

/* unused
 gboolean missings_show_p;            * scatterplot, scatmat, parcoords *
 gboolean axes_center_p;              * scatterplot *
 gboolean double_buffer_p;            * parcoords *
 gboolean link_p;                     * scatterplot, scatmat, parcoords *
*/
} DisplayOptions;

extern DisplayOptions DefaultDisplayOptions;

/* bin struct for bins in Barcharts, histograms, ... */
typedef struct
{
  glong count, nhidden;  /* dfs: count includes nhidden */
  gint index;
  gint value;  /* dfs; restricting index to non-categorical variables */
  gcoords planar;
  GdkRectangle rect;
} gbind;
/* end bind */

/* structure for a barchart, in splotd only a pointer to this structure is made to save memory */
typedef struct
{
 gboolean is_histogram;  /* true if variable not categorical */
 gboolean is_spine;      /* false by default */
 gint nbins;
 gint new_nbins;

 gint ncolors;
 gint maxbincounts;
 gbind  *bins;
 gbind  **cbins;
 gfloat *breaks;

 GdkPoint anchor_rgn[4];
 GdkPoint offset_rgn[4];
 gboolean anchor_drag;
 gboolean width_drag;
 gfloat offset;

/* whenever points in histograms "overflow" to the left or right of the first or last bin, they need a place to stay */
 gboolean high_pts_missing;
 gboolean low_pts_missing;
 gbind *high_bin;
 gbind *low_bin;
 gbind *col_high_bin;
 gbind *col_low_bin;

/* identify bars */
 gboolean *bar_hit;
 gboolean *old_bar_hit;
 gboolean same_hits;
 gint old_nbins;

 vector_i index_to_rank;
} barchartd;

#define TYPES_H

#endif
