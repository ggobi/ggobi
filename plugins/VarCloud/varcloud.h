#ifndef VARCLOUD_H

#include "defines.h"
#include "plugin.h"

typedef struct {

  datad *dsrc;
  gint xcoord, ycoord, var1;

  GtkTooltips *tips;

} vcld;


void vcl_init (vcld *vclg, ggobid *);
vcld * vclFromInst (PluginInstance *inst);
void launch_varcloud_cb (GtkWidget *w, PluginInstance *inst);

#define VARCLOUD_H
#endif
