/*-- win32_draw.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
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

static void drawing_arrays_alloc (splotd *sp, datad *d, ggobid *gg);
static void build_circle (icoords *, gint, arcd *, gint, gshort);
static void build_plus (icoords *, gint, GdkSegment *, gint, gshort);
static void build_rect (icoords *, gint, rectd *, gint, gshort);
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
static rectd      *open_rects;
static rectd      *filled_rects;
static arcd       *open_arcs;
static arcd       *filled_arcs;
*/

static void
drawing_arrays_alloc (splotd *sp, datad *d, ggobid *gg) {
  gint n = d->nrows;

  if (sp->win32.npoints == 0) {
    sp->win32.points = (GdkPoint *) g_malloc (n * sizeof (GdkPoint));
    sp->win32.segs = (GdkSegment *) g_malloc (2 * n * sizeof (GdkSegment));
    sp->win32.whisker_segs = (GdkSegment *) g_malloc (2*n*sizeof (GdkSegment));
    sp->win32.ash_segs = (GdkSegment *) g_malloc (n * sizeof (GdkSegment));
    sp->win32.open_rects = (rectd *) g_malloc (n * sizeof (rectd));
    sp->win32.filled_rects = (rectd *) g_malloc (n * sizeof (rectd));
    sp->win32.open_arcs = (arcd *) g_malloc (n * sizeof (arcd));
    sp->win32.filled_arcs = (arcd *) g_malloc (n * sizeof (arcd));
  } else {
    sp->win32.points = (GdkPoint *)
      g_realloc (sp->win32.points, n * sizeof (GdkPoint));
    sp->win32.segs = (GdkSegment *)
      g_realloc (sp->win32.segs, 2 * n * sizeof (GdkSegment));
    sp->win32.whisker_segs = (GdkSegment *)
      g_realloc (sp->win32.whisker_segs, 2 * n * sizeof (GdkSegment));
    sp->win32.ash_segs = (GdkSegment *)
      g_realloc (sp->win32.ash_segs, n * sizeof (GdkSegment));
    sp->win32.open_rects = (rectd *)
      g_realloc (sp->win32.open_rects, n * sizeof (rectd));
    sp->win32.filled_rects = (rectd *)
      g_realloc (sp->win32.filled_rects, n * sizeof (rectd));
    sp->win32.open_arcs = (arcd *)
      g_realloc (sp->win32.open_arcs, n * sizeof (arcd));
    sp->win32.filled_arcs = (arcd *)
      g_realloc (sp->win32.filled_arcs, n * sizeof (arcd));
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
                 arcd         *arcs,
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
        arcs[i].width, arcs[i].height, 0, (gshort) 23040);
  }
}

