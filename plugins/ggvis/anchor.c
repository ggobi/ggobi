#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>
#include <math.h>

#include "plugin.h"
#include "defines.h"
#include "ggvis.h"

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

  /*-- skip datasets without variables --*/
  if (!datad_has_variables (d))
    return;

  arg.name = "n_columns";
  gtk_widget_get (d->cluster_table, &arg);
  nc = (guint) GTK_VALUE_UINT(arg);
  gtk_table_resize (GTK_TABLE(d->cluster_table), d->nclusters+1, nc+1);

  /* for tooltips, put this label in an event box */
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
