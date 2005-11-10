/* scatmatClass.h */
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

#ifndef SCATMAT_CLASS_H
#define SCATMAT_CLASS_H

#include "ggobi.h"

#define GGOBI_TYPE_SCATMAT_SPLOT          (ggobi_scatmat_splot_get_type())
#define GGOBI_SCATMAT_SPLOT(obj)	        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_SCATMAT_SPLOT, scatmatSPlotd))
#define GGOBI_SCATMAT_SPLOT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_SCATMAT_SPLOT, GGobiScatmatSPlotClass))
#define GGOBI_IS_SCATMAT_SPLOT(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_SCATMAT_SPLOT))
#define GGOBI_IS_SCATMAT_SPLOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_SCATMAT_SPLOT))
#define GGOBI_SCATMAT_SPLOT_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_SCATMAT_SPLOT, GGobiScatmatSPlotClass))

GType ggobi_scatmat_splot_get_type(void);


typedef struct 
{
    GGobiExtendedSPlotClass parent_class;

} GGobiScatmatSPlotClass;

typedef  struct {

    extendedSPlotd extendedSPlot;

} scatmatSPlotd;




#define GGOBI_TYPE_SCATMAT_DISPLAY	 (ggobi_scatmat_display_get_type ())
#define GGOBI_SCATMAT_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_SCATMAT_DISPLAY, scatmatDisplayd))
#define GGOBI_SCATMAT_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_SCATMAT_DISPLAY, GGobiScatmatDisplayClass))
#define GGOBI_IS_SCATMAT_DISPLAY(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_SCATMAT_DISPLAY))
#define GGOBI_IS_SCATMAT_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_SCATMAT_DISPLAY))
#define GGOBI_SCATMAT_DISPLAY_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_SCATMAT_DISPLAY, GGobiScatmatDisplayClass))


GType ggobi_scatmat_display_get_type();
displayd *ggobi_scatmat_display_new(gint type, gboolean missing_p, datad *d, ggobid *gg);

typedef struct 
{
    GGobiExtendedDisplayClass parent_class;


} GGobiScatmatDisplayClass;


typedef struct {

  extendedDisplayd extendedDpy;
 
} scatmatDisplayd;


void scatmatDisplayClassInit(GGobiScatmatDisplayClass *klass);
void scatmatSPlotClassInit(GGobiScatmatSPlotClass *klass) ;


#endif

