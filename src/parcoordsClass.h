/* parcoordsClass.h */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Eclipse Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#ifndef PARCOORDS_CLASS_H
#define PARCOORDS_CLASS_H

#include "ggobi.h"

#define GGOBI_TYPE_PAR_COORDS_SPLOT          (ggobi_par_coords_splot_get_type())
#define GGOBI_PAR_COORDS_SPLOT(obj)	        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_PAR_COORDS_SPLOT, parcoordsSPlotd))
#define GGOBI_PAR_COORDS_SPLOT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_PAR_COORDS_SPLOT, GGobiParCoordsSPlotClass))
#define GGOBI_IS_PAR_COORDS_SPLOT(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_PAR_COORDS_SPLOT))
#define GGOBI_IS_PAR_COORDS_SPLOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_PAR_COORDS_SPLOT))
#define GGOBI_PAR_COORDS_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_PAR_COORDS, GGobiParCoordsClass))

GType ggobi_par_coords_splot_get_type(void);

typedef struct 
{
    GGobiExtendedSPlotClass parent_class;

} GGobiParCoordsSPlotClass;

typedef  struct {

    extendedSPlotd extendedSPlot;

} parcoordsSPlotd;



#define GGOBI_TYPE_PAR_COORDS_DISPLAY	 (ggobi_par_coords_display_get_type ())
#define GGOBI_PAR_COORDS_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_PAR_COORDS_DISPLAY, parcoordsDisplayd))
#define GGOBI_PAR_COORDS_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_PAR_COORDS_DISPLAY, GGobiParCoordsDisplayClass))
#define GGOBI_IS_PAR_COORDS_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_PAR_COORDS_DISPLAY))
#define GGOBI_IS_PAR_COORDS_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_PAR_COORDS_DISPLAY))
#define GGOBI_PAR_COORDS_DISPLAY_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_PAR_COORDS_DISPLAY, GGobiParCoordsDisplayClass))


GType ggobi_par_coords_display_get_type();
displayd *ggobi_par_coords_display_new(gint type, gboolean missing_p, GGobiData *d, ggobid *gg);

typedef struct 
{
    GGobiExtendedDisplayClass parent_class;


} GGobiParCoordsDisplayClass;


typedef struct {

  extendedDisplayd extendedDpy;
 
} parcoordsDisplayd;


void parcoordsDisplayClassInit(GGobiParCoordsDisplayClass *klass);
void parcoordsSPlotClassInit(GGobiParCoordsSPlotClass *klass);
void parcoordsDisplayInit(parcoordsDisplayd *display);
splotd *ggobi_parcoords_splot_new(displayd *dpy, ggobid *gg);

#endif

