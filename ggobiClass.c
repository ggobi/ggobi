#include "ggobi.h"
#include "GGobiAPI.h"

#include "externs.h"

#include "tsdisplay.h"
#include "tsPlot.h"

#include "barchartDisplay.h"

extern gint num_ggobis, totalNumGGobis;
extern ggobid **all_ggobis;

void gtk_ggobi_class_init(GtkGGobiClass *klass);


/**
  This registers and returns a unique Gtk type representing
  the ggobi class.
 */
GtkType
gtk_ggobi_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobi",
	sizeof (ggobid),
	sizeof (GtkGGobiClass),
	(GtkClassInitFunc) gtk_ggobi_class_init,
	(GtkObjectInitFunc) ggobi_alloc,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_object_get_type (), &data_info);
    }

  return data_type;
}


/**
   Initialize the GtkGGobi class, called when the type is 
   initially registered with the Gtk mechanism.
   This registers
 */
void
gtk_ggobi_class_init(GtkGGobiClass *klass)
{
  if (gtk_signal_lookup ("datad_added", GTK_TYPE_GGOBI) == 0) {
      GGobiSignals[DATAD_ADDED_SIGNAL] =
	  gtk_object_class_user_signal_new (gtk_type_class (GTK_TYPE_GGOBI),
					    "datad_added",
					    GTK_RUN_LAST | GTK_RUN_ACTION,
					    gtk_marshal_NONE__POINTER_POINTER,
					    GTK_TYPE_NONE, 2,
					    GTK_TYPE_POINTER, GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup ("brush_motion", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[BRUSH_MOTION_SIGNAL] = 
      gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI),
        "brush_motion", GTK_RUN_LAST|GTK_RUN_ACTION,
        gtk_marshal_NONE__POINTER_POINTER_POINTER, GTK_TYPE_NONE, 3,
        GTK_TYPE_POINTER, GTK_TYPE_POINTER, GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup ("move_point", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[POINT_MOVE_SIGNAL] = 
      gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI),
        "move_point", GTK_RUN_LAST|GTK_RUN_ACTION,
        gtk_marshal_NONE__POINTER_POINTER_POINTER, GTK_TYPE_NONE, 3,
        GTK_TYPE_POINTER, GTK_TYPE_POINTER, GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup ("identify_point", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[IDENTIFY_POINT_SIGNAL] = 
      gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI),
        "identify_point", GTK_RUN_LAST|GTK_RUN_ACTION,
        gtk_marshal_NONE__POINTER_POINTER_POINTER, GTK_TYPE_NONE, 3,
        GTK_TYPE_POINTER, GTK_TYPE_POINTER, GTK_TYPE_POINTER);
  }

    /* This should be for a ggobi datad rather than a widget. Make that a
       GtkObject and give it a type. */
  if (gtk_signal_lookup ("select_variable", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[VARIABLE_SELECTION_SIGNAL] = 
      gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI),
        "select_variable", GTK_RUN_LAST|GTK_RUN_ACTION,
        gtk_marshal_NONE__INT_POINTER_POINTER_POINTER, GTK_TYPE_NONE, 4,
       GTK_TYPE_INT, GTK_TYPE_POINTER, GTK_TYPE_POINTER, GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup ("splot_new", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[SPLOT_NEW_SIGNAL] = 
      gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI),
        "splot_new", GTK_RUN_LAST|GTK_RUN_ACTION,
        gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
        GTK_TYPE_POINTER, GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup ("variable_added", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[VARIABLE_ADDED_SIGNAL] =
      gtk_object_class_user_signal_new (gtk_type_class (GTK_TYPE_GGOBI),
        "variable_added",
        GTK_RUN_LAST | GTK_RUN_ACTION,
        gtk_marshal_NONE__POINTER_POINTER,
        GTK_TYPE_NONE, 2,
        GTK_TYPE_POINTER, GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup ("sticky_point_added", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[STICKY_POINT_ADDED_SIGNAL] =
      gtk_object_class_user_signal_new (gtk_type_class (GTK_TYPE_GGOBI),
        "sticky_point_added",
        GTK_RUN_LAST | GTK_RUN_ACTION,
        gtk_marshal_NONE__INT_INT_POINTER,
        GTK_TYPE_NONE, 3,
        GTK_TYPE_INT, GTK_TYPE_INT, GTK_TYPE_POINTER);  /* record index and datad pointer **/
  }

  if (gtk_signal_lookup ("sticky_point_removed", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[STICKY_POINT_REMOVED_SIGNAL] =
      gtk_object_class_user_signal_new (gtk_type_class (GTK_TYPE_GGOBI),
        "sticky_point_removed",
        GTK_RUN_LAST | GTK_RUN_ACTION,
        gtk_marshal_NONE__INT_INT_POINTER,
        GTK_TYPE_NONE, 3,
        GTK_TYPE_INT, GTK_TYPE_INT, GTK_TYPE_POINTER);  /* record index and datad pointer **/
  }
}

/****************************/


void
gtk_ggobi_data_class_init(GtkGGobiDataClass *klass)
{

}

GtkType
gtk_ggobi_data_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiData",
	sizeof (datad),
	sizeof (GtkGGobiDataClass),
	(GtkClassInitFunc) gtk_ggobi_data_class_init,
	(GtkObjectInitFunc) datad_instance_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_object_get_type (), &data_info);
    }

  return data_type;
}


datad *
gtk_ggobi_data_new(ggobid *gg)
{
  datad *d;
   d = gtk_type_new(GTK_TYPE_GGOBI_DATA);
   datad_new(d, gg);
   return(d);
}

datad *
gtk_ggobi_data_new_with_dimensions(int nr, int nc, ggobid *gg)
{
  datad *d;
  gtk_ggobi_data_get_type();
  d = datad_create(nr, nc, gg);
  return(d);
}

void
datad_instance_init(datad *d)
{
/*  memset(d, 0, sizeof(datad)); */

  /*-- initialize arrays to NULL --*/
  arrayf_init_null (&d->raw);
  arrayf_init_null (&d->tform);
  arrayl_init_null (&d->world);
  arrayl_init_null (&d->jitdata);

  arrays_init_null (&d->missing);

  vectori_init_null (&d->clusterid);

  /*-- brushing and linking --*/
  rowids_init_null (d);
  vectorb_init_null (&d->edge.xed_by_brush);

  /*-- linking by categorical variable --*/
  d->linkvar_vt = NULL;

  sphere_init (d);
}

/******************************************************/

static void
gtk_ggobi_display_class_init(GtkGGobiDisplayClass *klass)
{

}

static void
display_init(displayd *display)
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



GtkType
gtk_ggobi_display_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiDisplay",
	sizeof (struct _displayd),
	sizeof (GtkGGobiDisplayClass),
	(GtkClassInitFunc) gtk_ggobi_display_class_init,
	(GtkObjectInitFunc) display_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_vbox_get_type (), &data_info);
    }

  return data_type;
}


