#include "ggobi.h"
#include "tsdisplay.h"

#include <string.h>
#include "externs.h"


void
tsWorldToPlane(splotd *sp, datad *d, ggobid *gg)
{
      xy_reproject (sp, d->world.vals, d, gg);
}

splotd *
gtk_time_series_splot_new(displayd *dpy, gint width, gint height, ggobid *gg)
{
  timeSeriesSPlotd *bsp;
  splotd *sp;

  bsp = gtk_type_new(GTK_TYPE_GGOBI_TS_SPLOT);
  sp = GTK_GGOBI_SPLOT(bsp);


  splot_init(sp, dpy, width, height, gg);

  return(sp);
}

void
tsDestroy(splotd *sp)
{
      g_free ((gpointer) sp->whiskers);
}

void
tsWithinPlaneToScreen(splotd *sp, displayd *display, datad *d, ggobid *gg)
{
      tsplot_whiskers_make (sp, display, gg);
}

gboolean
tsDrawEdge_p(splotd *sp, gint m, datad *d, datad *e, ggobid *gg)
{
   gboolean draw_edge = true;

   draw_edge = !(e->missing.vals[m][sp->xyvars.y] ||  e->missing.vals[m][sp->xyvars.x]);
   return(draw_edge);
}

gboolean
tsDrawCase_p(splotd *sp, gint m, datad *d, ggobid *gg)
{
  return !(d->missing.vals[m][sp->xyvars.y] || d->missing.vals[m][sp->xyvars.x]);
}

void
tsAddPlotLabels(splotd *sp, GdkDrawable *drawable, ggobid *gg) 
{
    displayd *display = sp->displayptr;
    GList *l = display->splots;
    gint lbearing, rbearing, width, ascent, descent;
    GtkStyle *style = gtk_widget_get_style (sp->da);
    vartabled *vtx, *vty;

    if (l->data == sp) {
      vtx = vartable_element_get (sp->xyvars.x, display->d);
      gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
        gtk_style_get_font (style),
#else
        style->font,
#endif
        vtx->collab_tform, strlen (vtx->collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
        gtk_style_get_font (style),
#else
        style->font,
#endif
        gg->plot_GC,
        sp->max.x - width - 5,
        sp->max.y - 5,
        vtx->collab_tform);
    }
    vty = vartable_element_get (sp->xyvars.y, display->d);
    gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
      gtk_style_get_font (style),
#else
      style->font,
#endif
      vty->collab_tform, strlen (vty->collab_tform),
      &lbearing, &rbearing, &width, &ascent, &descent);
    gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
      gtk_style_get_font (style),
#else
      style->font,
#endif
      gg->plot_GC,
      5, 5 + ascent + descent,
      vty->collab_tform);
}

void
tsWithinDrawBinned(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc)
{
	gdk_draw_line (drawable, gc,
		       sp->whiskers[m].x1, sp->whiskers[m].y1,
		       sp->whiskers[m].x2, sp->whiskers[m].y2);
}


void
tsShowWhiskers(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc)
{
   displayd *dpy = sp->displayptr;
   if (dpy->options.whiskers_show_p && m < dpy->d->nrows_in_plot-1)  /*-- there are n-1 whiskers --*/
      gdk_draw_line (drawable, gc,
		     sp->whiskers[m].x1, sp->whiskers[m].y1,
		     sp->whiskers[m].x2, sp->whiskers[m].y2);
}


GdkSegment * 
tsAllocWhiskers(splotd *sp, gint nrows, datad *d)
{
  return((GdkSegment *) g_malloc ((nrows-1) * sizeof (GdkSegment)));
}

gchar *
tsTreeLabel(splotd *sp, datad *d, ggobid *gg)
{
  vartabled *vty;
  int n;
  char *buf;

    vty = vartable_element_get (sp->xyvars.y, d);
    n = strlen (vty->collab);
    buf = (gchar*) g_malloc(n* sizeof (gchar*));
   sprintf(buf, "%s", vty->collab);

   return(buf);
}
