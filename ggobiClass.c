#include "ggobi.h"
#include "GGobiAPI.h"

#include "externs.h"

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
display_init(displayd *dpy)
{

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
	(GtkClassInitFunc) NULL, /*XX register any GGobi events. */
	(GtkObjectInitFunc) gtk_splot_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_drawing_area_get_type (), &data_info);
    }

  return data_type;
}

