#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "graphact.h"

void
ga_leaf_hide_cb (GtkWidget *btn, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  graphactd *ga = graphactFromInst (inst);
}

void
ga_nodes_show_cb (GtkWidget *btn, PluginInstance *inst)
{
}
