/* ppcorr_ui.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Eclipse Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define WIDTH   600
#define HEIGHT  300

static GtkWidget *window = NULL;
static GtkWidget *control_frame;
static GtkWidget *mbar;
//static GtkAccelGroup *cpp_accel_group;

static void
hide_cb (GtkWidget *w, GtkWidget *window)
{
  gtk_widget_hide (window);
}

static gboolean
hide_wmgr_cb (GtkWidget *w, GdkEvent *event, GtkWidget *window)
{
  gtk_widget_hide (window);
  return TRUE;
}

static void
optimize_cb (GtkToggleButton * w)
{
  g_printerr ("optimize?  %d\n", gtk_toggle_button_get_active (w));
}

void
ctourpp_window_open (ggobid * gg)
{
  GtkWidget *hbox, *vbox, *vbc, *frame, *tgl, *entry;
  GtkWidget *da, *label, *hb;
  GtkWidget *file_menu;
  GtkAccelGroup *accel_group = NULL;

  if (window == NULL) {
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (G_OBJECT (window), "delete_event",
                      G_CALLBACK (hide_wmgr_cb), window);
    gtk_window_set_title (GTK_WINDOW (window), "Projection Pursuit");
    //gtk_window_set_policy (GTK_WINDOW (window), true, true, false);
    gtk_container_set_border_width (GTK_CONTAINER (window), 5);

/*
 * Add the main menu bar
*/
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 1);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    accel_group = ggobi_window_add_accel_group (window);
    mbar = gtk_menu_bar_new ();
    file_menu = ggobi_menu_add_submenu (mbar, "_File");
    ggobi_menu_append_item (file_menu, "_Close", "<control>C",
                            accel_group, G_CALLBACK (hide_cb), window);

    gtk_box_pack_start (GTK_BOX (vbox), mbar, false, true, 0);

/*
 * Divide the window:  controls on the left, plot on the right
*/
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 1);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, true, true, 1);

/*
 * Controls
*/
    control_frame = gtk_frame_new (NULL);
    //gtk_frame_set_shadow_type (GTK_FRAME (control_frame), GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (control_frame), 5);
    gtk_box_pack_start (GTK_BOX (hbox), control_frame, false, false, 1);

    vbc = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width (GTK_CONTAINER (vbc), 5);
    gtk_container_add (GTK_CONTAINER (control_frame), vbc);

/*
 * Optimize toggle
*/
    tgl = gtk_check_button_new_with_mnemonic ("_Optimize");
    gtk_widget_set_tooltip_text ((tgl), gg->tips ? ("Guide the tour using projection pursuit optimization or tour passively") : NULL);
    g_signal_connect (G_OBJECT (tgl), "toggled",
                      G_CALLBACK (optimize_cb), (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vbc), tgl, false, false, 1);

/*
 * Index value with label
*/
    hb = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
    gtk_box_pack_start (GTK_BOX (vbc), hb, false, false, 2);

    label = gtk_label_new ("PP index:");
    gtk_label_set_xalign (GTK_LABEL (label), 0);
    gtk_label_set_yalign (GTK_LABEL (label), 0.5);
    gtk_box_pack_start (GTK_BOX (hb), label, false, false, 0);

    entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (entry), 32);
    gtk_editable_set_editable (GTK_EDITABLE (entry), false);
    gtk_widget_set_tooltip_text ((entry), gg->tips ? ("The value of the projection pursuit index for the current projection") : NULL);
    gtk_box_pack_start (GTK_BOX (hb), entry, false, false, 2);

/*
 * Drawing area in a frame
*/
    frame = gtk_frame_new (NULL);
    //gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_box_pack_start (GTK_BOX (hbox), frame, true, true, 1);

    da = gtk_drawing_area_new ();
    gtk_widget_set_size_request (GTK_WIDGET (da), WIDTH, HEIGHT);
    gtk_container_add (GTK_CONTAINER (frame), da);
  }

  gtk_widget_show_all (window);
}