void
win32_draw_rectangles (GdkDrawable *drawable,
                       GdkGC       *gc,
                       gint         filled,
                       rectd        *rects,
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
build_circle (icoords *pos, gint nrow, arcd *circv, gint ncirc, gshort size)
{
  circv[ncirc].x = (gushort) (pos[nrow].x - size);
  circv[ncirc].y = (gushort) (pos[nrow].y - size);
  circv[ncirc].width = circv[ncirc].height = 2*size;
}

void
build_rect (icoords *pos, gint nrow, rectd * rectv, gint nrect, gshort size)
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
build_glyph (glyphd *gl, icoords *xypos, gint jpos,
  GdkPoint *pointv,   gint *np,
  GdkSegment *segv,   gint *ns,
  rectd *openrectv,   gint *nr_open,
  rectd *filledrectv, gint *nr_filled,
  arcd *openarcv,     gint *nc_open,
  arcd *filledarcv,   gint *nc_filled)
{
  gshort size, type;
  size = gl->size;
  type = gl->type;

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
  if (GTK_IS_GGOBI_PARCOORDS_DISPLAY(display)) {
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
  else if (GTK_IS_GGOBI_TIME_SERIES_DISPLAY(display)) {
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
  rectd *open_rects,   gint nr_open,
  rectd *filled_rects, gint nr_filled,
  arcd *open_arcs,     gint nc_open,
  arcd *filled_arcs,   gint nc_filled,
  ggobid *gg)
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
win32_draw_to_pixmap_unbinned (gint current_color, splotd *sp, gboolean draw_hidden, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  datad *d = display->d;

  gint i, m;
  gint npt, nseg, nr_open, nr_filled, nc_open, nc_filled;
  gint nwhisker_segs = 0;
  gint nash_segs = 0;
#if OLD
  gint dtype = display->displaytype; 
#endif
  npt = nseg = nr_open = nr_filled = nc_open = nc_filled = 0;

  if (sp->win32.npoints < d->nrows)
    drawing_arrays_alloc (sp, d, gg);

  for (i=0; i<d->nrows_in_plot; i++) {
    m = d->rows_in_plot.els[i];

    if (d->color_now.els[m] == current_color &&
        splot_plot_case (m, d, sp, display, gg) &&
        (draw_hidden == d->hidden_now.els[m]))
    {
      if (display->options.points_show_p) {
        build_glyph (&d->glyph_now.els[m], sp->screen, m,
          sp->win32.points, &npt,           sp->win32.segs, &nseg,
          sp->win32.open_rects, &nr_open,   sp->win32.filled_rects, &nr_filled,
          sp->win32.open_arcs, &nc_open,    sp->win32.filled_arcs, &nc_filled);

        if ((GTK_IS_GGOBI_PARCOORDS_DISPLAY(display) || GTK_IS_GGOBI_TIME_SERIES_DISPLAY(display)) &&
            display->options.whiskers_show_p)
        {
          build_whisker_segs (m, &nwhisker_segs, sp);
        } else if (GTK_IS_GGOBI_SCATTERPLOT_DISPLAY(display) &&
                   projection_get(gg) == P1PLOT &&
                   cpanel->p1d.type == ASH &&
                   cpanel->p1d.ASH_add_lines_p) {
          build_ash_segs (m, &nash_segs, sp);
        }
      }
    }
  }
  if (nwhisker_segs)
    gdk_draw_segments (sp->pixmap0, gg->plot_GC,
      sp->win32.whisker_segs, nwhisker_segs);
  if (nash_segs)
    gdk_draw_segments (sp->pixmap0, gg->plot_GC, sp->win32.ash_segs, nash_segs);
  draw_glyphs (sp, sp->pixmap0,
    sp->win32.points, npt,           sp->win32.segs, nseg,
    sp->win32.open_rects, nr_open,   sp->win32.filled_rects, nr_filled,
    sp->win32.open_arcs, nc_open,    sp->win32.filled_arcs, nc_filled,
    gg);
}

void
win32_draw_to_pixmap_binned (icoords *bin0, icoords *bin1,
  gint current_color, splotd *sp, gboolean draw_hidden, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gint ih, iv;
  gint m, j;
  gint npt, nseg, nr_open, nr_filled, nc_open, nc_filled;
  gint nwhisker_segs = 0;
  npt = nseg = nr_open = nr_filled = nc_open = nc_filled = 0;

  for (ih=bin0->x; ih<=bin1->x; ih++) {
    for (iv=bin0->y; iv<=bin1->y; iv++) {
      for (m=0; m<d->brush.binarray[ih][iv].nels; m++) {
        j = d->rows_in_plot.els[d->brush.binarray[ih][iv].els[m]];
        if (d->color_now.els[j] == current_color &&
            splot_plot_case (j, d, sp, display, gg) &&
            (draw_hidden == d->hidden_now.els[j]))
        {
          build_glyph (&d->glyph_now.els[j], sp->screen, j,
            sp->win32.points, &npt,          
            sp->win32.segs, &nseg,
            sp->win32.open_rects, &nr_open,  
            sp->win32.filled_rects, &nr_filled,
            sp->win32.open_arcs, &nc_open,   
            sp->win32.filled_arcs, &nc_filled);

          if (GTK_IS_GGOBI_PARCOORDS_DISPLAY(display) ||
              GTK_IS_GGOBI_TIME_SERIES_DISPLAY(display))
          {
            build_whisker_segs (j, &nwhisker_segs, sp);
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
