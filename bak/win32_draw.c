/*
 * Batched drawing:   I may need to continue drawing this
 * way to achieve adequate speed in Windows.
*/

#ifdef _WIN32

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
drawing_arrays_alloc () {
  if (maxn == 0) {
    maxn = xg.nrows;
    points = (GdkPoint *) g_malloc (maxn * sizeof (GdkPoint));
    segs = (GdkSegment *) g_malloc (2 * maxn * sizeof (GdkSegment));
    whisker_segs = (GdkSegment *) g_malloc (2 * maxn * sizeof (GdkSegment));
    open_rects = (rectd *) g_malloc (maxn * sizeof (rectd));
    filled_rects = (rectd *) g_malloc (maxn * sizeof (rectd));
    open_arcs = (arcd *) g_malloc (maxn * sizeof (arcd));
    filled_arcs = (arcd *) g_malloc (maxn * sizeof (arcd));
  } else {
    maxn = xg.nrows;
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

win32_draw_to_pixmap_unbinned (gint *colors_used, gint ncolors,
  splotd *sp)
{
  displayd *display = (displayd *) sp->displayptr;
  gint m, j;
  gint npt, nseg, nr_open, nr_filled, nc_open, nc_filled;
  npt = nseg = nr_open = nr_filled = nc_open = nc_filled = 0;

  for (m=0; m<xg.nrows_in_plot; m++) {
    j = xg.rows_in_plot[m];
    if (!xg.erased[j] && xg.color_now[j] == current_color) {
      build_glyph (&xg.glyph_now[j], sp->screen, j,
        points, &npt,           segs, &nseg,
        open_rects, &nr_open,   filled_rects, &nr_filled,
        open_arcs, &nc_open,    filled_arcs, &nc_filled);

      if (display->displaytype == parcoords) {
        build_whisker_segs (&xg.glyph_now[j], sp->screen, j);
      }
    }
  }
  draw_glyphs (sp->pixmap0,
    points, npt,           segs, nseg,
    open_rects, nr_open,   filled_rects, nr_filled,
    open_arcs, nc_open,    filled_arcs, nc_filled);
}

/*
 * This routine seems to be missing in gdk, so here it is for UNIX
*/
#define MAXARCS 16384
/*
void
win32_draw_arcs (GdkDrawable *drawable,
          GdkGC       *gc,
          gint         filled,
          arcd         *arcs,
          gint         narcs)
{
  GdkWindowPrivate *drawable_private;
  GdkGCPrivate *gc_private;
  gint i, k, n;
  XArc *a = (XArc *) g_malloc (narcs * sizeof (XArc));
  for (i=0; i<narcs; i++) {
    a[i].x = arcs[i].x;
    a[i].y = arcs[i].y;
    a[i].width = arcs[i].width;
    a[i].height = arcs[i].height;
    a[i].angle1 = 0;
    a[i].angle2 = (gshort) 23040;
  }

  g_return_if_fail (drawable != NULL);
  g_return_if_fail (gc != NULL);

  drawable_private = (GdkWindowPrivate*) drawable;
  if (drawable_private->destroyed)
    return;
  gc_private = (GdkGCPrivate*) gc;

  k = 0;
  while (narcs > 0) {
    n = (narcs > MAXARCS) ? MAXARCS : narcs;

    if (filled)
      XFillArcs (drawable_private->xdisplay, drawable_private->xwindow,
        gc_private->xgc, a + k*MAXARCS, n);
    else
      XDrawArcs (drawable_private->xdisplay, drawable_private->xwindow,
        gc_private->xgc, a + k*MAXARCS, n);

    narcs -= n;
    k++;
  }
}
*/

/*
void
win32_draw_rectangles (GdkDrawable *drawable,
          GdkGC       *gc,
          gint         filled,
          rectd        *rects,
          gint         nrects)
{
  GdkWindowPrivate *drawable_private;
  GdkGCPrivate *gc_private;
  gint i, k;
  XRectangle *r = (XRectangle *) g_malloc (nrects * sizeof (XRectangle));
  for (i=0; i<nrects; i++) {
    r[i].x = rects[i].x;
    r[i].y = rects[i].y;
    r[i].width = rects[i].width;
    r[i].height = rects[i].height;
  }

  g_return_if_fail (drawable != NULL);
  g_return_if_fail (gc != NULL);

  drawable_private = (GdkWindowPrivate*) drawable;
  if (drawable_private->destroyed)
    return;
  gc_private = (GdkGCPrivate*) gc;

  if (filled)
    XFillRectangles (drawable_private->xdisplay, drawable_private->xwindow,
      gc_private->xgc, r, nrects);
  else
    XDrawRectangles (drawable_private->xdisplay, drawable_private->xwindow,
      gc_private->xgc, r, nrects);
}
*/

void
build_plus (icoords *pos, gint nrow, GdkSegment *segv, gint nplus, gshort size)
{
  gshort x = (gshort) pos[nrow].x;

  switch (size)
  {
    case TINY:
    case SMALL:
      break;
    case MEDIUM:
    case LARGE:
      size++ ;
      break;
    case JUMBO:
      size = size + 2 ;
      break;
    default:
      g_printerr ("error in build_plus; impossible size %d\n", size);
      size = (gshort) MEDIUM + 1 ;
      break;
  }
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
  size = size * 3;

  circv[ncirc].x = (gushort) (pos[nrow].x - size/2);
  circv[ncirc].y = (gushort) (pos[nrow].y - size/2);
  circv[ncirc].width = size;
  circv[ncirc].height = size;
}

void
build_rect (icoords *pos, gint nrow, rectd * rectv, gint nrect, gshort size)
{
  size = size * 3 - 1;

  rectv[nrect].x = (gushort) (pos[nrow].x - (size/2 + 1));
  rectv[nrect].y = (gushort) (pos[nrow].y - (size/2 + 1));
  rectv[nrect].width = rectv[nrect].height = size;
}

void
build_point (icoords *pos, gint nrow, GdkPoint * pointv, gint npt)
{
  pointv[npt].x = (gushort) (pos[nrow].x);
  pointv[npt].y = (gushort) (pos[nrow].y);
}

void
build_glyph (glyphv *gl, icoords *xypos, gint jpos,
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

  if (type == PLUS_GLYPH) {
    build_plus (xypos, jpos, segv, *ns, size);
    *ns = *ns+2;
  } else if (type == X_GLYPH) {
    build_x (xypos, jpos, segv, *ns, size);
    *ns = *ns+2;
  }
  else if (type == OPEN_RECTANGLE) {
    build_rect (xypos, jpos, openrectv, *nr_open, size);
    (*nr_open)++;
  }
  else if (type == FILLED_RECTANGLE) {
    build_rect (xypos, jpos, filledrectv, *nr_filled, size);
    (*nr_filled)++;
  }
  else if (type == OPEN_CIRCLE) {
    build_circle (xypos, jpos, openarcv, *nc_open, size);
    (*nc_open)++;
  }
  else if (type == FILLED_CIRCLE) {
    build_circle (xypos, jpos, filledarcv, *nc_filled, size);
    (*nc_filled)++;
  }
  else if (type == POINT_GLYPH) {
    build_point (xypos, jpos, pointv, *np);
    (*np)++;
  }
  else {
    g_printerr ("build_glyph: impossible glyph type %d\n", type);
  }
}

void
build_whisker_segs (gint j, splotd *sp) {
  gint n = 2*j;
  gint i;

  for (i=0; i<1; i++) {
    whisker_segs[n].x1 = sp->whiskers[n].x1;
    whisker_segs[n].y1 = sp->whiskers[n].y1;
    whisker_segs[n].x2 = sp->whiskers[n].x2;
    whisker_segs[n].y2 = sp->whiskers[n].y2;
  }
}

void
draw_glyphs (GdkDrawable *drawable,
  GdkPoint *points,    gint np,
  GdkSegment *segs,    gint ns,
  rectd *open_rects,   gint nr_open,
  rectd *filled_rects, gint nr_filled,
  arcd *open_arcs,     gint nc_open,
  arcd *filled_arcs,   gint nc_filled)
{
  if (plot_GC == NULL)
    init_plot_GC (drawable);

  if (np)
    gdk_draw_points (drawable, plot_GC, points, np);
  if (ns)
    gdk_draw_segments (drawable, plot_GC, segs, ns);

  if (nr_open)
    win32_draw_rectangles (drawable, plot_GC, OPEN, open_rects, nr_open);
  if (nr_filled)
    win32_draw_rectangles (drawable, plot_GC, FILL, filled_rects, nr_filled);

  if (nc_open)
    win32_draw_arcs (drawable, plot_GC, OPEN, open_arcs, nc_open);
  if (nc_filled)
    win32_draw_arcs (drawable, plot_GC, FILL, filled_arcs, nc_filled);
}

#endif
