/*-- win32_draw.c --*/
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

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

#include "scatterplotClass.h"
#include "parcoordsClass.h"
#include "tsdisplay.h"


/*
 * Batched drawing:   I may need to continue drawing this
 * way to achieve adequate speed in Windows.
 *
 * ... But I see I'm only able to do batched drawing for
 * plusses, x'es and points.  That's not very interesting.
*/

#ifdef WIN32

static void drawing_arrays_alloc (splotd *sp, GGobiStage *d, GGobiSession *gg);
static void build_circle (icoords *, gint, GdkRectangle *, gint, gshort);
static void build_plus (icoords *, gint, GdkSegment *, gint, gshort);
static void build_rect (icoords *, gint, GdkRectangle *, gint, gshort);
static void build_x (icoords *, gint, GdkSegment *, gint, gshort);
static void build_whisker_segs (gint j, gint *nwhisker_segs, splotd *sp);
static void build_ash_segs (gint, gint *nsegs, splotd *sp);

/*
 * ... just noticed these:  should they become part of the splotd
 * instead of sitting here as statics?  I think so.
 * 
static gint maxn = 0;
static GdkPoint   *points;
static GdkSegment *segs;
static GdkSegment *whisker_segs;
static GdkSegment *ash_segs;
static GdkRectangle      *open_rects;
static GdkRectangle      *filled_rects;
static GdkRectangle       *open_arcs;
static GdkRectangle       *filled_arcs;
*/

static void
drawing_arrays_alloc (splotd *sp, GGobiStage *d, GGobiSession *gg) {
  gint n = d->n_rows

  if (sp->win32.npoints == 0) {
    sp->win32.points = (GdkPoint *) g_malloc (n * sizeof (GdkPoint));
    sp->win32.segs = (GdkSegment *) g_malloc (2 * n * sizeof (GdkSegment));
    sp->win32.whisker_segs = (GdkSegment *) g_malloc (2*n*sizeof (GdkSegment));
    sp->win32.ash_segs = (GdkSegment *) g_malloc (n * sizeof (GdkSegment));
    sp->win32.open_rects = (GdkRectangle *) g_malloc (n * sizeof (GdkRectangle));
    sp->win32.filled_rects = (GdkRectangle *) g_malloc (n * sizeof (GdkRectangle));
    sp->win32.open_arcs = (GdkRectangle *) g_malloc (n * sizeof (GdkRectangle));
    sp->win32.filled_arcs = (GdkRectangle *) g_malloc (n * sizeof (GdkRectangle));
  } else {
    sp->win32.points = (GdkPoint *)
      g_realloc (sp->win32.points, n * sizeof (GdkPoint));
    sp->win32.segs = (GdkSegment *)
      g_realloc (sp->win32.segs, 2 * n * sizeof (GdkSegment));
    sp->win32.whisker_segs = (GdkSegment *)
      g_realloc (sp->win32.whisker_segs, 2 * n * sizeof (GdkSegment));
    sp->win32.ash_segs = (GdkSegment *)
      g_realloc (sp->win32.ash_segs, n * sizeof (GdkSegment));
    sp->win32.open_rects = (GdkRectangle *)
      g_realloc (sp->win32.open_rects, n * sizeof (GdkRectangle));
    sp->win32.filled_rects = (GdkRectangle *)
      g_realloc (sp->win32.filled_rects, n * sizeof (GdkRectangle));
    sp->win32.open_arcs = (GdkRectangle *)
      g_realloc (sp->win32.open_arcs, n * sizeof (GdkRectangle));
    sp->win32.filled_arcs = (GdkRectangle *)
      g_realloc (sp->win32.filled_arcs, n * sizeof (GdkRectangle));
  }
  sp->win32.npoints = n;
}

void
win32_drawing_arrays_free (splotd *sp){
  if (sp->win32.npoints) {
    g_free ((gpointer) sp->win32.points);
    g_free ((gpointer) sp->win32.segs);
    g_free ((gpointer) sp->win32.whisker_segs);
    g_free ((gpointer) sp->win32.ash_segs);
    g_free ((gpointer) sp->win32.open_rects);
    g_free ((gpointer) sp->win32.filled_rects);
    g_free ((gpointer) sp->win32.open_arcs);
    g_free ((gpointer) sp->win32.filled_arcs);
  }
}

void
win32_draw_arcs (GdkDrawable *drawable,
                 GdkGC       *gc,
                 gint         filled,
                 GdkRectangle         *arcs,
                 gint         narcs)
{
  gint i;
  for (i=0; i<narcs; i++) {
    gdk_draw_arc (drawable, gc, false,
      arcs[i].x, arcs[i].y,
      arcs[i].width, arcs[i].height, 0, (gshort) 23040);
    if (filled)
      gdk_draw_arc (drawable, gc, filled,
        arcs[i].x, arcs[i].y,
        arcs[i].width+1, arcs[i].height, 0, (gshort) 23040);
  }
}

