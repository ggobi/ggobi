#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "ggvis.h"

gint
ggv_histogram_configure_cb (GtkWidget *w, GdkEventExpose *event,
  PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  gboolean retval = true;

  if (ggv == NULL)  /*-- too early to configure --*/
    return retval;
  if (w->allocation.width < 2 || w->allocation.height < 2)
    return retval;

  if (ggv->histogram_pix != NULL)
    gdk_pixmap_unref (ggv->histogram_pix);
  ggv->histogram_pix = gdk_pixmap_new (w->window,
    w->allocation.width, w->allocation.height, -1);

  gtk_widget_queue_draw (w);

  return retval;
}

static void
histogram_pixmap_clear (ggvisd *ggv, ggobid *gg)
{
  colorschemed *scheme = gg->activeColorScheme;
  GtkWidget *da = ggv->histogram_da;

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (ggv->histogram_pix, gg->plot_GC,
                      TRUE, 0, 0,
                      da->allocation.width,
                      da->allocation.height);
}

void
histogram_pixmap_copy (ggvisd *ggv, ggobid *gg)
{
  GtkWidget *da = ggv->histogram_da;

  /* copy the pixmap to the screen */
  gdk_draw_pixmap (da->window, gg->plot_GC, ggv->histogram_pix,
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
  gboolean retval = true;

  /*-- sanity checks --*/
  if (ggv == NULL)  /*-- too early to expose or configure --*/
    return retval;
  if (ggv->histogram_pix == NULL)
    return retval;
  if (w->allocation.width < 2 || w->allocation.height < 2)
    return retval;

  /*-- copy the pixmap to the screen --*/
  histogram_pixmap_copy (ggv, gg);

  return retval;
}

void update_histogram (ggvisd *ggv)
{
}
