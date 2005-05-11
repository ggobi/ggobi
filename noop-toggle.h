/*-- noop-toggle.h --*/

#ifndef _INC_NOOP_TOGGLE_BUTTON_H
#define _INC_NOOP_TOGGLE_BUTTON_H

#include <gtk/gtk.h>
#include <gtk/gtktogglebutton.h>

typedef struct _GtkNoopToggleButton GtkNoopToggleButton;
typedef struct _GtkNoopToggleButtonClass GtkNoopToggleButtonClass;

struct _GtkNoopToggleButton {
  GtkToggleButton parent;
};

struct _GtkNoopToggleButtonClass {
  GtkToggleButtonClass parent_class;
};

#define GTK_TYPE_NOOP_TOGGLE_BUTTON (gtk_noop_toggle_button_get_type())
#define GTK_NOOP_TOGGLE_BUTTON(obj) (GTK_CHECK_CAST((obj),GTK_TYPE_NOOP_TOGGLE_BUTTON,GtkNoopToggleButton))
#define GTK_NOOP_TOGGLE_BUTTON_CLASS(klass) (GTK_TOGGLE_CLASS_CAST((klass),GTK_TYPE_NOOP_TOGGLE_BUTTON,GtkNoopToggleButtonClass))
#define GTK_IS_NOOP_TOGGLE_BUTTON(obj) (GTK_CHECK_TYPE((obj), GTK_TYPE_NOOP_TOGGLE_BUTTON))
#define GTK_IS_NOOP_TOGGLE_BUTTON_CLASS(klass) (GTK_CHECK_CLASS_TYPE((klass), GTK_TYPE_NOOP_TOGGLE_BUTTON))

GtkType gtk_noop_toggle_button_get_type(void);
GtkWidget* gtk_noop_toggle_button_new(void);
GtkWidget* gtk_noop_toggle_button_new_with_label(const gchar *label);

#endif /* _INC_NOOP_TOGGLE_BUTTON_H */
