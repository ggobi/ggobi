#ifndef DSPDESC_H

#include "plugin.h"

typedef struct {
  GtkWidget *window;

  gchar *title;
  gchar *filename;

} dspdescd;

dspdescd *dspdescFromInst (PluginInstance *inst);

#define DSPDESC_H
#endif

