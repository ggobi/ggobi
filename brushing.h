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

#define NGLYPHTYPES 7
#define NGLYPHSIZES 8
#define NGLYPHS ((NGLYPHTYPES-1)*NGLYPHSIZES + 1)

#define PLUS_GLYPH       1
#define X_GLYPH          2
#define OPEN_RECTANGLE   3
#define FILLED_RECTANGLE 4
#define OPEN_CIRCLE      5
#define FILLED_CIRCLE    6
#define POINT_GLYPH      7

#define OPEN 0
#define FILL 1

#define NCOLORS 10


/*
 * brushing
*/
/* br_scope */
#define BR_POINTS 0
#define BR_LINES  1
#define BR_PANDL  2  /* points and lines */
/* br_mode */
#define BR_PERSISTENT 0
#define BR_TRANSIENT  1
/* br_target */
#define BR_CANDG 0  /* color and glyph */
#define BR_COLOR 1
#define BR_GLYPH 2  /*-- glyph type and size --*/
#define BR_GSIZE 3  /*-- glyph size only --*/
#define BR_HIDE  4
/* for binning the screen */
#define BRUSH_NBINS  20
#define BRUSH_MARGIN 10
#define BINBLOCKSIZE 50
/* */

typedef enum {PLUS=1, X, OR, FR, OC, FC, DOT,UNKNOWN_GLYPH} GlyphType;

typedef enum { RESET_UNHIDE_POINTS, RESET_POINT_COLORS, RESET_GLYPHS, 
               RESET_UNHIDE_LINES, RESET_LINES, RESET_INIT_BRUSH} BrushReset;

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
  gint glyphtype, glyphsize;
  gshort color;
} clusterd;
typedef struct {
  GtkWidget *da;
  /*-- buttons and labels for hide, show, complement --*/
  GtkWidget *h_btn, *s_btn, *c_btn;
  GtkWidget *nh_lbl, *ns_lbl, *n_lbl;
} clusteruid;

/* glyph vectors */
typedef struct {
  gint type;
  gint size;
} glyphv;

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
} rgroupd;

#endif
