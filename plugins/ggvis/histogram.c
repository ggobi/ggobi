#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>
#include <string.h>

#include "plugin.h"
#include "ggvis.h"

#define STR_VMARGIN  5
#define STR_HMARGIN 10

static gint histogram_plotheight_get (ggvisd *ggv);
static void histogram_bins_reset (ggvisd *ggv);
static void histogram_make (ggvisd *ggv);
static void histogram_draw (ggvisd *ggv, ggobid *gg);

gdouble trans_dist_max, trans_dist_min;

static void
histogram_pixmap_clear (ggvisd *ggv, ggobid *gg)
{
  colorschemed *scheme = gg->activeColorScheme;
  dissimd *D = ggv->dissim;
  GtkWidget *da = D->da;

  if (gg->plot_GC == NULL)
    init_plot_GC (D->pix, gg);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (D->pix, gg->plot_GC,
                      TRUE, 0, 0,
                      da->allocation.width,
                      da->allocation.height);
}

gint
ggv_histogram_configure_cb (GtkWidget *w, GdkEventExpose *event,
  PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  dissimd *D = ggv->dissim;
  ggobid *gg = inst->gg;
  gboolean retval = true;

  if (ggv == NULL)  /*-- too early to configure --*/
    return retval;
  if (w->allocation.width < 2 || w->allocation.height < 2)
    return retval;

  if (D->pix != NULL)
    gdk_pixmap_unref (D->pix);
  D->pix = gdk_pixmap_new (w->window,
    w->allocation.width, w->allocation.height, -1);
  histogram_pixmap_clear (ggv, gg);

  if (ggv->Dtarget.nrows == 0 || ggv->Dtarget.ncols == 0)
    return retval;  /*-- too early to figure out the histogram bins --*/

  histogram_bins_reset (ggv);
  if (D->nbins > 0) {
    histogram_make (ggv);
    histogram_draw (ggv, gg);
  }

  gtk_widget_queue_draw (w);

  return retval;
}

void
histogram_pixmap_copy (ggvisd *ggv, ggobid *gg)
{
  dissimd *D = ggv->dissim;
  GtkWidget *da = D->da;

  if (gg->plot_GC == NULL)
    init_plot_GC (D->pix, gg);

  /* copy the pixmap to the screen */
  gdk_draw_pixmap (da->window, gg->plot_GC, D->pix,
                   0, 0, 0, 0,
                   da->allocation.width,
                   da->allocation.height);
}

gint
ggv_histogram_expose_cb (GtkWidget *w, GdkEventExpose *event,
  PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  ggvisd *ggv = ggvisFromInst (inst);
  dissimd *D = ggv->dissim;
  gboolean retval = true;

  /*-- sanity checks --*/
  if (ggv == NULL)  /*-- too early to expose or configure --*/
    return retval;
  if (D->pix == NULL)
    return retval;
  if (w->allocation.width < 2 || w->allocation.height < 2)
    return retval;

  /*-- copy the pixmap to the screen --*/
  histogram_pixmap_copy (ggv, gg);

  return retval;
}


static void
histogram_make (ggvisd *ggv)
{
/*
 * Scale the counts into (xmin, ymin), (xmax, ymax);
 * construct XRectangle structures for drawing.
*/
  dissimd *D = ggv->dissim;
  gint maxcount = 0;
  gint barheight;
  gint i;
  GtkWidget *da = D->da;
  gint xmin = HISTOGRAM_HMARGIN;
  /*gint xmax = da->allocation.width - HISTOGRAM_HMARGIN;*/
  /*gint ymin = HISTOGRAM_VMARGIN;*/
  gint ymax = da->allocation.height - 2*HISTOGRAM_VMARGIN -
    HISTOGRAM_GRIP_SPACE;
  gint pheight = histogram_plotheight_get (ggv);

  /* find maximum count */
  for (i=0; i<D->nbins; i++)
    if (D->bins.els[i] > maxcount) maxcount = D->bins.els[i];

  for (i=0; i<D->nbins; i++) {
    barheight = (int) (pheight * (double)D->bins.els[i]/(double)maxcount);
    /* x,y: upper lefthand corner */
    D->bars[i].x = xmin + (i * HISTOGRAM_BWIDTH);
    D->bars[i].y = ymax - barheight;
    D->bars[i].width = HISTOGRAM_BWIDTH;
    D->bars[i].height = barheight;
  }
}

