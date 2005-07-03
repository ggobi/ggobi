#include "GGobiApp.h"
#include "ggobi.h"

void gtk_ggobi_app_class_init(GtkGGobiAppClass * klass);
void ggobi_app_init(GtkGGobiApp *app);

/**
  This registers and returns a unique Gtk type representing
  the ggobi class.
 */
GtkType gtk_ggobi_app_get_type(void)
{
  static GtkType data_type = 0;

  if (!data_type) {
    static const GtkTypeInfo data_info = {
      "GtkGGobiApp",
      sizeof(GtkGGobiApp),
      sizeof(GtkGGobiAppClass),
      (GtkClassInitFunc) gtk_ggobi_app_class_init,
      (GtkObjectInitFunc) ggobi_app_init,
      /* reserved_1 */ NULL,
      /* reserved_2 */ NULL,
      (GtkClassInitFunc) NULL,
    };

    data_type = gtk_type_unique(gtk_object_get_type(), &data_info);
  }

  return data_type;
}


void
ggobi_app_init(GtkGGobiApp *app)
{

}

void gtk_ggobi_app_class_init(GtkGGobiAppClass * klass)
{
#ifndef GTK_2_0
  if (gtk_signal_lookup("new_ggobi", GTK_TYPE_GGOBI_APP) == 0) {
    GGobiSignals[DATAD_ADDED_SIGNAL] =
      gtk_object_class_user_signal_new(gtk_type_class(GTK_TYPE_GGOBI_APP),
        "new_ggobi",
        GTK_RUN_LAST | GTK_RUN_ACTION,
        gtk_marshal_NONE__POINTER,
        GTK_TYPE_NONE, 1,
        GTK_TYPE_GGOBI);
  }

#endif

}
