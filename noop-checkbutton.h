/*-- gtk-noop-checkbutton.h --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#ifndef _INC_GTK_NOOP_CHECK_BUTTON_H
#define _INC_GTK_NOOP_CHECK_BUTTON_H

#include <gtk/gtk.h>
#include <gtk/gtkcheckbutton.h>

typedef struct _GtkNoopCheckButton GtkNoopCheckButton;
typedef struct _GtkNoopCheckButtonClass GtkNoopCheckButtonClass;

struct _GtkNoopCheckButton {
  GtkCheckButton parent;
};

struct _GtkNoopCheckButtonClass {
  GtkCheckButtonClass parent_class;
};

#define GTK_TYPE_NOOP_CHECK_BUTTON (gtk_noop_check_button_get_type())
#define GTK_NOOP_CHECK_BUTTON(obj) (GTK_CHECK_CAST((obj),GTK_TYPE_NOOP_CHECK_BUTTON,GtkNoopCheckButton))
#define GTK_NOOP_CHECK_BUTTON_CLASS(klass) (GTK_CHECK_CLASS_CAST((klass),GTK_TYPE_NOOP_CHECK_BUTTON,GtkNoopCheckButtonClass))
#define GTK_IS_NOOP_CHECK_BUTTON(obj) (GTK_CHECK_TYPE((obj), GTK_TYPE_NOOP_CHECK_BUTTON))
#define GTK_IS_NOOP_CHECK_BUTTON_CLASS(klass) (GTK_CHECK_CLASS_TYPE((klass), GTK_TYPE_NOOP_CHECK_BUTTON))

GtkType gtk_noop_check_button_get_type(void);

GtkWidget* gtk_noop_check_button_new(void);
GtkWidget* gtk_noop_check_button_new_with_label(const gchar *);

#endif /* _INC_GTK_NOOP_CHECK_BUTTON_H */

/* $Id$ */
