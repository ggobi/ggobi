#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>
#include <math.h>

#include "plugin.h"
#include "defines.h"
#include "ggvis.h"

/*
void toggle_group_cb (GtkWidget *w, gpointer cbd)
{
  PluginInstance *inst = (PluginInstance *)
     gtk_object_get_data (GTK_OBJECT (w), "PluginInst");
  ggvisd *ggv = ggvisFromInst (inst);
  gint k = GPOINTER_TO_INT(cbd);

  ggv->anchor_group.els[k] = !ggv->anchor_group.els[k];
}

void
add_anchor_toggles (datad *d, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggobid *gg = inst->gg;
  GtkWidget *toggle, *label;
  gint k;
  guint nc;
  GtkArg arg;
  gboolean rval = false;

  if (!datad_has_variables (d))
    return;

  arg.name = "n_columns";
  gtk_widget_get (d->cluster_table, &arg);
  nc = (guint) GTK_VALUE_UINT(arg);
  gtk_table_resize (GTK_TABLE(d->cluster_table), d->nclusters+1, nc+1);

  label = gtk_label_new ("Anchor");
  gtk_table_attach(GTK_TABLE(d->cluster_table), label,
    nc, nc+1, 0, 1,
    GTK_FILL, GTK_FILL, 5, 2);
  gtk_widget_show (label);

  for (k=0; k<d->nclusters; k++) {
    toggle = gtk_check_button_new ();
    gtk_object_set_data (GTK_OBJECT (toggle), "PluginInst", inst);
    gtk_signal_connect(GTK_OBJECT(toggle), "toggled",
      GTK_SIGNAL_FUNC(toggle_group_cb), GINT_TO_POINTER(k));
    gtk_table_attach(GTK_TABLE(d->cluster_table), toggle,
      nc, nc+1, k + 1, k + 2,
      GTK_FILL, GTK_FILL, 5, 2);
    gtk_widget_show (toggle);
  }
  gtk_signal_emit_by_name(GTK_OBJECT(gg->cluster_ui.window), "expose_event",
    (gpointer) gg, (gpointer) & rval);

  if (ggv->anchor_group.nels < d->nclusters)
    vectorb_realloc (&ggv->anchor_group, d->nclusters);
  for (k=0; k<d->nclusters; k++) ggv->anchor_group.els[k] = false;
}

CHECK_EVENT_SIGNATURE(clusters_changed_cb, clusters_changed_f)
void clusters_changed_cb (ggobid *gg, datad *d, void *inst)
{
  add_anchor_toggles (d, inst);
}
*/

/*------- new: add symbols to ggvis panel ----------*/

gint
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
    gtk_object_get_data (GTK_OBJECT(w), "PluginInst");
  ggobid *gg = inst->gg;
  ggvisd *ggv = ggvisFromInst (inst);
  icoords pos;
  glyphd g;
  datad *d = ggv->dpos;
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
    gtk_object_get_data (GTK_OBJECT(w), "PluginInst");
  ggvisd *ggv = ggvisFromInst (inst);
  gboolean rval = false;

  if (ggv->anchor_group.nels > n) {
    ggv->anchor_group.els[n] = !ggv->anchor_group.els[n];
    gtk_signal_emit_by_name (GTK_OBJECT(w), "expose_event", cbd, 
      (gboolean) &rval);

    ggv->n_anchors = recount_anchor_groups (ggv);
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
#if GTK_MAJOR_VERSION == 2
  gtk_widget_set_double_buffered(da, false);
#endif
  gtk_drawing_area_size(GTK_DRAWING_AREA(da), dawidth, dawidth);

  gtk_widget_set_events(da,
    GDK_EXPOSURE_MASK | GDK_ENTER_NOTIFY_MASK |
    GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);

  gtk_signal_connect(GTK_OBJECT(da), "expose_event",
    GTK_SIGNAL_FUNC(symbol_show),
    GINT_TO_POINTER(k));
  gtk_signal_connect(GTK_OBJECT(da), "button_press_event",
    GTK_SIGNAL_FUNC(anchor_toggle),
    GINT_TO_POINTER(k));

  gtk_object_set_data (GTK_OBJECT(da), "PluginInst", inst);

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
  datad *d;

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
  ggv->n_anchors = recount_anchor_groups (ggv);

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
void clusters_changed_cb (ggobid *gg, datad *d, void *inst)
{  /* ignore the datad argument and use ggv->dpos or dsrc */
  ggv_anchor_table_build (inst);
}

