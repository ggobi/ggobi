/*-- gtk-noop-checkbutton.h --*/

#ifndef _INC_GTK_NOOP_CHECK_BUTTON_H
#define _INC_GTK_NOOP_CHECK_BUTTON_H

#include <gtk/gtk.h>
#include <gtk/gtkcheckbutton.h>

typedef struct _GGobiNoopCheckButton GGobiNoopCheckButton;
typedef struct _GGobiNoopCheckButtonClass GGobiNoopCheckButtonClass;

struct _GGobiNoopCheckButton {
  GtkCheckButton parent;
};

struct _GGobiNoopCheckButtonClass {
  GtkCheckButtonClass parent_class;
};

#define GGOBI_TYPE_NOOP_CHECK_BUTTON (ggobi_noop_check_button_get_type())
#define GGOBI_NOOP_CHECK_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),GGOBI_TYPE_NOOP_CHECK_BUTTON,GGobiNoopCheckButton))
#define GGOBI_NOOP_CHECK_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),GGOBI_TYPE_NOOP_CHECK_BUTTON,GGobiNoopCheckButtonClass))
#define GGOBI_IS_NOOP_CHECK_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GGOBI_TYPE_NOOP_CHECK_BUTTON))
#define GGOBI_IS_NOOP_CHECK_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GGOBI_TYPE_NOOP_CHECK_BUTTON))

GType ggobi_noop_check_button_get_type(void);

GtkWidget* ggobi_noop_check_button_new(void);
GtkWidget* ggobi_noop_check_button_new_with_label(const gchar *);

#endif /* _INC_GTK_NOOP_CHECK_BUTTON_H */

/* $Id$ */