void
win32_draw_rectangles (GdkDrawable *drawable,
                       GdkGC       *gc,
                       gint         filled,
                       GdkRectangle        *rects,
                       gint         nrects)
{
  gint i;
  for (i=0; i<nrects; i++)
    gdk_draw_rectangle (drawable, gc, filled,
      rects[i].x, rects[i].y, rects[i].width, rects[i].height);
}

void
build_plus (icoords *pos, gint nrow, GdkSegment *segv, gint nplus, gshort size)
{
  gshort x = (gshort) pos[nrow].x;

/*
  gboolean append;

  append = (sp->win32->segs.len < nplus+1);
  segp = (append) ? (GdkSegment *) g_malloc (sizeof (GdkSegment)) :
                    (GdkSegment *) &sp->win32->segs.data[nplus];
  segp->x1 = x - size;
  segp->x2 = x + size;
  segp->y1 = segp->y2 = (gshort) pos[nrow].y;
  if (append) g_array_append_val (sp->win32->segs, segp);

  nplus++;
  append = (sp->win32->segs.len < nplus+1);
  segp = (append) ? (GdkSegment *) g_malloc (sizeof (GdkSegment)) :
                    (GdkSegment *) &sp->win32->segs.data[nplus];
  segp->x1 = segp->x2 = x;
  segp->y1 = (gshort) pos[nrow].y - size;
  segp->y2 = (gshort) pos[nrow].y + size;
  if (append) g_array_append_val (sp->win32->segs, segp);
*/

  segv[nplus].x1 = x - size;
  segv[nplus].x2 = x + size;
  segv[nplus].y1 = segv[nplus].y2 = (gshort) pos[nrow].y;
  nplus++;
  segv[nplus].x1 = segv[nplus].x2 = x;
  segv[nplus].y1 = (gshort) pos[nrow].y - size;
  segv[nplus].y2 = (gshort) pos[nrow].y + size;
}

void
build_x (icoords *pos, gint nrow, GdkSegment *segv, gint nx, gshort size)
{
  gshort x;
  x = (gshort) pos[nrow].x;
  segv[nx].x1 = x - size;
  segv[nx].x2 = x + size;
  segv[nx].y1 = (gshort) pos[nrow].y - size;
  segv[nx].y2 = (gshort) pos[nrow].y + size;
  nx++;
  segv[nx].x1 = x + size;
  segv[nx].x2 = x - size;
  segv[nx].y1 = pos[nrow].y - size;
  segv[nx].y2 = pos[nrow].y + size;
}

void
build_circle (icoords *pos, gint nrow, GdkRectangle *circv, gint ncirc, gshort size)
{
  circv[ncirc].x = (gushort) (pos[nrow].x - size);
  circv[ncirc].y = (gushort) (pos[nrow].y - size);
  circv[ncirc].width = circv[ncirc].height = 2*size;
}

void
build_rect (icoords *pos, gint nrow, GdkRectangle * rectv, gint nrect, gshort size)
{
  rectv[nrect].x = (gushort) (pos[nrow].x - size);
  rectv[nrect].y = (gushort) (pos[nrow].y - size);
  rectv[nrect].width = rectv[nrect].height = 2*(size+1);
}

void
build_point (icoords *pos, gint nrow, GdkPoint * pointv, gint npt)
{
  pointv[npt].x = (gushort) (pos[nrow].x);
  pointv[npt].y = (gushort) (pos[nrow].y);
}

void
build_glyph (glyphd gl, icoords *xypos, gint jpos,
  GdkPoint *pointv,   gint *np,
  GdkSegment *segv,   gint *ns,
  GdkRectangle *openrectv,   gint *nr_open,
  GdkRectangle *filledrectv, gint *nr_filled,
  GdkRectangle *openarcv,     gint *nc_open,
  GdkRectangle *filledarcv,   gint *nc_filled)
{
  gshort size, type;
  size = gl.size + 1;
  type = gl.type;

  switch (type) {
    case PLUS:
      build_plus (xypos, jpos, segv, *ns, size);
      *ns = *ns+2;
    break;
    case X:
      build_x (xypos, jpos, segv, *ns, size);
      *ns = *ns+2;
    break;
    case OR:
      build_rect (xypos, jpos, openrectv, *nr_open, size);
      (*nr_open)++;
    break;
    case FR:
      build_rect (xypos, jpos, filledrectv, *nr_filled, size);
      (*nr_filled)++;
    break;
    case OC:
      build_circle (xypos, jpos, openarcv, *nc_open, size);
      (*nc_open)++;
    break;
    case FC:
      build_circle (xypos, jpos, filledarcv, *nc_filled, size);
      (*nc_filled)++;
    break;
    case DOT_GLYPH:
      build_point (xypos, jpos, pointv, *np);
      (*np)++;
    break;
    case UNKNOWN_GLYPH:
    break;
    default:
      g_printerr ("build_glyph: impossible glyph type %d\n", type);
  }
}

