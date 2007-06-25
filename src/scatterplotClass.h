/* scatterplotClass.h */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#ifndef SCATTERPLOT_CLASS_H
#define SCATTERPLOT_CLASS_H

#include "ggobi.h"


#define GGOBI_TYPE_SCATTER_SPLOT          (ggobi_scatter_splot_get_type())
#define GGOBI_SCATTER_SPLOT(obj)	        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_SCATTER_SPLOT, scatterSPlotd))
#define GGOBI_SCATTER_SPLOT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_SCATTER_SPLOT, GGobiScatterSPlotClass))
#define GGOBI_IS_SCATTER_SPLOT(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_SCATTER_SPLOT))
#define GGOBI_IS_SCATTER_SPLOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_SCATTER_SPLOT))
#define GGOBI_SCATTER_SPLOT_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_SCATTER_SPLOT, GGobiScatterSPlotClass))

GType ggobi_scatter_splot_get_type(void);


typedef struct 
{
    GGobiExtendedSPlotClass parent_class;

} GGobiScatterSPlotClass;

typedef  struct {

    extendedSPlotd extendedSPlot;

} scatterSPlotd;



#define GGOBI_TYPE_SCATTERPLOT_DISPLAY	 (ggobi_scatterplot_display_get_type ())
#define GGOBI_SCATTERPLOT_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_SCATTERPLOT_DISPLAY, scatterplotDisplayd))
#define GGOBI_SCATTERPLOT_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_SCATTERPLOT_DISPLAY, GGobiScatterplotDisplayClass))
#define GGOBI_IS_SCATTERPLOT_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_SCATTERPLOT_DISPLAY))
#define GGOBI_IS_SCATTERPLOT_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_SCATTERPLOT_DISPLAY))
#define GGOBI_SCATTERPLOT_DISPLAY_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_SCATTERPLOT_DISPLAY, GGobiScatterplotDisplayClass))


GType ggobi_scatterplot_display_get_type();
displayd *ggobi_scatterplot_display_new(gint type, gboolean missing_p, GGobiData *d, ggobid *gg);

typedef struct 
{
    GGobiExtendedDisplayClass parent_class;


} GGobiScatterplotDisplayClass;


typedef struct {

  extendedDisplayd extendedDpy;
 
} scatterplotDisplayd;


void scatterSPlotClassInit(GGobiScatterSPlotClass *klass);
void scatterplotDisplayClassInit(GGobiScatterplotDisplayClass *display);
void scatterplotDisplayInit(scatterplotDisplayd *display);


displayd *createScatterplot(displayd *, gboolean use_window, gboolean missing_p, splotd *sp, gint numVars, gint *vars, GGobiData *d, ggobid *gg);

#endif

