#ifndef TS_PLOT_METHODS_H
#define TS_PLOT_METHODS_H

#include "ggobi.h"
#include "tsdisplay.h"

void timeSeriesClassInit(GtkGGobiTimeSeriesDisplayClass *klass);
void timeSeriesSPlotClassInit(GtkGGobiTimeSeriesSPlotClass *klass);
splotd *gtk_time_series_splot_new(displayd *dpy, gint width, gint height, ggobid *gg);

#endif