void
build_whisker_segs (gint j, gint *nwhisker_segs, splotd *sp) 
{
  displayd *display = (displayd *) sp->displayptr;
  gint n;
  if (GGOBI_IS_PAR_COORDS_DISPLAY(display)) {
    n = 2*j;
    sp->win32.whisker_segs[*nwhisker_segs].x1 = sp->whiskers[n].x1;
    sp->win32.whisker_segs[*nwhisker_segs].y1 = sp->whiskers[n].y1;
    sp->win32.whisker_segs[*nwhisker_segs].x2 = sp->whiskers[n].x2;
    sp->win32.whisker_segs[*nwhisker_segs].y2 = sp->whiskers[n].y2;
    n++;
    *nwhisker_segs += 1;
    sp->win32.whisker_segs[*nwhisker_segs].x1 = sp->whiskers[n].x1;
    sp->win32.whisker_segs[*nwhisker_segs].y1 = sp->whiskers[n].y1;
    sp->win32.whisker_segs[*nwhisker_segs].x2 = sp->whiskers[n].x2;
    sp->win32.whisker_segs[*nwhisker_segs].y2 = sp->whiskers[n].y2;
    *nwhisker_segs += 1;
  }
  else if (GGOBI_IS_TIME_SERIES_DISPLAY(display)) {
    sp->win32.whisker_segs[*nwhisker_segs].x1 = sp->whiskers[j].x1;
    sp->win32.whisker_segs[*nwhisker_segs].y1 = sp->whiskers[j].y1;
    sp->win32.whisker_segs[*nwhisker_segs].x2 = sp->whiskers[j].x2;
    sp->win32.whisker_segs[*nwhisker_segs].y2 = sp->whiskers[j].y2; 
    *nwhisker_segs += 1; 
  }
}

void
build_ash_segs (gint i, gint *nsegs, splotd *sp)
{
  displayd *display = (displayd *) sp->displayptr;

  if (display->p1d_orientation == HORIZONTAL) {
    sp->win32.ash_segs[*nsegs].x1 = sp->screen[i].x;
    sp->win32.ash_segs[*nsegs].y1 = sp->screen[i].y;
    sp->win32.ash_segs[*nsegs].x2 = sp->screen[i].x;
    sp->win32.ash_segs[*nsegs].y2 = sp->p1d.ash_baseline.y;
  } else {
    sp->win32.ash_segs[*nsegs].x1 = sp->screen[i].x;
    sp->win32.ash_segs[*nsegs].y1 = sp->screen[i].y;
    sp->win32.ash_segs[*nsegs].x2 = sp->p1d.ash_baseline.x;
    sp->win32.ash_segs[*nsegs].y2 = sp->screen[i].y;
  }

  *nsegs += 1;
}

static void
draw_glyphs (splotd *sp, GdkDrawable *drawable,
  GdkPoint *points,    gint np,
  GdkSegment *segs,    gint ns,
  GdkRectangle *open_rects,   gint nr_open,
  GdkRectangle *filled_rects, gint nr_filled,
  GdkRectangle *open_arcs,     gint nc_open,
  GdkRectangle *filled_arcs,   gint nc_filled,
  GGobiSession *gg)
{
  if (gg->plot_GC == NULL)
    init_plot_GC (drawable, gg);

  if (np)
    gdk_draw_points (drawable, gg->plot_GC, points, np);
  if (ns)
    gdk_draw_segments (drawable, gg->plot_GC, segs, ns);

  if (nr_open)
    win32_draw_rectangles (drawable, gg->plot_GC, OPEN, open_rects, nr_open);
  if (nr_filled)
    win32_draw_rectangles (drawable, gg->plot_GC, FILL, filled_rects, nr_filled);

  if (nc_open)
    win32_draw_arcs (drawable, gg->plot_GC, OPEN, open_arcs, nc_open);
  if (nc_filled)
    win32_draw_arcs (drawable, gg->plot_GC, FILL, filled_arcs, nc_filled);
}

