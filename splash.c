/* main_ui.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#include <gtk/gtk.h>
#include <string.h>

#include "vars.h"
#include "externs.h"

#include "splash.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

void
splash_destroy (GtkWidget *w, GdkEventButton *event, GdkPixmap *pix)
{
  GtkWidget *win = (GtkWidget *) gtk_object_get_data (GTK_OBJECT(w), "window");

  gdk_pixmap_unref (pix);
  gtk_widget_destroy (win);
}

void
splash_show (ggobid *gg, guint action, GtkWidget *w)
{
  char *versionInfo;
  GdkPixmap *splash_pix;
  GtkWidget *splashw, *label;
  gchar homePrefix[] = "    GGobi home: ";
  GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  GtkWidget *ebox = gtk_event_box_new ();
  GtkWidget *hbox = gtk_vbox_new (false, 0);

  splash_pix = gdk_pixmap_colormap_create_from_xpm_d (NULL,
    gtk_widget_get_colormap (w),
    NULL, NULL, (gchar **) splash);
  splashw = gtk_pixmap_new (splash_pix, NULL);

  gtk_container_add (GTK_CONTAINER (window), ebox);
  gtk_container_add (GTK_CONTAINER (ebox), hbox);
  gtk_box_pack_start (GTK_BOX (hbox), splashw, FALSE, FALSE, 0);

  versionInfo = (char *) g_malloc(sizeof(gchar)*(strlen("Version ") + 
                                                 strlen(GGOBI_VERSION_STRING) + 
                                                 2 +
                                                 strlen(GGOBI_RELEASE_DATE) + 1)
				                 + strlen(homePrefix)
				                 + strlen(sessionOptions->ggobiHome));
  sprintf(versionInfo,"%s %s, %s%s%s", 
	               "Version", GGOBI_VERSION_STRING, GGOBI_RELEASE_DATE,
	               homePrefix, sessionOptions->ggobiHome);
  label = gtk_label_new(versionInfo);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  g_free(versionInfo);

  gtk_object_set_data (GTK_OBJECT (ebox), "window", (gpointer) window);

  gtk_signal_connect (GTK_OBJECT (ebox),
                      "button_press_event",
                      (GtkSignalFunc) splash_destroy,
                      (gpointer) splash_pix);
  gtk_widget_set_events (ebox, GDK_BUTTON_PRESS_MASK);

  gtk_widget_show_all (window);
}