void
draw_grip_control (ggvisd *ggv, ggobid *gg) {
  gint ypos;
  GtkWidget *da = ggv->dissim->da;
  dissimd *D = ggv->dissim;
  gint xmin = HISTOGRAM_HMARGIN;
  gint xmax = da->allocation.width - HISTOGRAM_HMARGIN;
  /* The grip positions are at the center of the grips */

  /* Suppose I allow the grips to go a bit into the margins ... */
  gint min_grip_pos = xmin - HISTOGRAM_HMARGIN/2;
  gint max_grip_pos = xmax + HISTOGRAM_HMARGIN/2;

  if (gg->plot_GC == NULL)
    init_plot_GC (D->pix, gg);

  if (D->lgrip_pos == -1 && D->rgrip_pos == -1) {  /* first time */
    D->lgrip_pos = min_grip_pos;
    D->rgrip_pos = max_grip_pos;
  }

  ypos = da->allocation.height - HISTOGRAM_VMARGIN - HISTOGRAM_GRIP_HEIGHT/2;

  gdk_gc_set_foreground (gg->plot_GC, &gg->mediumgray);
  gdk_draw_line (D->pix, gg->plot_GC, min_grip_pos, ypos, max_grip_pos, ypos);

  draw_3drectangle (D->pix, D->lgrip_pos, ypos,
    HISTOGRAM_GRIP_WIDTH, HISTOGRAM_GRIP_HEIGHT, gg);
  draw_3drectangle (D->pix, D->rgrip_pos, ypos,
    HISTOGRAM_GRIP_WIDTH, HISTOGRAM_GRIP_HEIGHT, gg);
}

static void
histogram_draw (ggvisd *ggv, ggobid *gg)
{
  gint i;
  gchar *str;
  colorschemed *scheme = gg->activeColorScheme;
  dissimd *D = ggv->dissim;
  GtkWidget *da = D->da;
  gint ymax = da->allocation.height - 2*HISTOGRAM_VMARGIN -
    HISTOGRAM_GRIP_SPACE;
  GtkStyle *style = gtk_widget_get_style (da);
  gint lbearing, rbearing, strwidth, ascent, descent;

  if (D->pix == NULL)
    return;
  if (ggv->trans_dist.nels == 0)
    return;

  if (gg->plot_GC == NULL)
    init_plot_GC (D->pix, gg);

  histogram_pixmap_clear (ggv, gg);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);

  for (i=0; i<D->nbins; i++) {

    if (D->bars_included.els[i]) {
      gdk_draw_rectangle (D->pix, gg->plot_GC, true,
        D->bars[i].x, D->bars[i].y, D->bars[i].width, D->bars[i].height);
    } else {
      if (i>0)  /* leading vertical edge */
        gdk_draw_line (D->pix, gg->plot_GC,
          D->bars[i].x, D->bars[i-1].y, D->bars[i].x, D->bars[i].y);

      /* horizontal line */
      gdk_draw_line (D->pix, gg->plot_GC,
        D->bars[i].x, D->bars[i].y,
        D->bars[i].x+D->bars[i].width, D->bars[i].y);

      if (i<D->nbins-1)  /* trailing vertical edge */
        gdk_draw_line (D->pix, gg->plot_GC,
          D->bars[i].x+D->bars[i].width, D->bars[i].y,
          D->bars[i].x+D->bars[i].width, D->bars[i+1].y);
    }
  }

  gdk_draw_line (D->pix, gg->plot_GC,
    D->bars[D->nbins-1].x+D->bars[D->nbins-1].width,
      D->bars[D->nbins-1].y,
    D->bars[D->nbins-1].x+D->bars[D->nbins-1].width,
      ymax);


  if (trans_dist_max == DBL_MIN)  /*-- not initialized -- why? --*/
    str = g_strdup_printf ("%s", "??");
  else
    str = g_strdup_printf ("%2.2f", trans_dist_max);
  gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
    gtk_style_get_font (style),
#else
    style->font,
#endif
    str, strlen (str),
    &lbearing, &rbearing, &strwidth, &ascent, &descent);
  gdk_draw_string (D->pix,
#if GTK_MAJOR_VERSION == 2
      gtk_style_get_font (style),
#else
      style->font,
#endif
    gg->plot_GC,
    da->allocation.width - STR_HMARGIN - strwidth,
    ascent + descent + STR_VMARGIN,
    str);
  g_free (str);

  if (trans_dist_min == DBL_MAX)  /*-- not initialized -- why? --*/
    str = g_strdup_printf ("%s", "??");
  else
    str = g_strdup_printf ("%2.2f", trans_dist_min);
  gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
    gtk_style_get_font (style),
