/* splash.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <gtk/gtk.h>
#include <string.h>

#include "vars.h"
#include "externs.h"

#include "splash.h"

#ifdef HAVE_GGOBI_CONFIG_H
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
