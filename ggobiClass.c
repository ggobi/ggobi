#include "ggobi.h"
#include "GGobiAPI.h"

#include "externs.h"

#include "tsdisplay.h"
#include "tsPlot.h"

#include "barchartDisplay.h"
#include "parcoordsClass.h"
#include "scatterplotClass.h"
#include "scatmatClass.h"


extern gint num_ggobis, totalNumGGobis;
extern ggobid **all_ggobis;

void gtk_ggobi_class_init(GtkGGobiClass * klass);


/**
  This registers and returns a unique Gtk type representing
  the ggobi class.
 */
GtkType gtk_ggobi_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type) {
    static const GtkTypeInfo data_info = {
      "GtkGGobi",
      sizeof(ggobid),
      sizeof(GtkGGobiClass),
      (GtkClassInitFunc) gtk_ggobi_class_init,
      (GtkObjectInitFunc) ggobi_alloc,
      /* reserved_1 */ NULL,
      /* reserved_2 */ NULL,
      (GtkClassInitFunc) NULL,
    };

    data_type = gtk_type_unique(gtk_object_get_type(), &data_info);
  }

  return data_type;
}


/**
   Initialize the GtkGGobi class, called when the type is 
   initially registered with the Gtk mechanism.
   This registers
 */
void gtk_ggobi_class_init(GtkGGobiClass * klass)
{
  if (gtk_signal_lookup("datad_added", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[DATAD_ADDED_SIGNAL] =
        gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI),
                                         "datad_added",
                                         GTK_RUN_LAST | GTK_RUN_ACTION,
                                         gtk_marshal_NONE__POINTER_POINTER,
                                         GTK_TYPE_NONE, 2,
                                         GTK_TYPE_POINTER,
                                         GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup("brush_motion", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[BRUSH_MOTION_SIGNAL] =
        gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI),
                                         "brush_motion",
                                         GTK_RUN_LAST | GTK_RUN_ACTION,
                                         gtk_marshal_NONE__POINTER_POINTER_POINTER,
                                         GTK_TYPE_NONE, 3,
                                         GTK_TYPE_POINTER,
                                         GTK_TYPE_POINTER,
                                         GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup("move_point", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[POINT_MOVE_SIGNAL] =
        gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI),
                                         "move_point",
                                         GTK_RUN_LAST | GTK_RUN_ACTION,
                                         gtk_marshal_NONE__POINTER_POINTER_POINTER,
                                         GTK_TYPE_NONE, 3,
                                         GTK_TYPE_POINTER,
                                         GTK_TYPE_POINTER,
                                         GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup("identify_point", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[IDENTIFY_POINT_SIGNAL] =
        gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI),
                                         "identify_point",
                                         GTK_RUN_LAST | GTK_RUN_ACTION,
                                         gtk_marshal_NONE__POINTER_INT_POINTER,
                                         GTK_TYPE_NONE, 3,
                                         GTK_TYPE_GGOBI_SPLOT,
                                         GTK_TYPE_INT,
                                         GTK_TYPE_GGOBI_DATA);
  }

  /* This should be for a ggobi datad rather than a widget. Make that a
     GtkObject and give it a type. */
  if (gtk_signal_lookup("select_variable", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[VARIABLE_SELECTION_SIGNAL] =
        gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI),
                                         "select_variable",
                                         GTK_RUN_LAST | GTK_RUN_ACTION,
                                         gtk_marshal_NONE__INT_POINTER_POINTER_POINTER,
                                         GTK_TYPE_NONE, 4, GTK_TYPE_INT,
                                         GTK_TYPE_POINTER,
                                         GTK_TYPE_POINTER,
                                         GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup("splot_new", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[SPLOT_NEW_SIGNAL] =
        gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI),
                                         "splot_new",
                                         GTK_RUN_LAST | GTK_RUN_ACTION,
                                         gtk_marshal_NONE__POINTER_POINTER,
                                         GTK_TYPE_NONE, 2,
                                         GTK_TYPE_POINTER,
                                         GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup("variable_added", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[VARIABLE_ADDED_SIGNAL] =
        gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI),
                                         "variable_added",
                                         GTK_RUN_LAST | GTK_RUN_ACTION,
                                         gtk_marshal_NONE__POINTER_POINTER,
                                         GTK_TYPE_NONE, 2,
                                         GTK_TYPE_POINTER,
                                         GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup("variable_list_changed", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[VARIABLE_LIST_CHANGED_SIGNAL] =
        gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI),
                                         "variable_list_changed",
                                         GTK_RUN_LAST | GTK_RUN_ACTION,
                                         gtk_marshal_NONE__POINTER_POINTER,
                                         GTK_TYPE_NONE, 2,
                                         GTK_TYPE_POINTER,
                                         GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup("sticky_point_added", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[STICKY_POINT_ADDED_SIGNAL] = 
	    gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI), 
					     "sticky_point_added", 
					     GTK_RUN_LAST | GTK_RUN_ACTION, 
					     gtk_marshal_NONE__INT_INT_POINTER,
					     GTK_TYPE_NONE, 3, 
					     GTK_TYPE_INT, GTK_TYPE_INT, GTK_TYPE_POINTER); /* record index and datad pointer * */
  }

  if (gtk_signal_lookup("sticky_point_removed", GTK_TYPE_GGOBI) == 0) {
	  GGobiSignals[STICKY_POINT_REMOVED_SIGNAL] = 
		  gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI), 
						   "sticky_point_removed", 
						   GTK_RUN_LAST | GTK_RUN_ACTION, 
						   gtk_marshal_NONE__INT_INT_POINTER, 
						   GTK_TYPE_NONE, 3, 
						   GTK_TYPE_INT, GTK_TYPE_INT, GTK_TYPE_POINTER);     /* record index and datad pointer * */
  }
}

