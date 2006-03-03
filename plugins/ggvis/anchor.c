#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>
#include <math.h>

#include "plugin.h"
#include "defines.h"
#include "ggvis.h"

static void
recount_anchor_groups (ggvisd *ggv)
{
  gint i, na;

  na = 0;
  for (i=0; i<ggv->anchor_group.nels; i++)
    if (ggv->anchor_group.els[i])
      na++;
  ggv->n_anchors = na;
}


/* response to expose event */
static gint
symbol_show (GtkWidget *w, GdkEventExpose *event, gpointer cbd)
{
  gint k = GPOINTER_TO_INT(cbd);
  PluginInstance *inst = (PluginInstance *)
    g_object_get_data(G_OBJECT(w), "PluginInst");
  ggobid *gg = inst->gg;
  ggvisd *ggv = ggvisFromInst (inst);
  icoords pos;
  glyphd g;
  GGobiData *d = ggv->dpos;
  colorschemed *scheme = gg->activeColorScheme;

  if (d == NULL)
    d = ggv->dsrc;

  if (k >= d->nclusters)
    return false;

  /*-- fill in the background color --*/
  gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle(w->window, gg->plot_GC,
    true, 0, 0,
    w->allocation.width, w->allocation.height);

  /*-- draw the appropriate symbol in the appropriate color --*/
  gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb[d->clusv[k].color]);
  g.type = d->clusv[k].glyphtype;
  g.size = d->clusv[k].glyphsize;

  pos.x = w->allocation.width / 2;
  pos.y = w->allocation.height / 2;
  draw_glyph(w->window, &g, &pos, 0, gg);

  /*-- add outline if symbol is selected --*/
  if (ggv->anchor_group.nels > 0 && ggv->anchor_group.els[k]) {
    gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb_accent);
    gdk_draw_rectangle (w->window, gg->plot_GC, false, 1, 1,
      w->allocation.width-3, w->allocation.height-3);
    gdk_draw_rectangle (w->window, gg->plot_GC, false, 2, 2,
      w->allocation.width-5, w->allocation.height-5);
  }

  return FALSE;
}

/* response to click event */
static gint
anchor_toggle (GtkWidget *w, GdkEvent *event, gpointer cbd)
{
  gint n = GPOINTER_TO_INT(cbd);
  PluginInstance *inst = (PluginInstance *)
    g_object_get_data(G_OBJECT(w), "PluginInst");
  ggvisd *ggv = ggvisFromInst (inst);
  gboolean rval = false;

  if (ggv->anchor_group.nels > n) {
    ggv->anchor_group.els[n] = !ggv->anchor_group.els[n];
    g_signal_emit_by_name (G_OBJECT(w), "expose_event", cbd, 
      (gboolean) &rval);

    recount_anchor_groups (ggv);
  }

  return false;
}


/* build one symbol */
static void
symbol_add (GtkWidget *table, gint k, gint row, gint col, 
  PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  GtkWidget *ebox, *da;
  gint dawidth = 2 * NGLYPHSIZES + 1 + 10;

  ebox = gtk_event_box_new();
  gtk_tooltips_set_tip(GTK_TOOLTIPS(ggv->tips), ebox,
    "Select to add a cluster to the anchor set, deselect to remove it",
    NULL);

  da = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(ebox), da);
  gtk_widget_set_double_buffered(da, false);
  gtk_widget_set_size_request(GTK_WIDGET(da), dawidth, dawidth);

  gtk_widget_set_events(da,
    GDK_EXPOSURE_MASK | GDK_ENTER_NOTIFY_MASK |
    GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);

  g_signal_connect(G_OBJECT(da), "expose_event",
    G_CALLBACK(symbol_show),
    GINT_TO_POINTER(k));
  g_signal_connect(G_OBJECT(da), "button_press_event",
    G_CALLBACK(anchor_toggle),
    GINT_TO_POINTER(k));

  g_object_set_data(G_OBJECT(da), "PluginInst", inst);

  gtk_table_attach (GTK_TABLE (table), ebox, col, col+1, row, row+1,
    (GtkAttachOptions) (GTK_FILL), 
    (GtkAttachOptions) (GTK_FILL),
    1, 1);
}


void
ggv_anchor_table_build (PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  gint row, col, i;
  gint nrows = 2, ncols = 7;
  GGobiData *d;

  if (inst->data == NULL)
    return;

  d = ggv->dpos;
  if (ggv->dpos == NULL)
    d = ggv->dsrc;

  if (ggv->anchor_table != NULL)
    gtk_widget_destroy (ggv->anchor_table);

  /* I won't do anything to change the status of the existing clusters */
  if (ggv->anchor_group.nels < d->nclusters) {
    vectorb_realloc (&ggv->anchor_group, d->nclusters);
  }

  /* this shouldn't change, but it can't hurt ... */
  recount_anchor_groups (ggv);

  ggv->anchor_table =  gtk_table_new (nrows, ncols, true);
  gtk_container_set_border_width (GTK_CONTAINER (ggv->anchor_table), 2);

  col = 0;
  row = 0;
  for (i=0; i<d->nclusters && i<nrows*ncols; i++) {
    symbol_add (ggv->anchor_table, i, row, col, inst);
    col++;
    if (col == ncols) {
      col = 0; 
      row++;
    }
  }

  gtk_container_add (GTK_CONTAINER (ggv->anchor_frame), ggv->anchor_table);
  gtk_widget_show_all (ggv->anchor_table);
}

CHECK_EVENT_SIGNATURE(clusters_changed_cb, clusters_changed_f)
void clusters_changed_cb (ggobid *gg, GGobiData *d, void *inst)
{  /* ignore the datad argument and use ggv->dpos or dsrc */
  ggv_anchor_table_build (inst);
}

