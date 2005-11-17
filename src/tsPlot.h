#ifndef TS_PLOT_METHODS_H
#define TS_PLOT_METHODS_H

#include "ggobi.h"
#include "tsdisplay.h"

void timeSeriesClassInit(GGobiTimeSeriesDisplayClass *klass);
void timeSeriesSPlotClassInit(GGobiTimeSeriesSPlotClass *klass);
splotd *ggobi_time_series_splot_new(displayd *dpy, ggobid *gg);

#endif