void
win32_draw_to_pixmap_unbinned (gint current_color, splotd *sp, gboolean draw_hidden, GGobiSession *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  GGobiStage *d = display->d;

  gint i, m;
  gint npt, nseg, nr_open, nr_filled, nc_open, nc_filled;
  gint nwhisker_segs = 0;
  gint nash_segs = 0;
#if OLD
  gint dtype = display->displaytype; 
#endif
  npt = nseg = nr_open = nr_filled = nc_open = nc_filled = 0;

  if (sp->win32.npoints < d->n_rows
    drawing_arrays_alloc (sp, d, gg);

  GGOBI_STAGE_ATTR_INIT_ALL(d);  
  for (i=0; i<d->n_rows; i++) {
    if (splot_plot_case (i, d, sp, display, gg)) {
      if ((draw_hidden && GGOBI_STAGE_GET_ATTR_HIDDEN(d, i)) ||  /*-- drawing hiddens --*/
         (GGOBI_STAGE_GET_ATTR_COLOR(d, i) == current_color &&   /*-- drawing unhiddens --*/
              !draw_hidden && !GGOBI_STAGE_GET_ATTR_HIDDEN(d, i)))
      {
        if (display->options.points_show_p) {
          build_glyph (GGOBI_STAGE_GET_ATTR_GLYPH(d, i), sp->screen, i,
            sp->win32.points, &npt,         sp->win32.segs, &nseg,
            sp->win32.open_rects, &nr_open, sp->win32.filled_rects, &nr_filled,
            sp->win32.open_arcs, &nc_open,  sp->win32.filled_arcs, &nc_filled);

          if ((GGOBI_IS_PAR_COORDS_DISPLAY(display) ||
               GGOBI_IS_TIME_SERIES_DISPLAY(display)) &&
              display->options.whiskers_show_p)
          {
            build_whisker_segs (m, &nwhisker_segs, sp);
          } else if (GGOBI_IS_SCATTERPLOT_DISPLAY(display) &&
            ((pmode_get(display, gg) == TOUR1D && cpanel->t1d.ASH_add_lines_p) ||
             (pmode_get(display, gg) == P1PLOT && cpanel->p1d.type == ASH &&
              cpanel->p1d.ASH_add_lines_p))) {
            build_ash_segs (i, &nash_segs, sp);
          }
        }
      }
    }
  }
  if (nwhisker_segs)
    gdk_draw_segments (sp->pixmap0, gg->plot_GC,
      sp->win32.whisker_segs, nwhisker_segs);
  if (nash_segs) {
    gdk_draw_segments (sp->pixmap0, gg->plot_GC, sp->win32.ash_segs, nash_segs);
  }
  draw_glyphs (sp, sp->pixmap0,
    sp->win32.points, npt,           sp->win32.segs, nseg,
    sp->win32.open_rects, nr_open,   sp->win32.filled_rects, nr_filled,
    sp->win32.open_arcs, nc_open,    sp->win32.filled_arcs, nc_filled,
    gg);
}

void
win32_draw_to_pixmap_binned (icoords *bin0, icoords *bin1,
  gint current_color, splotd *sp, gboolean draw_hidden, GGobiSession *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  GGobiStage *d = display->d;
  gint ih, iv;
  gint m, j;
  gint npt, nseg, nr_open, nr_filled, nc_open, nc_filled;
  gint nwhisker_segs = 0;
  npt = nseg = nr_open = nr_filled = nc_open = nc_filled = 0;

  GGOBI_STAGE_ATTR_INIT_ALL(d);  
  for (ih=bin0->x; ih<=bin1->x; ih++) {
    for (iv=bin0->y; iv<=bin1->y; iv++) {
      for (m=0; m<d->brush.binarray[ih][iv].nels; m++) {
        j = d->brush.binarray[ih][iv].els[m];
        if (splot_plot_case (j, d, sp, display, gg)) {
          if ((draw_hidden && GGOBI_STAGE_GET_ATTR_HIDDEN(d, j)) ||  /*-- hiddens --*/
             (GGOBI_STAGE_GET_ATTR_COLOR(d, j) == current_color &&   /*-- unhiddens --*/
                  !draw_hidden && !GGOBI_STAGE_GET_ATTR_HIDDEN(d, j)))
          {
            build_glyph (GGOBI_STAGE_GET_ATTR_GLYPH(d, j), sp->screen, j,
              sp->win32.points, &npt,          
              sp->win32.segs, &nseg,
              sp->win32.open_rects, &nr_open,  
              sp->win32.filled_rects, &nr_filled,
              sp->win32.open_arcs, &nc_open,   
              sp->win32.filled_arcs, &nc_filled);

            if (GGOBI_IS_PAR_COORDS_DISPLAY(display) ||
                GGOBI_IS_TIME_SERIES_DISPLAY(display))
            {
              build_whisker_segs (j, &nwhisker_segs, sp);
            }
          }
        }
      }
    }
  }
  gdk_draw_segments (sp->pixmap0, gg->plot_GC,
    sp->win32.whisker_segs, nwhisker_segs);
  draw_glyphs (sp, sp->pixmap0,
    sp->win32.points, npt,           sp->win32.segs, nseg,
    sp->win32.open_rects, nr_open,   sp->win32.filled_rects, nr_filled,
    sp->win32.open_arcs, nc_open,    sp->win32.filled_arcs, nc_filled,
    gg);
}
#endif
