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

/* cluster; to be used in Group Exclusion tool */
typedef struct {
  glong n;  /*-- Can I know the number of elements in this cluster? --*/
  gint glyphtype, glyphsize;
  gshort color;
  gboolean hidden, included;
  GtkWidget *da, *lbl, *hide_tgl, *exclude_tgl;
} clusterd;

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
  gboolean included;  /* for linked brushing */
  gboolean sampled;   /* for subsetting */
} rgroupd;

#endif