GtkType
gtk_ggobi_window_display_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiWindowDisplay",
	sizeof (struct _windowDisplayd),
	sizeof (GtkGGobiWindowDisplayClass),
	(GtkClassInitFunc) NULL,
	(GtkObjectInitFunc) NULL,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_ggobi_display_get_type (), &data_info);
    }

  return data_type;
}


/***************************/

static void
gtk_splot_init(splotd *sp)
{
    sp->da = (GtkWidget *) &sp->canvas;

    sp->pixmap0 = NULL;
    sp->pixmap1 = NULL;
    sp->redraw_style = FULL;
    /*sp->tour1d.firsttime = true;*//* Ensure that the 1D tour should be initialized. */
}

static void
splotClassInit(GtkGGobiSPlotClass *klass)
{
   klass->redraw = QUICK;
}


GtkType
gtk_ggobi_splot_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiSPlot",
	sizeof (splotd),
	sizeof (GtkGGobiSPlotClass),
	(GtkClassInitFunc) splotClassInit, 
	(GtkObjectInitFunc) gtk_splot_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_drawing_area_get_type (), &data_info);
    }

  return data_type;
}

static void
extendedSPlotClassInit(GtkGGobiExtendedSPlotClass *klass)
{
  klass->tree_label = NULL;
}

GtkType
gtk_ggobi_extended_splot_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiExtendedSPlot",
	sizeof (extendedSPlotd),
	sizeof (GtkGGobiExtendedSPlotClass),
	(GtkClassInitFunc) extendedSPlotClassInit, 
	(GtkObjectInitFunc) NULL,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_ggobi_splot_get_type (), &data_info);
    }

  return data_type;
}


/********************************************/

static void
extendedDisplayInit(extendedDisplayd *dpy)
{
}

static GtkWidget *
getExtendedDisplayCPanelWidget(displayd *dpy, gint viewmode, gchar **modeName, ggobid *gg)
{
  *modeName = "Unknown mode!";
  return(GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget);
}

