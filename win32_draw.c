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


/*
 * Batched drawing:   I may need to continue drawing this
 * way to achieve adequate speed in Windows.
*/

#ifdef WIN32

static void build_circle (icoords *, gint, arcd *, gint, gshort);
static void build_plus (icoords *, gint, GdkSegment *, gint, gshort);
static void build_rect (icoords *, gint, rectd *, gint, gshort);
static void build_x (icoords *, gint, GdkSegment *, gint, gshort);

/*
 * I think I could eliminate a lot of copies if I made these
 * vectors of pointers and just assigned addresses.
*/
static gint maxn = 0;
static GdkPoint   *points;
static GdkSegment *segs;
static GdkSegment *whisker_segs;
static rectd      *open_rects;
static rectd      *filled_rects;
static arcd       *open_arcs;
static arcd       *filled_arcs;

static void
drawing_arrays_alloc (datad *d, ggobid *gg) {
  if (maxn == 0) {
    maxn = d->nrows;
    points = (GdkPoint *) g_malloc (maxn * sizeof (GdkPoint));
    segs = (GdkSegment *) g_malloc (2 * maxn * sizeof (GdkSegment));
    whisker_segs = (GdkSegment *) g_malloc (2 * maxn * sizeof (GdkSegment));
    open_rects = (rectd *) g_malloc (maxn * sizeof (rectd));
    filled_rects = (rectd *) g_malloc (maxn * sizeof (rectd));
    open_arcs = (arcd *) g_malloc (maxn * sizeof (arcd));
    filled_arcs = (arcd *) g_malloc (maxn * sizeof (arcd));
  } else {
    maxn = d->nrows;
    points = (GdkPoint *) g_realloc (points, maxn * sizeof (GdkPoint));
    segs = (GdkSegment *) g_realloc (segs, 2 * maxn * sizeof (GdkSegment));
    whisker_segs = (GdkSegment *)
      g_realloc (whisker_segs, 2 * maxn * sizeof (GdkSegment));
    open_rects = (rectd *) g_realloc (open_rects, maxn * sizeof (rectd));
    filled_rects = (rectd *) g_realloc (filled_rects, maxn * sizeof (rectd));
    open_arcs = (arcd *) g_realloc (open_arcs, maxn * sizeof (arcd));
    filled_arcs = (arcd *) g_realloc (filled_arcs, maxn * sizeof (arcd));
  }
}

static void
drawing_arrays_free (){
  g_free ((gpointer) points);
  g_free ((gpointer) segs);
  g_free ((gpointer) whisker_segs);
  g_free ((gpointer) open_rects);
  g_free ((gpointer) filled_rects);
  g_free ((gpointer) open_arcs);
  g_free ((gpointer) filled_arcs);
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
    case DOT:
      build_point (xypos, jpos, pointv, *np);
      (*np)++;
    break;
    UNKNOWN_GLYPH:
    default:
      g_printerr ("build_glyph: impossible glyph type %d\n", type);
  }
}

void
build_whisker_segs (gint j, gint *nwhisker_segs, splotd *sp) {
  displayd *display = (displayd *) sp->displayptr;
  gint n;
  if (display->displaytype == parcoords){
    n = 2*j;
    whisker_segs[*nwhisker_segs].x1 = sp->whiskers[n].x1;
    whisker_segs[*nwhisker_segs].y1 = sp->whiskers[n].y1;
    whisker_segs[*nwhisker_segs].x2 = sp->whiskers[n].x2;
    whisker_segs[*nwhisker_segs].y2 = sp->whiskers[n].y2;
    n++;
    *nwhisker_segs += 1;
    whisker_segs[*nwhisker_segs].x1 = sp->whiskers[n].x1;
    whisker_segs[*nwhisker_segs].y1 = sp->whiskers[n].y1;
    whisker_segs[*nwhisker_segs].x2 = sp->whiskers[n].x2;
    whisker_segs[*nwhisker_segs].y2 = sp->whiskers[n].y2;
    *nwhisker_segs += 1;
  }
  else if (display->displaytype == tsplot) {
    whisker_segs[*nwhisker_segs].x1 = sp->whiskers[j].x1;
    whisker_segs[*nwhisker_segs].y1 = sp->whiskers[j].y1;
    whisker_segs[*nwhisker_segs].x2 = sp->whiskers[j].x2;
    whisker_segs[*nwhisker_segs].y2 = sp->whiskers[j].y2; 
    *nwhisker_segs += 1; 
  }
}

void
draw_glyphs (GdkDrawable *drawable,
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
win32_draw_to_pixmap_unbinned (gint current_color, splotd *sp, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gint i, m;
  gint npt, nseg, nr_open, nr_filled, nc_open, nc_filled;
  gint nwhisker_segs = 0;
  gboolean draw_case;
  gint dtype = display->displaytype;

  npt = nseg = nr_open = nr_filled = nc_open = nc_filled = 0;

  if (maxn != d->ncols)
    drawing_arrays_alloc (d, gg);

  for (i=0; i<d->nrows_in_plot; i++) {
    m = d->rows_in_plot[i];
    draw_case = splot_plot_case (m, d, sp, display, gg);

    if (draw_case && d->color_now.els[m] == current_color) {
      if (display->options.points_show_p) {
        build_glyph (&d->glyph_now.els[m], sp->screen, m,
          points, &npt,           segs, &nseg,
          open_rects, &nr_open,   filled_rects, &nr_filled,
          open_arcs, &nc_open,    filled_arcs, &nc_filled);

        if ((dtype == parcoords || dtype == tsplot) &&
            display->options.whiskers_show_p)
        {
          build_whisker_segs (m, &nwhisker_segs, sp);
        }

      }
    }
  }
  if (nwhisker_segs)
    gdk_draw_segments (sp->pixmap0, gg->plot_GC, whisker_segs, nwhisker_segs);
  draw_glyphs (sp->pixmap0,
    points, npt,           segs, nseg,
    open_rects, nr_open,   filled_rects, nr_filled,
    open_arcs, nc_open,    filled_arcs, nc_filled,
    gg);
}

void
win32_draw_to_pixmap_binned (icoords *bin0, icoords *bin1,
  gint current_color, splotd *sp, ggobid *gg)
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
        j = d->rows_in_plot[d->brush.binarray[ih][iv].els[m]];
        if (!d->hidden_now.els[j] && d->color_now.els[j] == current_color) {
          build_glyph (&d->glyph_now.els[j], sp->screen, j,
            points, &npt,           segs, &nseg,
            open_rects, &nr_open,   filled_rects, &nr_filled,
            open_arcs, &nc_open,    filled_arcs, &nc_filled);

          if (display->displaytype == parcoords ||
              display->displaytype == tsplot)
          {
            build_whisker_segs (j, &nwhisker_segs, sp);
          }
        }
      }
    }
  }
  gdk_draw_segments (sp->pixmap0, gg->plot_GC, whisker_segs, nwhisker_segs);
  draw_glyphs (sp->pixmap0,
    points, npt,           segs, nseg,
    open_rects, nr_open,   filled_rects, nr_filled,
    open_arcs, nc_open,    filled_arcs, nc_filled,
    gg);
}
#endif
