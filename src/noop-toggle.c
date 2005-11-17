/*-- noop-toggle.c --*/

#include "noop-toggle.h"
#include <gtk/gtklabel.h>

static GtkObjectClass* parent_class = NULL;

static void
ggobi_noop_toggle_button_destroy(GtkObject* obj)
{
  if (parent_class->destroy)
    parent_class->destroy(obj);
}

static gint
button_press_event(GtkWidget* w, GdkEventButton* event)
{
  /* Handle your button presses here and return TRUE.  If you want the
     event to be passed on to the parent class to handle, just return
     FALSE. 

     You have total control, and should be able to get any
     button-press semantics you can dream up...
  */

  /*-- I want it to do nothing whatever -- dfs --*/

  return TRUE;
}

static void
ggobi_noop_toggle_button_class_init(GGobiNoopToggleButtonClass* klass)
{
  GtkObjectClass* object_class = (GtkObjectClass*)klass;
  GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

  parent_class = g_type_class_peek(GTK_TYPE_TOGGLE_BUTTON);

  object_class->destroy = ggobi_noop_toggle_button_destroy;
  widget_class->button_press_event = button_press_event;
}

static void
ggobi_noop_toggle_button_init(GGobiNoopToggleButton* obj)
{
  /* Initialize any state here. */
}

GType
ggobi_noop_toggle_button_get_type(void)
{
  static GType ggobi_noop_toggle_button_type = 0;
  if (!ggobi_noop_toggle_button_type) {
    static const GTypeInfo ggobi_noop_toggle_button_info = {
      sizeof(GGobiNoopToggleButtonClass),
      NULL, NULL,
	  (GClassInitFunc)ggobi_noop_toggle_button_class_init,
      NULL, NULL,
      sizeof(GGobiNoopToggleButton), 0,
      (GInstanceInitFunc)ggobi_noop_toggle_button_init,
      NULL
    };
    ggobi_noop_toggle_button_type = g_type_register_static(GTK_TYPE_TOGGLE_BUTTON, "GGobiNoopToggleButton",
      &ggobi_noop_toggle_button_info, 0);
  }
  return ggobi_noop_toggle_button_type;
}

GtkWidget*
ggobi_noop_toggle_button_new(void)
{
  /* Do initialization up in _init(), not here.  If you need to
     pass in args to _new, write a _construct() function that
     takes those args and applies them to the object you build here
     before returning it. */
  return GTK_WIDGET(g_object_new(ggobi_noop_toggle_button_get_type(), NULL));
}

GtkWidget*
ggobi_noop_toggle_button_new_with_label (const gchar *label)
{
  GtkWidget *noop_toggle_button;
  GtkWidget *label_widget;

  noop_toggle_button = ggobi_noop_toggle_button_new ();
  label_widget = gtk_label_new (label);
  gtk_misc_set_alignment (GTK_MISC (label_widget), 0, 0.5);

  gtk_container_add (GTK_CONTAINER (noop_toggle_button), label_widget);
  gtk_widget_show (label_widget);

  return noop_toggle_button;
}

