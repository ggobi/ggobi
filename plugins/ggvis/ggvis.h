#ifndef GGVIS_H

#include "defines.h"
#include "plugin.h"

#define STRESSPLOT_WIDTH  300
#define STRESSPLOT_HEIGHT 150
#define STRESSPLOT_MARGIN  10

#define HISTOGRAM_WIDTH  250
#define HISTOGRAM_HEIGHT 150
#define HISTOGRAM_MARGIN  10

typedef enum {deflt, within, between, anchorscales, anchorfixed} MDSGroupInd;

typedef struct {

  array_d dist_orig;
  array_d dist;
  array_d pos_orig;
  array_d pos;

  GdkPixmap *stressplot_pix;
  GdkPixmap *histogram_pix;

} ggvisd;


/*----------------------------------------------------------------------*/
/*                          functions                                   */
/*----------------------------------------------------------------------*/

void ggvis_init (ggvisd *);
ggvisd* ggvisFromInst (PluginInstance *inst);
gint stressplot_configure_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);
gint stressplot_expose_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);
gint histogram_configure_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);
gint histogram_expose_cb (GtkWidget *, GdkEventExpose *, PluginInstance *);

#define GGVIS_H
#endif