#else
    style->font,
#endif
    str, strlen (str),
    &lbearing, &rbearing, &strwidth, &ascent, &descent);
  gdk_draw_string (D->pix,
#if GTK_MAJOR_VERSION == 2
      gtk_style_get_font (style),
#else
      style->font,
#endif
    gg->plot_GC,
    (gint) STR_HMARGIN/2,
    ascent + descent + STR_VMARGIN,
    str);
  g_free (str);

  draw_grip_control (ggv, gg);

  histogram_pixmap_copy (ggv, gg);
}

void
set_threshold (ggvisd *ggv)
{
  gint k;
  dissimd *D = ggv->dissim;
  GtkWidget *da = D->da;
  gint xmin = HISTOGRAM_HMARGIN;
  gint xmax = da->allocation.width - HISTOGRAM_HMARGIN;

  for (k=0; k<D->nbins; k++) {

    /* bars are included if their left edge is greater than
       the left grip's center and their right edge is less than
       the right grip's center.
    */
    if (D->bars[k].x >= D->lgrip_pos &&
        D->bars[k].x + D->bars[k].width <= D->rgrip_pos)
    {
      D->bars_included.els[k] = true;
    }
    else
      D->bars_included.els[k] = false;
  }

  /* Now specify the thresholds in data terms */

  /*
   * Scale the grip position onto [0,1]
  */

  D->low = MAX(0, (gdouble)(D->lgrip_pos - xmin) / (gdouble) (xmax - xmin) );
  D->high = MIN(1, (gdouble)(D->rgrip_pos - xmin) / (gdouble) (xmax - xmin) );

  ggv->threshold_low  = D->low  * ggv->Dtarget_max;
  ggv->threshold_high = D->high * ggv->Dtarget_max;
}

