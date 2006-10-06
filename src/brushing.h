/*-- brushing.h --*/
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

#ifndef BRUSH_H
#define BRUSH_H

#include <gtk/gtk.h>

#define NGLYPHTYPES 7  /* 0:6 */
#define NGLYPHSIZES 8  /* 0:7 */
#define NGLYPHS ((NGLYPHTYPES-1)*NGLYPHSIZES + 1)  /* 0:48 */

#define OPEN 0
#define FILL 1

#define MAXNCOLORS 15

/* br_mode */
#define BR_PERSISTENT 0
#define BR_TRANSIENT  1

/* br_point_targets and br_edge_targets */
typedef enum {
   br_off, br_candg, br_color, br_glyph, br_shadow, br_unshadow, br_exclude, br_include
} BrushTargetType;

/* for binning the screen */
#define BRUSH_NBINS  20
#define BRUSH_MARGIN 10
#define BINBLOCKSIZE 50


/*-- for edge brushing:  all sizes, 3 types for now --*/
#define NEDGETYPES 3
typedef enum {SOLID, WIDE_DASH, NARROW_DASH} EdgeType;
/*-- --*/

typedef enum {
  RESET_EXCLUDE_SHADOW_POINTS,
  RESET_INCLUDE_SHADOW_POINTS,
  RESET_UNSHADOW_POINTS,
  RESET_EXCLUDE_SHADOW_EDGES,
  RESET_INCLUDE_SHADOW_EDGES,
  RESET_UNSHADOW_EDGES,
  RESET_INIT_BRUSH,

  RESET_POINT_COLORS,  /* unused */
  RESET_POINT_GLYPHS,  /* unused */
  RESET_EDGE_COLORS,   /* unused */
  RESET_EDGE_TYPES    /* unused */
} BrushReset;

typedef struct {
    gint x1, y1, x2, y2;
} brush_coords;

/* these data types seem to be missing in gdk */
typedef struct {
  gint x, y;
  gint width, height;
} arcd; 
typedef struct {
  gint x, y;
  gint width, height;
} rectd; 

typedef struct {
  guint n, nhidden, nshown;
} symbol_cell;

typedef struct {
  glong n, nshown, nhidden;
  gboolean hidden_p;
  gint glyphtype, glyphsize;
  gshort color;
} clusterd;
typedef struct {
  GtkWidget *da;
  /*-- buttons and labels for hide, show, complement --*/
  GtkWidget *h_btn;  /* hide */
  GtkWidget *nh_lbl, *ns_lbl, *n_lbl;
} clusteruid;

/* structure for binning the plot window */
typedef struct {
  gulong *els;
  gint nels;
  gint nblocks;  /* how many blocks have been allocated for els */
} bin_struct;

/* row groups */
typedef struct {
  gint id, nels, *els;
  gboolean sampled;   /* for subsetting */
} groupd;

#endif
