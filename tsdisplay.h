#ifndef GGOBI_TSDISPLAY_H
#define GGOBI_TSDISPLAY_H

/**
 This defines a new class of display (GtkGGobiTimeSeriesDisplay)
 which is the top-level container for multiple time series plots.
 This extends the windowed display class (GtkGGobiWindowDisplay).
*/

#define GTK_TYPE_GGOBI_TIME_SERIES_DISPLAY	 (gtk_ggobi_time_series_display_get_type ())
#define GTK_GGOBI_TIME_SERIES_DISPLAY(obj)	 (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_TIME_SERIES_DISPLAY, timeSeriesDisplayd))
#define GTK_GGOBI_TIME_SERIES_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_TIME_SERIES_DISPLAY, GtkGGobiTimeSeriesDisplayClass))
#define GTK_IS_GGOBI_TIME_SERIES_DISPLAY(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_TIME_SERIES_DISPLAY))
#define GTK_IS_GGOBI_TIME_SERIES_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_TIME_SERIES_DISPLAY))

GtkType gtk_ggobi_time_series_display_get_type();
displayd *gtk_ggobi_time_series_display_new(gint type, gboolean missing_p, datad *d, ggobid *gg);

typedef struct 
{
    GtkGGobiExtendedDisplayClass parent_class;


} GtkGGobiTimeSeriesDisplayClass;


typedef struct _timeSeriesDisplayd {
 extendedDisplayd dpy;


 
} timeSeriesDisplayd;


#endif