/* ARGSUSED */
gint
ggv_histogram_motion_cb (GtkWidget *w, GdkEventMotion *xmotion,
  PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  ggvisd *ggv = ggvisFromInst (inst);
  dissimd *D = ggv->dissim;
  GtkWidget *da = D->da;
  gint xmin = HISTOGRAM_HMARGIN;
  gint xmax = da->allocation.width - HISTOGRAM_HMARGIN;
  gint min_grip_pos = xmin - HISTOGRAM_HMARGIN/2;
  gint max_grip_pos = xmax + HISTOGRAM_HMARGIN/2;
  gint x, y;
  GdkModifierType state;
  gboolean buttondown = false;

  gdk_window_get_pointer (w->window, &x, &y, &state);
  if ((state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
    buttondown = true;
  else if ((state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
    buttondown = true;
  else if ((state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
    buttondown = true;

  if (!buttondown)
    return false;

  if (D->lgrip_down &&
      x + HISTOGRAM_GRIP_WIDTH < D->rgrip_pos &&
      x >= min_grip_pos)
  {
    D->lgrip_pos = x;
  }
  else if (D->rgrip_down &&
           x > D->lgrip_pos + HISTOGRAM_GRIP_WIDTH &&
           x <= max_grip_pos)
  {
    D->rgrip_pos = x;
  }

  set_threshold (ggv);
  histogram_draw (ggv, gg);

  return true;
}

void
ggv_histogram_button_release_cb (GtkWidget *w, GdkEventButton *evnt,
  PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  dissimd *D = ggv->dissim;
  D->lgrip_down = D->rgrip_down = false;
}

void
ggv_histogram_button_press_cb (GtkWidget *w, GdkEventButton *evnt,
  PluginInstance *inst)
{
/*
 * Using only the x coordinate, see if we're on top of one
 * of the two grips.
*/
  ggvisd *ggv = ggvisFromInst (inst);
  dissimd *D = ggv->dissim;
  gint x, y;
  GdkModifierType state;

  gdk_window_get_pointer (w->window, &x, &y, &state);

  if (x >= D->lgrip_pos - HISTOGRAM_GRIP_WIDTH/2 &&
      x <= D->lgrip_pos + HISTOGRAM_GRIP_WIDTH/2)
  {
    D->lgrip_down = true;
  }
  else if (x >= D->rgrip_pos - HISTOGRAM_GRIP_WIDTH/2 &&
           x <= D->rgrip_pos + HISTOGRAM_GRIP_WIDTH/2)
  {
    D->rgrip_down = true;
  }
}

static void
histogram_bins_reset (ggvisd *ggv)
{
  dissimd *D = ggv->dissim;
  gint i, k;
  gdouble fac, t_d, t_delta;
  GtkWidget *da = D->da;
  gint xmin = HISTOGRAM_HMARGIN;
  gint xmax = da->allocation.width - HISTOGRAM_HMARGIN;
  gint pwidth = xmax - xmin;

  /* width of plot divided by binwidth */
  D->nbins = (int) ( (double)pwidth / (double)HISTOGRAM_BWIDTH );
  /* fix any rounding errors */
  pwidth = D->nbins * HISTOGRAM_BWIDTH; 
  xmax = xmin + pwidth;

  /* map trans_dist[i] to [0,1] and sort into bins */
  trans_dist_min = DBL_MAX; trans_dist_max = DBL_MIN;

/*
 * Probably I can initialize trans_dist before mds_once is run
*/
  if (ggv->trans_dist.nels > 0) {
    for (i=0; i<ggv->Dtarget.nrows*ggv->Dtarget.ncols; i++) {
      t_d = ggv->trans_dist.els[i];
      if (t_d != DBL_MAX) {
        if (t_d > trans_dist_max) trans_dist_max = t_d;
        if (t_d < trans_dist_min) trans_dist_min = t_d;
      }
    }
  } else g_printerr ("trans_dist not initialized\n");

  /* in case trans_dist is constant and t_delta would be zero */
  t_delta = MAX(trans_dist_max-trans_dist_min, 1E-100);
  /* so rounding off results is strictly < thr_nbins */
  fac = (double)D->nbins * 0.999999;  

  D->nbins = MIN (D->nbins, D->bins.nels);
  for (i=0; i<D->nbins; i++)
    D->bins.els[i] = 0;

  for (i=0; i < ggv->Dtarget.nrows*ggv->Dtarget.ncols; i++) {
    t_d = ggv->trans_dist.els[i];
    if (t_d != DBL_MAX) {
      k = (gint) ((ggv->trans_dist.els[i]-trans_dist_min)/t_delta * fac);
      if (k >= D->bins.nels) g_printerr ("k too large: %d\n", k);
      D->bins.els[k]++;
    }
  }
}

static gint
histogram_plotheight_get (ggvisd *ggv)
{
  dissimd *D = ggv->dissim;
  GtkWidget *da = D->da;
  gint ymin = HISTOGRAM_VMARGIN;
  /*
   * subtract two lower margins, above and below the grip control,
   * and the height of the grip control.
  */
  gint ymax = da->allocation.height - 2*HISTOGRAM_VMARGIN -
    HISTOGRAM_GRIP_SPACE;
  return (ymax - ymin);
}

void
ggv_histogram_init (ggvisd *ggv, ggobid *gg)
{
  gint i;
  dissimd *D = ggv->dissim;

  D->bars = g_malloc (100 * sizeof (GdkRectangle));
  vectorb_alloc (&D->bars_included, 100);
  for (i=0; i<100; i++) D->bars_included.els[i] = true;
  vectori_alloc (&D->bins, 100);
}

void
ggv_Dtarget_histogram_update (ggvisd *ggv, ggobid *gg)
{
  dissimd *D = ggv->dissim;
  GtkWidget *da = D->da;
  gint k;
  gint xmin = HISTOGRAM_HMARGIN;
  gint xmax = da->allocation.width - HISTOGRAM_HMARGIN;
  /*
   * I can't explain this, but somehow this is being called
   * once before init_dissim has been called.
  if (!initd) return;
  */

  histogram_bins_reset (ggv);

  /* And given that the dthresh values have changed, reset grip_pos[] */

  D->lgrip_pos = (gint) (D->low * (gdouble) (xmax - xmin) + (gdouble) xmin);
  D->rgrip_pos = (gint) (D->high * (gdouble) (xmax - xmin) + (gdouble) xmin);

  /* Given that the grip has changed, reset the included bars */

  for (k=0; k<D->nbins; k++) {
    if (D->bars[k].x >= D->lgrip_pos &&
        D->bars[k].x + D->bars[k].width <= D->rgrip_pos)
      D->bars_included.els[k] = true;
    else
      D->bars_included.els[k] = false;
  }

  histogram_make (ggv);
  histogram_draw (ggv, gg);
}