static void
extendedDisplayClassInit(  GtkGGobiExtendedDisplayClass *klass)
{
   klass->viewmode_control_box = getExtendedDisplayCPanelWidget;
   klass->options_menu_p = true;
   klass->allow_reorientation = true;
   klass->binning_ok = true;

    /* DFS noticed that if this is false, no display is drawn. */
   klass->loop_over_points = true;
}


GtkType
gtk_ggobi_extended_display_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiExtendedDisplay",
	sizeof (extendedDisplayd),
	sizeof (GtkGGobiExtendedDisplayClass),
	(GtkClassInitFunc) extendedDisplayClassInit, 
	(GtkObjectInitFunc) extendedDisplayInit,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_ggobi_window_display_get_type (), &data_info);
    }

  return data_type;
}

static void 
timeSeriesClassInit(GtkGGobiTimeSeriesDisplayClass *klass)
{
/*XX    klass->parent_class.tree_label = tsplot_tree_label; */

    klass->parent_class.treeLabel =  klass->parent_class.titleLabel = "Time Series";
    klass->parent_class.create = timeSeriesDisplayCreate;
    klass->parent_class.variable_select = tsplot_varsel;
    klass->parent_class.variable_plotted_p = tsplotIsVarPlotted;
    klass->parent_class.cpanel_set = tsplotCPanelSet;
    klass->parent_class.display_unset = NULL;
    klass->parent_class.display_set = tsplotDisplaySet;
    klass->parent_class.varpanel_refresh = tsplotVarpanelRefresh;

    klass->parent_class.handles_action = tsplotHandlesAction;

    klass->parent_class.xml_describe = add_xml_tsplot_variables;

    klass->parent_class.varpanel_tooltips_set = tsplotVarpanelTooltipsSet;
    klass->parent_class.plotted_vars_get = tsplotPlottedColsGet;


    klass->parent_class.viewmode_control_box = tsplotCPanelWidget;
    klass->parent_class.menus_make = tsplotMenusMake;

    klass->parent_class.event_handlers_toggle = tsplotEventHandlersToggle;

    klass->parent_class.splot_key_event_handler = tsplotSPlotKeyEventHandler;


    klass->parent_class.add_plot_labels = NULL; 
}


static void
timeSeriesDisplayInit(timeSeriesDisplayd *dpy)
{
   dpy->extendedDpy.titleLabel = "times series";
}

GtkType
gtk_ggobi_time_series_display_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiTimeSerieDisplay",
	sizeof (timeSeriesDisplayd),
	sizeof (GtkGGobiTimeSeriesDisplayClass),
	(GtkClassInitFunc) timeSeriesClassInit,
	(GtkObjectInitFunc) timeSeriesDisplayInit,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_ggobi_extended_display_get_type (), &data_info);
    }

  return data_type;
}


/***********************************************************************/

#ifdef BARCHART_IMPLEMENTED 

static void 
barchartDisplayClassInit(GtkGGobiBarChartDisplayClass *klass)
{
    klass->parent_class.treeLabel = klass->parent_class.titleLabel = "Barchart";
    klass->parent_class.create = barchart_new;
    klass->parent_class.variable_select = barchartVarSel;
    klass->parent_class.variable_plotted_p = barchartVarIsPlotted;
    klass->parent_class.cpanel_set = barchartCPanelSet;
    klass->parent_class.display_unset = NULL;
    klass->parent_class.display_set = barchartDisplaySet;

    klass->parent_class.build_symbol_vectors = barchart_build_symbol_vectors;

    klass->parent_class.ruler_ranges_set = ruler_ranges_set;

    klass->parent_class.varpanel_refresh = barchartVarpanelRefresh;

    klass->parent_class.handles_action = barchartHandlesAction;

    klass->parent_class.xml_describe = NULL;

    klass->parent_class.varpanel_tooltips_set = barchartVarpanelTooltipsSet;

    klass->parent_class.plotted_vars_get = barchartPlottedColsGet;

    klass->parent_class.menus_make = barchartMenusMake;

    klass->parent_class.viewmode_control_box = barchartCPanelWidget;

    klass->parent_class.allow_reorientation = false;

    klass->parent_class.binning_ok = false;
    klass->parent_class.event_handlers_toggle = barchartEventHandlersToggle;
    klass->parent_class.splot_key_event_handler = barchartSPlotKeyEventHandler;
}

static void
barchartDisplayInit(barchartDisplayd *dpy)
{
   dpy->extendedDpy.titleLabel = "barchart";
}

GtkType
gtk_ggobi_barchart_display_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiBarChartDisplay",
	sizeof (barchartDisplayd),
	sizeof (GtkGGobiBarChartDisplayClass),
	(GtkClassInitFunc) barchartDisplayClassInit,
	(GtkObjectInitFunc) barchartDisplayInit,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_ggobi_extended_display_get_type (), &data_info);
    }

  return data_type;
}