/****************************/


void gtk_ggobi_data_class_init(GtkGGobiDataClass * klass)
{

}

GtkType gtk_ggobi_data_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type) {
    static const GtkTypeInfo data_info = {
      "GtkGGobiData",
      sizeof(datad),
      sizeof(GtkGGobiDataClass),
      (GtkClassInitFunc) gtk_ggobi_data_class_init,
      (GtkObjectInitFunc) datad_instance_init,
      /* reserved_1 */ NULL,
      /* reserved_2 */ NULL,
      (GtkClassInitFunc) NULL,
    };

    data_type = gtk_type_unique(gtk_object_get_type(), &data_info);
  }

  return data_type;
}


datad *gtk_ggobi_data_new(ggobid * gg)
{
  datad *d;
  d = gtk_type_new(GTK_TYPE_GGOBI_DATA);
  datad_new(d, gg);
  return (d);
}

datad *gtk_ggobi_data_new_with_dimensions(int nr, int nc, ggobid * gg)
{
  datad *d;
  gtk_ggobi_data_get_type();
  d = datad_create(nr, nc, gg);
  return (d);
}

void datad_instance_init(datad * d)
{
/*  memset(d, 0, sizeof(datad)); */

  /*-- initialize arrays to NULL --*/
  arrayf_init_null(&d->raw);
  arrayf_init_null(&d->tform);
  arrayg_init_null(&d->world);
  arrayg_init_null(&d->jitdata);

  arrays_init_null(&d->missing);

  vectori_init_null(&d->clusterid);

  /*-- brushing and linking --*/
  rowids_init_null(d);
  vectorb_init_null(&d->edge.xed_by_brush);

  /*-- linking by categorical variable --*/
  d->linkvar_vt = NULL;

  sphere_init(d);
}

/******************************************************/

#ifdef TEST_DESTROY
static void
testDisplayDestroy(GtkObject *obj)
{
  GtkObjectClass *klass;
  g_print("In testDisplayDestroy\n");
    klass = GTK_OBJECT_CLASS(gtk_type_class(GTK_TYPE_VBOX));
    if(klass->destroy)
       klass->destroy(obj);

}
#endif

static void 
gtk_ggobi_display_class_init(GtkGGobiDisplayClass * klass)
{
#ifdef TEST_DESTROY /* Just here to test the destroy mechanism is working. */
  GTK_OBJECT_CLASS(klass)->destroy = testDisplayDestroy;
#endif
}

