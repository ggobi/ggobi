/*-- brushing.h --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
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
typedef enum {br_off, br_candg, br_color, br_glyph, br_hide, br_select} BrushTargetType;
/* for binning the screen */
#define BRUSH_NBINS  20
#define BRUSH_MARGIN 10
#define BINBLOCKSIZE 50
/* br_linkby */
#define BR_LINKBYID   0
#define BR_LINKBYVAR  1
/* */

/*-- for edge brushing:  all sizes, 3 types for now --*/
#define NEDGETYPES 3
typedef enum {SOLID, WIDE_DASH, NARROW_DASH} EdgeType;
/*-- --*/

typedef enum { RESET_UNHIDE_POINTS, RESET_UNHIDE_EDGES, BRUSH_RESET_SCALE,
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
  gboolean excluded_p;
  gint glyphtype, glyphsize;
  gshort color;
} clusterd;
typedef struct {
  GtkWidget *da;
  /*-- buttons and labels for hide, show, complement --*/
/* GtkWidget *h_btn, *s_btn, *c_btn; */
  GtkWidget *h_btn, *e_btn;  /* hide, exclude */
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