static void 
barchartSPlotClassInit(GtkGGobiBarChartSPlotClass *klass)
{
      /* barcharts need more attention than redrawing the brush */
    klass->extendedSPlotClass.splot.redraw = FULL;
    klass->extendedSPlotClass.tree_label = barchart_tree_label;

    klass->extendedSPlotClass.identify_notify = barchart_identify_bars;
    klass->extendedSPlotClass.add_markup_cues =  barchart_add_bar_cues;
    klass->extendedSPlotClass.add_scaling_cues = barchart_scaling_visual_cues_draw;
    klass->extendedSPlotClass.add_plot_labels = barchart_splot_add_plot_labels;
    klass->extendedSPlotClass.redraw = barchart_redraw;

    klass->extendedSPlotClass.world_to_plane = barchart_recalc_dimensions;
    klass->extendedSPlotClass.plane_to_screen = barchartPlaneToScreen;

    klass->extendedSPlotClass.active_paint_points = barchart_active_paint_points;

    GTK_OBJECT_CLASS(klass)->destroy = barchartDestroy;
}

static void
barchartSPlotInit(barchartSPlotd *sp)
{
  sp->bar = (barchartd *) g_malloc (1 * sizeof (barchartd)); 
  sp->bar->index_to_rank = NULL;
  sp->bar->is_spine = FALSE;

  barchart_init_vectors(sp);
}

GtkType
gtk_ggobi_barchart_splot_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiBarChartSPlot",
	sizeof (barchartSPlotd),
	sizeof (GtkGGobiBarChartSPlotClass),
	(GtkClassInitFunc) barchartSPlotClassInit,
	(GtkObjectInitFunc) barchartSPlotInit,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_ggobi_extended_splot_get_type (), &data_info);
    }

  return data_type;
}

#endif /* BARCHART_IMPLEMENTED */


#if 0
allocCPanels(GTK_GGOBI_EXTENDED_DISPLAY(dpy));
void
allocCPanels(extendedDisplayd *dpy)
{
   GtkGGobiExtendedDisplayClass *klass;
   klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(dpy)->klass);

   if(klass->numControlPanels > 0)
      dpy->cpanel = (cpaneld **) g_malloc(sizeof(cpaneld *) * klass->numControlPanels);
}

#endif



/**************************************************************************/


static void 
timeSeriesSPlotClassInit(GtkGGobiBarChartSPlotClass *klass)
{
#if 0
      /* barcharts need more attention than redrawing the brush */
    klass->extendedSPlotClass.splot.redraw = FULL;

    klass->extendedSPlotClass.identify_notify = barchart_identify_bars;
    klass->extendedSPlotClass.add_markup_cues =  barchart_add_bar_cues;
    klass->extendedSPlotClass.add_scaling_cues = barchart_scaling_visual_cues_draw;
    klass->extendedSPlotClass.add_plot_labels = barchart_splot_add_plot_labels;
    klass->extendedSPlotClass.redraw = barchart_redraw;

    klass->extendedSPlotClass.active_paint_points = barchart_active_paint_points;
#endif
    klass->extendedSPlotClass.tree_label = tsTreeLabel;

    klass->extendedSPlotClass.within_draw_to_binned = tsWithinDrawBinned;
    klass->extendedSPlotClass.within_draw_to_unbinned = tsShowWhiskers;

    klass->extendedSPlotClass.draw_edge_p = tsDrawEdge_p;
    klass->extendedSPlotClass.draw_case_p = tsDrawCase_p;

    klass->extendedSPlotClass.add_plot_labels = tsAddPlotLabels;

    klass->extendedSPlotClass.sub_plane_to_screen = tsWithinPlaneToScreen;
    klass->extendedSPlotClass.alloc_whiskers = tsAllocWhiskers;

    klass->extendedSPlotClass.world_to_plane = tsWorldToPlane;
    GTK_OBJECT_CLASS(klass)->destroy = tsDestroy;
}


GtkType 
gtk_ggobi_time_series_splot_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      static const GtkTypeInfo data_info =
      {
	"GtkGGobiTimeSeriesSPlot",
	sizeof (timeSeriesSPlotd),
	sizeof (GtkGGobiTimeSeriesSPlotClass),
	(GtkClassInitFunc) timeSeriesSPlotClassInit,
	(GtkObjectInitFunc) NULL,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_ggobi_extended_splot_get_type (), &data_info);
    }

  return data_type;
}