static void 
display_init(displayd * display)
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

}



GtkType gtk_ggobi_display_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type) {
    static const GtkTypeInfo data_info = {
      "GtkGGobiDisplay",
      sizeof(struct _displayd),
      sizeof(GtkGGobiDisplayClass),
      (GtkClassInitFunc) gtk_ggobi_display_class_init,
      (GtkObjectInitFunc) display_init,
      /* reserved_1 */ NULL,
      /* reserved_2 */ NULL,
      (GtkClassInitFunc) NULL,
    };

    data_type = gtk_type_unique(gtk_vbox_get_type(), &data_info);
  }

  return data_type;
}


GtkType gtk_ggobi_window_display_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type) {
    static const GtkTypeInfo data_info = {
      "GtkGGobiWindowDisplay",
      sizeof(struct _windowDisplayd),
      sizeof(GtkGGobiWindowDisplayClass),
      (GtkClassInitFunc) NULL,
      (GtkObjectInitFunc) NULL,
      /* reserved_1 */ NULL,
      /* reserved_2 */ NULL,
      (GtkClassInitFunc) NULL,
    };

    data_type = gtk_type_unique(gtk_ggobi_display_get_type(), &data_info);
  }

  return data_type;
}


/***************************/

static void 
gtk_splot_init(splotd * sp)
{
  sp->da = (GtkWidget *) & sp->canvas;

  sp->pixmap0 = NULL;
  sp->pixmap1 = NULL;
  sp->redraw_style = FULL;

  sp->whiskers = NULL;

/*sp->tour1d.firsttime = true; *//* Ensure that the 1D tour should be initialized. */
}

static void
splotDestroy(GtkObject *obj)
{
    GtkObjectClass *klass;
    splotd *sp = GTK_GGOBI_SPLOT(obj);
     /* Can't we just do this in the extended display class, or even the displayd class itself. */
    if(sp->whiskers) {
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


    klass = GTK_OBJECT_CLASS(gtk_type_class(GTK_TYPE_DRAWING_AREA));
    if(klass->destroy)
       klass->destroy(obj);
}

static void 
splotClassInit(GtkGGobiSPlotClass * klass)
{
  klass->redraw = QUICK;
  GTK_OBJECT_CLASS(klass)->destroy = splotDestroy;
}


GtkType 
gtk_ggobi_splot_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type) {
    static const GtkTypeInfo data_info = {
      "GtkGGobiSPlot",
      sizeof(splotd),
      sizeof(GtkGGobiSPlotClass),
      (GtkClassInitFunc) splotClassInit,
      (GtkObjectInitFunc) gtk_splot_init,
      /* reserved_1 */ NULL,
      /* reserved_2 */ NULL,
      (GtkClassInitFunc) NULL,
    };

    data_type = gtk_type_unique(gtk_drawing_area_get_type(), &data_info);
  }

  return data_type;
}


static void 
extendedSPlotClassInit(GtkGGobiExtendedSPlotClass * klass)
{
  klass->tree_label = NULL;
}

GtkType gtk_ggobi_extended_splot_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type) {
    static const GtkTypeInfo data_info = {
      "GtkGGobiExtendedSPlot",
      sizeof(extendedSPlotd),
      sizeof(GtkGGobiExtendedSPlotClass),
      (GtkClassInitFunc) extendedSPlotClassInit,
      (GtkObjectInitFunc) NULL,
      /* reserved_1 */ NULL,
      /* reserved_2 */ NULL,
      (GtkClassInitFunc) NULL,
    };

    data_type = gtk_type_unique(gtk_ggobi_splot_get_type(), &data_info);
  }

  return data_type;
}


/********************************************/

static void extendedDisplayInit(extendedDisplayd * dpy)
{
}

static GtkWidget *getExtendedDisplayCPanelWidget(displayd * dpy,
                                                 gint viewmode,
                                                 gchar ** modeName,
                                                 ggobid * gg)
{
  *modeName = "Unknown mode!";
  return (GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget);
}

