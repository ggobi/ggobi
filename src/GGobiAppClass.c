#include "GGobiApp.h"
#include "ggobi.h"

void ggobi_app_class_init (GGobiAppClass * klass);
void ggobi_app_init (GGobiApp * app);

/**
  This registers and returns a unique GType representing
  the ggobi app class.
 */
GType
ggobi_app_get_type (void)
{
  static GType app_type = 0;

  if (!app_type) {
    static const GTypeInfo app_info = {
      sizeof (GGobiAppClass),
      NULL, NULL,
      (GClassInitFunc) ggobi_app_class_init,
      NULL, NULL,
      sizeof (GGobiApp), 0,
      (GInstanceInitFunc) ggobi_app_init,
      NULL
    };

    app_type =
      g_type_register_static (G_TYPE_OBJECT, "GGobiApp", &app_info, 0);
  }

  return app_type;
}


void
ggobi_app_init (GGobiApp * app)
{

}

void
ggobi_app_class_init (GGobiAppClass * klass)
{                               /* why is this stored as the "DATAD_ADDED_SIGNAL" ? */
  if (g_signal_lookup ("new_ggobi", GGOBI_TYPE_APP) == 0) {
    //GGobiSignals[DATAD_ADDED_SIGNAL] =
    g_signal_new ("new_ggobi",
                  GGOBI_TYPE_APP,
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, GGOBI_TYPE_SESSION);
  }
}
