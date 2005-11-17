/*-- noop-toggle.h --*/

#ifndef _INC_NOOP_TOGGLE_BUTTON_H
#define _INC_NOOP_TOGGLE_BUTTON_H

#include <gtk/gtk.h>
#include <gtk/gtktogglebutton.h>

typedef struct _GGobiNoopToggleButton GGobiNoopToggleButton;
typedef struct _GGobiNoopToggleButtonClass GGobiNoopToggleButtonClass;

struct _GGobiNoopToggleButton {
  GtkToggleButton parent;
};

struct _GGobiNoopToggleButtonClass {
  GtkToggleButtonClass parent_class;
};

#define GGOBI_TYPE_NOOP_TOGGLE_BUTTON (ggobi_noop_toggle_button_get_type())
#define GGOBI_NOOP_TOGGLE_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),GGOBI_TYPE_NOOP_TOGGLE_BUTTON,GGobiNoopToggleButton))
#define GGOBI_NOOP_TOGGLE_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),GGOBI_TYPE_NOOP_TOGGLE_BUTTON,GGobiNoopToggleButtonClass))
#define GGOBI_IS_NOOP_TOGGLE_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GGOBI_TYPE_NOOP_TOGGLE_BUTTON))
#define GGOBI_IS_NOOP_TOGGLE_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GGOBI_TYPE_NOOP_TOGGLE_BUTTON))

GType ggobi_noop_toggle_button_get_type(void);
GtkWidget* ggobi_noop_toggle_button_new(void);
GtkWidget* ggobi_noop_toggle_button_new_with_label(const gchar *label);

#endif /* _INC_NOOP_TOGGLE_BUTTON_H */