static void extendedDisplayClassInit(GtkGGobiExtendedDisplayClass * klass)
{
  klass->viewmode_control_box = getExtendedDisplayCPanelWidget;
  klass->options_menu_p = true;
  klass->allow_reorientation = true;
  klass->binning_ok = true;

  /* DFS noticed that if this is false, no display is drawn. */
  klass->loop_over_points = true;
}


GtkType gtk_ggobi_extended_display_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type) {
    static const GtkTypeInfo data_info = {
      "GtkGGobiExtendedDisplay",
      sizeof(extendedDisplayd),
      sizeof(GtkGGobiExtendedDisplayClass),
      (GtkClassInitFunc) extendedDisplayClassInit,
      (GtkObjectInitFunc) extendedDisplayInit,
      /* reserved_1 */ NULL,
      /* reserved_2 */ NULL,
      (GtkClassInitFunc) NULL,
    };

    data_type =
        gtk_type_unique(gtk_ggobi_window_display_get_type(), &data_info);
  }

  return data_type;
}


static void timeSeriesDisplayInit(timeSeriesDisplayd * dpy)
{
  dpy->extendedDpy.titleLabel = NULL;
}

GtkType gtk_ggobi_time_series_display_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type) {
    static const GtkTypeInfo data_info = {
      "GtkGGobiTimeSeriesDisplay",
      sizeof(timeSeriesDisplayd),
      sizeof(GtkGGobiTimeSeriesDisplayClass),
      (GtkClassInitFunc) timeSeriesClassInit,
      (GtkObjectInitFunc) timeSeriesDisplayInit,
      /* reserved_1 */ NULL,
      /* reserved_2 */ NULL,
      (GtkClassInitFunc) NULL,
    };

    data_type =
        gtk_type_unique(gtk_ggobi_extended_display_get_type(), &data_info);
  }

  return data_type;
}


/***********************************************************************/

#ifdef BARCHART_IMPLEMENTED

static void barchartDisplayInit(barchartDisplayd * dpy)
{
   dpy->extendedDpy.titleLabel = NULL;
}

/**
 This is where we register the barchart class with the Gtk type/class system.
This is invoked "transparently" when we use the GTK_GGOBI... macros
 */
GtkType
gtk_ggobi_barchart_display_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
	    /* only register once. */
      static const GtkTypeInfo data_info =
      {
        "GtkGGobiBarChartDisplay", /* Name of the class */
        sizeof (barchartDisplayd),       /* size of the instance of this class. */
	sizeof (GtkGGobiBarChartDisplayClass), /* size of the class definition itself, methods, etc.*/
	(GtkClassInitFunc) barchartDisplayClassInit, /* routine to initialize the class, set the method pointers and constants */
	(GtkObjectInitFunc) barchartDisplayInit, 
/* very basic routine to initialize an instance of this class, 
   after it is allocated by the Gtk system using gtk_type_new(). 
   Typically we will have a higher level routine say, gtk_<myclass>_new_with...() 
   which will in turn call gtk_type_new() and then initializes the structure with
   its own arguments.
*/
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };


    data_type =
        gtk_type_unique(gtk_ggobi_extended_display_get_type(), &data_info);
  }

  return data_type;
}


static void barchartSPlotInit(barchartSPlotd * sp)
{
  sp->bar = (barchartd *) g_malloc(1 * sizeof(barchartd));
  sp->bar->index_to_rank = NULL;
  sp->bar->is_spine = FALSE;

  barchart_init_vectors(sp);
}

GtkType gtk_ggobi_barchart_splot_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type) {
    static const GtkTypeInfo data_info = {
      "GtkGGobiBarChartSPlot",
      sizeof(barchartSPlotd),
      sizeof(GtkGGobiBarChartSPlotClass),
      (GtkClassInitFunc) barchartSPlotClassInit,
      (GtkObjectInitFunc) barchartSPlotInit,
      /* reserved_1 */ NULL,
      /* reserved_2 */ NULL,
      (GtkClassInitFunc) NULL,
    };

    data_type =
        gtk_type_unique(gtk_ggobi_extended_splot_get_type(), &data_info);
  }

  return data_type;
}

#endif                          /* BARCHART_IMPLEMENTED */


