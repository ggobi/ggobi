#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "ggvis.h"


gint
ggv_stressplot_configure_cb (GtkWidget *w, GdkEventExpose *event,
  PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  gboolean retval = true;

  if (ggv == NULL)  /*-- too early to configure --*/
    return retval;
  if (w->allocation.width < 2 || w->allocation.height < 2)
    return retval;

  if (ggv->stressplot_pix != NULL) {
    gdk_pixmap_unref (ggv->stressplot_pix);
  }
  ggv->stressplot_pix = gdk_pixmap_new (w->window,
    w->allocation.width, w->allocation.height, -1);

  gtk_widget_queue_draw (w);

  return retval;
}

static void
stress_pixmap_clear (ggvisd *ggv, ggobid *gg)
{
  colorschemed *scheme = gg->activeColorScheme;
  GtkWidget *da = ggv->stressplot_da;

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (ggv->stressplot_pix, gg->plot_GC,
                      TRUE, 0, 0,
                      da->allocation.width,
                      da->allocation.height);
}

void
stress_pixmap_copy (ggvisd *ggv, ggobid *gg)
{
  GtkWidget *da = ggv->stressplot_da;

  /* copy the pixmap to the screen */
  gdk_draw_pixmap (da->window, gg->plot_GC, ggv->stressplot_pix,
                   0, 0, 0, 0,
                   da->allocation.width,
                   da->allocation.height);
}


gint
ggv_stressplot_expose_cb (GtkWidget *w, GdkEventExpose *event,
  PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  ggvisd *ggv = ggvisFromInst (inst);
  gboolean retval = true;

  /*-- sanity checks --*/
  if (ggv == NULL)  /*-- too early to expose or configure --*/
    return retval;
  if (ggv->stressplot_pix == NULL)
    return retval;
  if (w->allocation.width < 2 || w->allocation.height < 2)
    return retval;

  /*-- clear the pixmap --*/
  stress_pixmap_clear (ggv, gg);

  /*-- do the drawing --*/

  /*-- copy the pixmap to the screen --*/
  stress_pixmap_copy (ggv, gg);

  return retval;
}

void
reinit_stress (ggvisd *ggv, ggobid *gg)
{
  mds_once (false, ggv, gg);
}


void
draw_stress (ggvisd *ggv, ggobid *gg)
{
  gint i, j, npixels, start, end;
  gfloat x, y;
  GdkPoint axes[3];
  gchar *str;
  GdkPoint strPts[NSTRESSVALUES];
  gfloat height;
  GtkWidget *da = ggv->stressplot_da;
  GtkStyle *style = gtk_widget_get_style (da);
  gint lbearing, rbearing, strwidth, ascent, descent;
  colorschemed *scheme = gg->activeColorScheme;

  height = (gfloat)da->allocation.height - 2. * (gfloat)STRESSPLOT_MARGIN;

  str = g_strdup_printf ("%s", ".9999");
  gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
    gtk_style_get_font (style),
#else
    style->font,
#endif
    str, strlen (str),
    &lbearing, &rbearing, &strwidth, &ascent, &descent);
  g_free (str);

  if (ggv->stressplot_pix == NULL)
    return;

  /* plotting one point per pixel ... */
  npixels = MIN(da->allocation.width - 2*STRESSPLOT_MARGIN, ggv->nstressvalues);
  start = MAX(0, ggv->nstressvalues - npixels);
  end = ggv->nstressvalues;

  npixels = 0;
  for (i=start, j=0; i<end; i++, j++) {
    x = (gfloat) j ;
    strPts[j].x = (gint) (x + STRESSPLOT_MARGIN);
    y = (gfloat) (1 - ggv->stressvalues.els[i]) * height;
    strPts[j].y = (gint) (y + STRESSPLOT_MARGIN);
    npixels++;
  }

  /* axes */
  axes[0].x = STRESSPLOT_MARGIN;
  axes[0].y = STRESSPLOT_MARGIN;
  axes[1].x = STRESSPLOT_MARGIN;
  axes[1].y = da->allocation.height - STRESSPLOT_MARGIN;
  axes[2].x = da->allocation.width - STRESSPLOT_MARGIN;
  axes[2].y = da->allocation.height - STRESSPLOT_MARGIN;

  /* stress as a fraction */

  stress_pixmap_clear (ggv, gg);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  gdk_draw_lines (ggv->stressplot_pix, gg->plot_GC, axes, 3);

  if (ggv->nstressvalues > 0) {

    str = g_strdup_printf ("%2.4f",
      ggv->stressvalues.els[ggv->nstressvalues-1]);
    gdk_draw_string (ggv->stressplot_pix,
#if GTK_MAJOR_VERSION == 2
      gtk_style_get_font (style),
#else
      style->font,
#endif
      gg->plot_GC,
      da->allocation.width - 2*STRESSPLOT_MARGIN - strwidth,
      STRESSPLOT_MARGIN,
      str);
    gdk_draw_lines (ggv->stressplot_pix, gg->plot_GC,
      strPts, npixels);
    g_free(str);
  }

  stress_pixmap_copy (ggv, gg);
}

void add_stress_value (gdouble stress, ggvisd *ggv)
{
  gint i;

  if (ggv->nstressvalues == NSTRESSVALUES) {
    for (i=0; i < (NSTRESSVALUES-1); i++) {
      ggv->stressvalues.els[i] = ggv->stressvalues.els[i+1];
    }
    ggv->nstressvalues--;
  }

  ggv->stressvalues.els[ggv->nstressvalues] = stress;
  ggv->nstressvalues++;
}
