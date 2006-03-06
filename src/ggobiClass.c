/* ggobiClass.c */
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

#include <string.h>

#include "ggobi.h"
#include "GGobiAPI.h"

#include "externs.h"

#include "tsdisplay.h"
#include "tsPlot.h"

#include "barchartDisplay.h"
#include "parcoordsClass.h"
#include "scatterplotClass.h"
#include "scatmatClass.h"

#include "marshal.h"

extern gint num_ggobis, totalNumGGobis;
extern ggobid **all_ggobis;

void ggobi_ggobi_class_init (GGobiGGobiClass * klass);


/**
  This registers and returns a unique GGobi type representing
  the ggobi class.
 */
GType
ggobi_ggobi_get_type (void)
{
  static GType ggobi_type = 0;

  if (!ggobi_type) {
    static const GTypeInfo ggobi_info = {
      sizeof (GGobiGGobiClass),
      NULL, NULL,
      (GClassInitFunc) ggobi_ggobi_class_init,
      NULL, NULL,
      sizeof (ggobid), 0,
      (GInstanceInitFunc) ggobi_alloc,
      NULL
    };

    ggobi_type =
      g_type_register_static (G_TYPE_OBJECT, "GGobi", &ggobi_info, 0);
  }

  return ggobi_type;
}


/**
   Initialize the GGobiGGobi class, called when the type is 
   initially registered with the GGobi mechanism.
   This registers
 */
