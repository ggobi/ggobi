/*-- noop-toggle.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include "noop-toggle.h"
#include <gtk/gtklabel.h>

static GtkObjectClass* parent_class = NULL;

static void
gtk_noop_toggle_button_destroy(GtkObject* obj)
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
gtk_noop_toggle_button_class_init(GtkNoopToggleButtonClass* klass)
{
  GtkObjectClass* object_class = (GtkObjectClass*)klass;
  GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

  parent_class = gtk_type_class(GTK_TYPE_TOGGLE_BUTTON);

  object_class->destroy = gtk_noop_toggle_button_destroy;
  widget_class->button_press_event = button_press_event;
}

static void
gtk_noop_toggle_button_init(GtkNoopToggleButton* obj)
{
  /* Initialize any state here. */
}

GtkType
gtk_noop_toggle_button_get_type(void)
{
  static GtkType gtk_noop_toggle_button_type = 0;
  if (!gtk_noop_toggle_button_type) {
    static const GtkTypeInfo gtk_noop_toggle_button_info = {
      "GtkNoopToggleButton",
      sizeof(GtkNoopToggleButton),
      sizeof(GtkNoopToggleButtonClass),
      (GtkClassInitFunc) gtk_noop_toggle_button_class_init,
      (GtkObjectInitFunc)gtk_noop_toggle_button_init,
      NULL, NULL, (GtkClassInitFunc)NULL
    };
    gtk_noop_toggle_button_type = gtk_type_unique(GTK_TYPE_TOGGLE_BUTTON,
      &gtk_noop_toggle_button_info);
  }
  return gtk_noop_toggle_button_type;
}

GtkWidget*
gtk_noop_toggle_button_new(void)
{
  /* Do initialization up in _init(), not here.  If you need to
     pass in args to _new, write a _construct() function that
     takes those args and applies them to the object you build here
     before returning it. */
  return GTK_WIDGET(gtk_type_new(gtk_noop_toggle_button_get_type()));
}

GtkWidget*
gtk_noop_toggle_button_new_with_label (const gchar *label)
{
  GtkWidget *noop_toggle_button;
  GtkWidget *label_widget;

  noop_toggle_button = gtk_noop_toggle_button_new ();
  label_widget = gtk_label_new (label);
  gtk_misc_set_alignment (GTK_MISC (label_widget), 0, 0.5);

  gtk_container_add (GTK_CONTAINER (noop_toggle_button), label_widget);
  gtk_widget_show (label_widget);

  return noop_toggle_button;
}

