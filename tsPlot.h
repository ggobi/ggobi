#ifndef TS_PLOT_METHODS_H
#define TS_PLOT_METHODS_H

#include "ggobi.h"
#include "tsdisplay.h"

void tsWorldToPlane(splotd *sp, datad *d, ggobid *gg);
splotd *gtk_time_series_splot_new(displayd *dpy, gint width, gint height, ggobid *gg);
void tsDestroy(splotd *sp);
void tsWithinPlaneToScreen(splotd *sp, displayd *display, datad *d, ggobid *gg);
gboolean tsDrawEdge_p(splotd *sp, gint m, datad *d, datad *e, ggobid *gg);
gboolean tsDrawCase_p(splotd *sp, gint m, datad *d, ggobid *gg);
void tsAddPlotLabels(splotd *sp, GdkDrawable *drawable, ggobid *gg) ;
void tsWithinDrawBinned(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc);
void tsShowWhiskers(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc);
GdkSegment * tsAllocWhiskers(splotd *sp, gint nrows, datad *d);
gchar *tsTreeLabel(splotd *sp, datad *d, ggobid *gg);


#endif
