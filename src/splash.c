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

#include "config.h"

void
splash_destroy (GtkWidget * w, GdkEventButton * event, GtkWidget *win)
{
  gtk_widget_destroy (win);
}

static void
about_expose_cb(GtkWidget *w, GdkEventExpose *event)
{
  static gchar *message = 
  "<b>GGobi</b>\n"
  "<i>is created and maintained by </i>\n"
  "Deborah Swayne\n"
  "Duncan Temple Lang\n"
  "Dianne Cook\n"
  "Andreas Buja\n"
  "Michael Lawrence\n"
  "Hadley Wickham\n\n"
  "<i>www.ggobi.org</i>\n";
  
  PangoMatrix matrix = PANGO_MATRIX_INIT;
  PangoContext *ctx = gtk_widget_get_pango_context(w);
  PangoLayout *layout = pango_layout_new(ctx);
  PangoRectangle rect;
  
  pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
  pango_layout_set_markup(layout, message, -1);
  pango_layout_get_extents(layout, NULL, &rect);
  pango_matrix_scale(&matrix, (w->allocation.width - 5.) / (PANGO_PIXELS(rect.width)),
    (w->allocation.height - 5.) / (PANGO_PIXELS(rect.height)));
  pango_context_set_matrix(ctx, &matrix);
  pango_layout_context_changed(layout);
  gdk_draw_layout(w->window, w->style->fg_gc[GTK_STATE_NORMAL], 5, 5, layout);
  g_object_unref(G_OBJECT(layout));
}
  
void
splash_show (GGobiSession * gg)
{
  char *versionInfo;
  GtkWidget *drawing, *label;
  GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  GtkWidget *ebox = gtk_event_box_new ();
  GtkWidget *hbox = gtk_vbox_new (false, 0);
  
  gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
  
  gtk_container_add(GTK_CONTAINER(window), ebox);
  gtk_container_add (GTK_CONTAINER (ebox), hbox);
  
  drawing = gtk_drawing_area_new();
  g_signal_connect(G_OBJECT(drawing), "expose-event", G_CALLBACK(about_expose_cb), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), drawing, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (hbox), gtk_hseparator_new(), FALSE, FALSE, 0);
  
  versionInfo = g_strdup_printf("Version %s, %s", PACKAGE_VERSION, GGOBI_RELEASE_DATE);
  label = gtk_label_new (versionInfo);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  g_free (versionInfo);

  g_signal_connect (G_OBJECT (ebox),
                    "button_press_event",
                    G_CALLBACK (splash_destroy), window);
  gtk_widget_set_events (ebox, GDK_BUTTON_PRESS_MASK);

  gtk_widget_show_all (window);
}