void
ggobi_ggobi_class_init (GGobiGGobiClass * klass)
{
  if (g_signal_lookup ("datad_added", GGOBI_TYPE_GGOBI) == 0) {
    GGobiSignals[DATAD_ADDED_SIGNAL] =
      g_signal_new ("datad_added",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL,
                    g_cclosure_marshal_VOID__OBJECT,
                    G_TYPE_NONE, 1, GGOBI_TYPE_DATA);
  }

  if (g_signal_lookup ("brush_motion", GGOBI_TYPE_GGOBI) == 0) {
    GGobiSignals[BRUSH_MOTION_SIGNAL] = g_signal_new ("brush_motion", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL, ggobi_marshal_VOID__OBJECT_POINTER_OBJECT, G_TYPE_NONE, 3, GGOBI_TYPE_SPLOT, G_TYPE_POINTER, /* GdkEventMotion pointer */
                                                      GGOBI_TYPE_DATA);
  }

  if (g_signal_lookup ("move_point", GGOBI_TYPE_GGOBI) == 0) {
    GGobiSignals[POINT_MOVE_SIGNAL] =
      g_signal_new ("move_point",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL,
                    ggobi_marshal_VOID__OBJECT_INT_OBJECT,
                    G_TYPE_NONE, 3,
                    GGOBI_TYPE_SPLOT, G_TYPE_INT, GGOBI_TYPE_DATA);
  }

  if (g_signal_lookup ("identify_point", GGOBI_TYPE_GGOBI) == 0) {
    GGobiSignals[IDENTIFY_POINT_SIGNAL] =
      g_signal_new ("identify_point",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL,
                    ggobi_marshal_VOID__OBJECT_INT_OBJECT,
                    G_TYPE_NONE, 3,
                    GGOBI_TYPE_SPLOT, G_TYPE_INT, GGOBI_TYPE_DATA);
  }

  /* This should be for a ggobi datad rather than a widget. Make that a
     GObject and give it a type. */
  if (g_signal_lookup ("select_variable", GGOBI_TYPE_GGOBI) == 0) {
    GGobiSignals[VARIABLE_SELECTION_SIGNAL] =
      g_signal_new ("select_variable",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL,
                    ggobi_marshal_VOID__OBJECT_INT_OBJECT,
                    G_TYPE_NONE, 3,
                    GGOBI_TYPE_DATA, G_TYPE_INT, GGOBI_TYPE_SPLOT);
  }

  if (g_signal_lookup ("splot_new", GGOBI_TYPE_GGOBI) == 0) {
    GGobiSignals[SPLOT_NEW_SIGNAL] =
      g_signal_new ("splot_new",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL,
                    g_cclosure_marshal_VOID__OBJECT,
                    G_TYPE_NONE, 1, GGOBI_TYPE_SPLOT);
  }

  if (g_signal_lookup ("variable_added", GGOBI_TYPE_GGOBI) == 0) {
    GGobiSignals[VARIABLE_ADDED_SIGNAL] = g_signal_new ("variable_added", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL, ggobi_marshal_VOID__POINTER_INT_OBJECT, G_TYPE_NONE, 3, G_TYPE_POINTER,  /*vartabled XX */
                                                        G_TYPE_INT, /*index variable */
                                                        GGOBI_TYPE_DATA);
  }

  if (g_signal_lookup ("variable_list_changed", GGOBI_TYPE_GGOBI) == 0) {
    GGobiSignals[VARIABLE_LIST_CHANGED_SIGNAL] =
      g_signal_new ("variable_list_changed",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL,
                    g_cclosure_marshal_VOID__OBJECT,
                    G_TYPE_NONE, 1, GGOBI_TYPE_DATA);
  }

  if (g_signal_lookup ("sticky_point_added", GGOBI_TYPE_GGOBI) == 0) {
    GGobiSignals[STICKY_POINT_ADDED_SIGNAL] = g_signal_new ("sticky_point_added", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL, ggobi_marshal_VOID__INT_INT_OBJECT, G_TYPE_NONE, 3, G_TYPE_INT, G_TYPE_INT, GGOBI_TYPE_DATA);  /* record index and datad pointer */
  }

  if (g_signal_lookup ("sticky_point_removed", GGOBI_TYPE_GGOBI) == 0) {
    GGobiSignals[STICKY_POINT_REMOVED_SIGNAL] = g_signal_new ("sticky_point_removed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL, ggobi_marshal_VOID__INT_INT_OBJECT, G_TYPE_NONE, 3, G_TYPE_INT, G_TYPE_INT, GGOBI_TYPE_DATA);  /* record index and datad pointer */
  }

  if (g_signal_lookup ("clusters_changed", GGOBI_TYPE_GGOBI) == 0) {
    GGobiSignals[CLUSTERS_CHANGED_SIGNAL] = g_signal_new ("clusters_changed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, GGOBI_TYPE_DATA); /* datad pointer */
  }


  if (g_signal_lookup ("display_new", GGOBI_TYPE_GGOBI) == 0) {
    GGobiSignals[DISPLAY_NEW_SIGNAL] = g_signal_new ("display_new", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, GGOBI_TYPE_DISPLAY);  /* displayd pointer */
  }


  /* This signal is to be emitted by display_set_current, and picked
     up by the console. */
  if (g_signal_lookup ("display_selected", GGOBI_TYPE_GGOBI) == 0) {
    GGobiSignals[DISPLAY_SELECTED_SIGNAL] = g_signal_new ("display_selected", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, GGOBI_TYPE_DISPLAY);  /* displayd pointer */
  }

  /*int i;
     for (i = 0; i < MAX_GGOBI_SIGNALS; i++) {
     printf("%d\n", GGobiSignals[i]);
     } */
}

/****************************/



/******************************************************/

#ifdef TEST_DESTROY
static void
testDisplayDestroy (GGobiObject * obj)
{
  GGobiObjectClass *klass;
  g_print ("In testDisplayDestroy\n");
  klass = GTK_OBJECT_CLASS (G_TYPE_VBOX);
  if (klass->destroy)
    klass->destroy (obj);

}
#endif

static void
ggobi_display_class_init (GGobiDisplayClass * klass)
{
#ifdef TEST_DESTROY             /* Just here to test the destroy mechanism is working. */
  G_OBJECT_CLASS (klass)->destroy = testDisplayDestroy;
#endif

  if (g_signal_lookup ("tour_step", GGOBI_TYPE_DISPLAY) == 0) {
    klass->signals[TOUR_STEP_SIGNAL] =
      g_signal_new ("tour_step",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL,
                    ggobi_marshal_VOID__POINTER_INT_OBJECT,
                    G_TYPE_NONE, 3,
                    G_TYPE_POINTER, G_TYPE_INT, GGOBI_TYPE_GGOBI);
  }
}

static void
display_init (displayd * display)
{

  display->e = NULL;

  /*-- for dragging in the rulers --*/
  display->drag_start.x = display->drag_start.y = 0;

  display->t1d_manip_var = -1;
  display->t2d_manip_var = -1;
  display->tc1_manip_var = -1;
  display->tc2_manip_var = -1;

  display->t1d_window = NULL;
  display->t2d_window = NULL;
  display->t1d_pp_pixmap = NULL;
  display->t2d_pp_pixmap = NULL;

  display->pmode_merge_id = display->imode_merge_id = 0;
}



GType
ggobi_display_get_type (void)
{
  static GType display_type = 0;

  if (!display_type) {
    static const GTypeInfo display_info = {
      sizeof (GGobiDisplayClass),
      NULL, NULL,
      (GClassInitFunc) ggobi_display_class_init,
      NULL, NULL,
      sizeof (struct _displayd), 0,
      (GInstanceInitFunc) display_init,
      NULL
    };

    display_type =
      g_type_register_static (GTK_TYPE_VBOX, "GGobiDisplay", &display_info,
                              0);
  }

  return display_type;
}


void
initWindowDisplayd (windowDisplayd * dpy)
{
  GGOBI_WINDOW_DISPLAY (dpy)->useWindow = true;
}

GType
ggobi_window_display_get_type (void)
{
  static GType window_type = 0;

  if (!window_type) {
    static const GTypeInfo window_info = {
      sizeof (GGobiWindowDisplayClass),
      NULL, NULL,
      (GClassInitFunc) NULL,
      NULL, NULL,
      sizeof (struct _windowDisplayd), 0,
      (GInstanceInitFunc) initWindowDisplayd,
      NULL
    };

    window_type =
      g_type_register_static (GGOBI_TYPE_DISPLAY, "GGobiWindowDisplay",
                              &window_info, 0);
  }

  return window_type;
}


GType
ggobi_embedded_display_get_type (void)
{
  static GType embedded_type = 0;

  if (!embedded_type) {
    static const GTypeInfo embedded_info = {
      sizeof (GGobiEmbeddedDisplayClass),
      NULL, NULL,
      (GClassInitFunc) NULL,
      NULL, NULL,
      sizeof (struct _embeddedDisplayd), 0,
      (GInstanceInitFunc) NULL,
      NULL
    };

    embedded_type =
      g_type_register_static (GGOBI_TYPE_DISPLAY, "GGobiEmbeddedDisplay",
                              &embedded_info, 0);
  }

  return (embedded_type);
}


/***************************/

static void
ggobi_splot_init (splotd * sp)
{
  sp->da = (GtkWidget *) & sp->canvas;

  sp->pixmap0 = NULL;
  sp->pixmap1 = NULL;
  sp->redraw_style = FULL;

  sp->whiskers = NULL;

  /*sp->tour1d.firsttime = true; *//* Ensure that the 1D tour should be initialized. */
}

/* In a version of a GGobi on my (DTL) Mac, there is no definition for GTK_TYPE_DRAWING_AREA
   I would be surprised if this bug still exists - mfl
#ifndef GTK_TYPE_DRAWING_AREA
#define GTK_TYPE_DRAWING_AREA gtk_drawing_area_get_type
#endif
*/

static void
splotDestroy (GtkObject * obj)
{
  GtkObjectClass *klass;
  splotd *sp = GGOBI_SPLOT (obj);
  /* Can't we just do this in the extended display class, or even the displayd class itself. */
  if (sp->whiskers) {
    g_free ((gpointer) sp->whiskers);
    sp->whiskers = NULL;
  }
  if (sp->edges != NULL) {
    g_free ((gpointer) sp->edges);
    sp->edges = NULL;
  }
  if (sp->arrowheads != NULL) {
    g_free ((gpointer) sp->arrowheads);
    sp->arrowheads = NULL;
  }

  sp->da = NULL;

  klass = GTK_OBJECT_CLASS (g_type_class_peek (GTK_TYPE_DRAWING_AREA));
  if (klass->destroy)
    klass->destroy (obj);
}

static void
splotClassInit (GGobiSPlotClass * klass)
{
  klass->redraw = QUICK;
  GTK_OBJECT_CLASS (klass)->destroy = splotDestroy;
}


GType
ggobi_splot_get_type (void)
{
  static GType splot_type = 0;

  if (!splot_type) {
    static const GTypeInfo splot_info = {
      sizeof (GGobiSPlotClass),
      NULL, NULL,
      (GClassInitFunc) splotClassInit,
      NULL, NULL,
      sizeof (splotd), 0,
      (GInstanceInitFunc) ggobi_splot_init,
      NULL
    };

    splot_type =
      g_type_register_static (GTK_TYPE_DRAWING_AREA, "GGobiSPlot",
                              &splot_info, 0);
  }

  return splot_type;
}


static void
extendedSPlotClassInit (GGobiExtendedSPlotClass * klass)
{
  klass->tree_label = NULL;
  klass->createWithVars = NULL;
}


GType
ggobi_extended_splot_get_type (void)
{
  static GType splot_type = 0;

  if (!splot_type) {
    static const GTypeInfo splot_info = {
      sizeof (GGobiExtendedSPlotClass),
      NULL, NULL,
      (GClassInitFunc) extendedSPlotClassInit,
      NULL, NULL,
      sizeof (extendedSPlotd), 0,
      (GInstanceInitFunc) NULL,
      NULL
    };

    splot_type =
      g_type_register_static (GGOBI_TYPE_SPLOT, "GGobiExtendedSPlot",
                              &splot_info, 0);
  }

  return splot_type;
}


/********************************************/

static void
extendedDisplayInit (extendedDisplayd * dpy)
{
}

static GtkWidget *
getExtendedDisplayCPanelWidget (displayd * dpy,
                                gchar ** modeName, ggobid * gg)
{
  *modeName = "Unknown mode!";
  return (GGOBI_EXTENDED_DISPLAY (dpy)->cpanelWidget);
}

static void
extendedDisplayClassInit (GGobiExtendedDisplayClass * klass)
{
  klass->imode_control_box = getExtendedDisplayCPanelWidget;
  klass->options_menu_p = true;
  klass->allow_reorientation = true;
  klass->binning_ok = true;

  /* DFS noticed that if this is false, no display is drawn. */
  klass->loop_over_points = true;

  klass->supports_edges_p = false;
}


GType
ggobi_extended_display_get_type (void)
{
  static GType display_type = 0;

  if (!display_type) {
    static const GTypeInfo display_info = {
      sizeof (GGobiExtendedDisplayClass),
      NULL, NULL,
      (GClassInitFunc) extendedDisplayClassInit,
      NULL, NULL,
      sizeof (extendedDisplayd), 0,
      (GInstanceInitFunc) extendedDisplayInit,
      NULL
    };

    display_type =
      g_type_register_static (GGOBI_TYPE_WINDOW_DISPLAY,
                              "GGobiExtendedDisplay", &display_info, 0);
  }

  return display_type;
}


static void
timeSeriesDisplayInit (timeSeriesDisplayd * dpy)
{
  dpy->extendedDpy.titleLabel = NULL;
}

GType
ggobi_time_series_display_get_type (void)
{
  static GType time_series_type = 0;

  if (!time_series_type) {
    static const GTypeInfo time_series_info = {
      sizeof (GGobiTimeSeriesDisplayClass),
      NULL, NULL,
      (GClassInitFunc) timeSeriesClassInit,
      NULL, NULL,
      sizeof (timeSeriesDisplayd), 0,
      (GInstanceInitFunc) timeSeriesDisplayInit,
      NULL
    };

    time_series_type =
      g_type_register_static (GGOBI_TYPE_EXTENDED_DISPLAY,
                              "GGobiTimeSeriesDisplay", &time_series_info, 0);
  }

  return time_series_type;
}


/***********************************************************************/

static void
barchartDisplayInit (barchartDisplayd * dpy)
{
  dpy->extendedDpy.titleLabel = NULL;
}

/**
 This is where we register the barchart class with the GGobi type/class system.
This is invoked "transparently" when we use the GGOBI_TYPE_... macros
 */
GType
ggobi_barchart_display_get_type (void)
{
  static GType barchart_type = 0;

  if (!barchart_type) {
    /* only register once. */
    static const GTypeInfo barchart_info = {
      sizeof (GGobiBarChartDisplayClass), /* size of the class definition itself, methods, etc. */
      NULL, NULL,
      (GClassInitFunc) barchartDisplayClassInit,  /* routine to initialize the class, set the method pointers and constants */
      NULL, NULL,
      sizeof (barchartDisplayd) /* size of the instance of this class. */ , 0,
      (GInstanceInitFunc) barchartDisplayInit,
/* very basic routine to initialize an instance of this class, 
   after it is allocated by the GGobi system using g_object_new(). 
   Typically we will have a higher level routine say, ggobi_<myclass>_new_with...() 
   which will in turn call g_object_new() and then initializes the structure with
   its own arguments.
*/
      NULL
    };


    barchart_type =
      g_type_register_static (GGOBI_TYPE_EXTENDED_DISPLAY,
                              "GGobiBarChartDisplay", &barchart_info, 0);
  }

  return barchart_type;
}


static void
barchartSPlotInit (barchartSPlotd * sp)
{
  sp->bar = (barchartd *) g_malloc (1 * sizeof (barchartd));
  vectori_init_null (&sp->bar->index_to_rank);
  sp->bar->is_spine = FALSE;

  barchart_init_vectors (sp);
}

GType
ggobi_barchart_splot_get_type (void)
{
  static GType barchart_type = 0;

  if (!barchart_type) {
    static const GTypeInfo barchart_info = {
      sizeof (GGobiBarChartSPlotClass),
      NULL, NULL,
      (GClassInitFunc) barchartSPlotClassInit,
      NULL, NULL,
      sizeof (barchartSPlotd), 0,
      (GInstanceInitFunc) barchartSPlotInit,
      NULL
    };

    barchart_type =
      g_type_register_static (GGOBI_TYPE_EXTENDED_SPLOT, "GGobiBarChartSPlot",
                              &barchart_info, 0);
  }

  return barchart_type;
}

/**************************************************************************/

GType
ggobi_time_series_splot_get_type (void)
{
  static GType time_series_type = 0;

  if (!time_series_type) {
    static const GTypeInfo time_series_info = {
      sizeof (GGobiTimeSeriesSPlotClass),
      NULL, NULL,
      (GClassInitFunc) timeSeriesSPlotClassInit,
      NULL, NULL,
      sizeof (timeSeriesSPlotd), 0,
      (GInstanceInitFunc) NULL,
      NULL
    };

    time_series_type =
      g_type_register_static (GGOBI_TYPE_EXTENDED_SPLOT,
                              "GGobiTimeSeriesSPlot", &time_series_info, 0);
  }

  return time_series_type;
}


/**************************************/

GType
ggobi_par_coords_display_get_type (void)
{
  static GType par_coords_type = 0;

  if (!par_coords_type) {
    static const GTypeInfo par_coords_info = {
      sizeof (GGobiParCoordsDisplayClass),
      NULL, NULL,
      (GClassInitFunc) parcoordsDisplayClassInit,
      NULL, NULL,
      sizeof (parcoordsDisplayd), 0,
      (GInstanceInitFunc) parcoordsDisplayInit,
      NULL
    };

    par_coords_type =
      g_type_register_static (GGOBI_TYPE_EXTENDED_DISPLAY,
                              "GGobiParCoordsDisplay", &par_coords_info, 0);
  }

  return par_coords_type;
}


GType
ggobi_par_coords_splot_get_type (void)
{
  static GType par_coords_type = 0;

  if (!par_coords_type) {
    static const GTypeInfo par_coords_info = {
      sizeof (GGobiParCoordsSPlotClass),
      NULL, NULL,
      (GClassInitFunc) parcoordsSPlotClassInit,
      NULL, NULL,
      sizeof (parcoordsSPlotd), 0,
      (GInstanceInitFunc) NULL,
      NULL
    };

    par_coords_type =
      g_type_register_static (GGOBI_TYPE_EXTENDED_SPLOT,
                              "GGobiParCoordsSPlot", &par_coords_info, 0);
  }

  return par_coords_type;
}

/********************************/

GType
ggobi_scatter_splot_get_type (void)
{
  static GType scatter_plot_type = 0;

  if (!scatter_plot_type) {
    static const GTypeInfo scatter_plot_info = {
      sizeof (GGobiScatterSPlotClass),
      NULL, NULL,
      (GClassInitFunc) scatterSPlotClassInit,
      NULL, NULL,
      sizeof (scatterSPlotd), 0,
      (GInstanceInitFunc) NULL,
      NULL
    };

    scatter_plot_type =
      g_type_register_static (GGOBI_TYPE_EXTENDED_SPLOT, "GGobiScatterSPlot",
                              &scatter_plot_info, 0);
  }

  return scatter_plot_type;
}



GType
ggobi_scatterplot_display_get_type (void)
{
  static GType scatter_plot_type = 0;

  if (!scatter_plot_type) {
    static const GTypeInfo scatter_plot_info = {
      sizeof (GGobiScatterplotDisplayClass),
      NULL, NULL,
      (GClassInitFunc) scatterplotDisplayClassInit,
      NULL, NULL,
      sizeof (scatterplotDisplayd), 0,
      (GInstanceInitFunc) scatterplotDisplayInit,
      NULL
    };

    scatter_plot_type =
      g_type_register_static (GGOBI_TYPE_EXTENDED_DISPLAY,
                              "GGobiScatterplotDisplay", &scatter_plot_info,
                              0);
  }

  return scatter_plot_type;
}


/******************************************/

GType
ggobi_scatmat_splot_get_type (void)
{
  static GType scatmat_type = 0;

  if (!scatmat_type) {
    static const GTypeInfo scatmat_info = {
      sizeof (GGobiScatmatSPlotClass),
      NULL, NULL,
      (GClassInitFunc) scatmatSPlotClassInit,
      NULL, NULL,
      sizeof (scatmatSPlotd), 0,
      (GInstanceInitFunc) NULL,
      NULL
    };

    scatmat_type =
      g_type_register_static (GGOBI_TYPE_EXTENDED_SPLOT, "GGobiScatmatSPlot",
                              &scatmat_info, 0);
  }

  return scatmat_type;
}

GType
ggobi_scatmat_display_get_type (void)
{
  static GType scatmat_type = 0;

  if (!scatmat_type) {
    static const GTypeInfo scatmat_info = {
      sizeof (GGobiScatmatDisplayClass),
      NULL, NULL,
      (GClassInitFunc) scatmatDisplayClassInit,
      NULL, NULL,
      sizeof (scatmatDisplayd), 0,
      (GInstanceInitFunc) NULL,
      NULL
    };

    scatmat_type =
      g_type_register_static (GGOBI_TYPE_EXTENDED_DISPLAY,
                              "GGobiScatmatDisplay", &scatmat_info, 0);
  }

  return scatmat_type;
}