#if 0
allocCPanels(GTK_GGOBI_EXTENDED_DISPLAY(dpy));
void allocCPanels(extendedDisplayd * dpy)
{
  GtkGGobiExtendedDisplayClass *klass;
  klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(dpy)->klass);

  if (klass->numControlPanels > 0)
    dpy->cpanel =
        (cpaneld **) g_malloc(sizeof(cpaneld *) * klass->numControlPanels);
}

#endif



/**************************************************************************/

GtkType gtk_ggobi_time_series_splot_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type) {
    static const GtkTypeInfo data_info = {
      "GtkGGobiTimeSeriesSPlot",
      sizeof(timeSeriesSPlotd),
      sizeof(GtkGGobiTimeSeriesSPlotClass),
      (GtkClassInitFunc) timeSeriesSPlotClassInit,
      (GtkObjectInitFunc) NULL,
      /* reserved_1 */ NULL,
      /* reserved_2 */ NULL,
      (GtkClassInitFunc) NULL,
    };

    data_type =
        gtk_type_unique(gtk_ggobi_extended_splot_get_type(), &data_info);
  }

  return data_type;
}


/**************************************/

GtkType gtk_ggobi_par_coords_display_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type) {
    static const GtkTypeInfo data_info = {
      "GtkGGobiParCoordsDisplay",
      sizeof(parcoordsDisplayd),
      sizeof(GtkGGobiParCoordsDisplayClass),
      (GtkClassInitFunc) parcoordsDisplayClassInit,
      (GtkObjectInitFunc) parcoordsDisplayInit,
      /* reserved_1 */ NULL,
      /* reserved_2 */ NULL,
      (GtkClassInitFunc) NULL,
    };

    data_type =
        gtk_type_unique(gtk_ggobi_extended_display_get_type(), &data_info);
  }

  return data_type;
}


GtkType gtk_ggobi_par_coords_splot_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type) {
    static const GtkTypeInfo data_info = {
      "GtkGGobiParCoordsSPlot",
      sizeof(parcoordsSPlotd),
      sizeof(GtkGGobiParCoordsSPlotClass),
      (GtkClassInitFunc) parcoordsSPlotClassInit,
      (GtkObjectInitFunc) NULL,
      /* reserved_1 */ NULL,
      /* reserved_2 */ NULL,
      (GtkClassInitFunc) NULL,
    };

    data_type =
        gtk_type_unique(gtk_ggobi_extended_splot_get_type(), &data_info);
  }

  return data_type;
}

/********************************/

GtkType 
gtk_ggobi_scatter_splot_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiScatterSPlot",
	sizeof (scatterSPlotd),
	sizeof (GtkGGobiScatterSPlotClass),
	(GtkClassInitFunc) scatterSPlotClassInit,
	(GtkObjectInitFunc) NULL,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_ggobi_extended_splot_get_type (), &data_info);
    }

  return data_type;
}



GtkType
gtk_ggobi_scatterplot_display_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiScatterplotDisplay",
	sizeof (scatterplotDisplayd),
	sizeof (GtkGGobiScatterplotDisplayClass),
	(GtkClassInitFunc) scatterplotDisplayClassInit,
	(GtkObjectInitFunc) scatterplotDisplayInit,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_ggobi_extended_display_get_type (), &data_info);
    }

  return data_type;
}





/******************************************/

GtkType 
gtk_ggobi_scatmat_splot_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiScatmatSPlot",
	sizeof (scatmatSPlotd),
	sizeof (GtkGGobiScatmatSPlotClass),
	(GtkClassInitFunc) scatmatSPlotClassInit,
	(GtkObjectInitFunc) NULL,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_ggobi_extended_splot_get_type (), &data_info);
    }

  return data_type;
}

GtkType
gtk_ggobi_scatmat_display_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiScatmatDisplay",
	sizeof (scatmatDisplayd),
	sizeof (GtkGGobiScatmatDisplayClass),
	(GtkClassInitFunc) scatmatDisplayClassInit,
	(GtkObjectInitFunc) NULL,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_ggobi_extended_display_get_type (), &data_info);
    }

  return data_type;
}
