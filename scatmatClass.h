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

#define GTK_TYPE_GGOBI_SCATMAT_SPLOT          (gtk_ggobi_scatmat_splot_get_type())
#define GTK_GGOBI_SCATMAT_SPLOT(obj)	        (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_SCATMAT_SPLOT, scatmatSPlotd))
#define GTK_GGOBI_SCATMAT_SPLOT_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_SCATMAT_SPLOT, GtkGGobiScatmatSPlotClass))
#define GTK_IS_GGOBI_SCATMAT_SPLOT(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_SCATMAT_SPLOT))
#define GTK_IS_GGOBI_SCATMAT_SPLOT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_SCATMAT_SPLOT))

GtkType gtk_ggobi_scatmat_splot_get_type(void);


typedef struct 
{
    GtkGGobiExtendedSPlotClass parent_class;

} GtkGGobiScatmatSPlotClass;

typedef  struct {

    extendedSPlotd extendedSPlot;

} scatmatSPlotd;




#define GTK_TYPE_GGOBI_SCATMAT_DISPLAY	 (gtk_ggobi_scatmat_display_get_type ())
#define GTK_GGOBI_SCATMAT_DISPLAY(obj)	 (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_SCATMAT_DISPLAY, scatmatDisplayd))
#define GTK_GGOBI_SCATMAT_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_SCATMAT_DISPLAY, GtkGGobiScatmatDisplayClass))
#define GTK_IS_GGOBI_SCATMAT_DISPLAY(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_SCATMAT_DISPLAY))
#define GTK_IS_GGOBI_SCATMAT_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_SCATMAT_DISPLAY))

GtkType gtk_ggobi_scatmat_display_get_type();
displayd *gtk_ggobi_scatmat_display_new(gint type, gboolean missing_p, datad *d, ggobid *gg);

typedef struct 
{
    GtkGGobiExtendedDisplayClass parent_class;


} GtkGGobiScatmatDisplayClass;


typedef struct {

  extendedDisplayd extendedDpy;
 
} scatmatDisplayd;


void scatmatDisplayClassInit(GtkGGobiScatmatDisplayClass *klass);
void scatmatSPlotClassInit(GtkGGobiScatmatSPlotClass *klass) ;


#endif

