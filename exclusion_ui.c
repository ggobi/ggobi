/*-- exclusion_ui.c  --*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static GtkWidget *exclusion_window = NULL;
static GtkWidget *exclusion_table;

static gint
exclusion_symbol_show (GtkWidget *w, GdkEventExpose *event, gpointer cbd)
{
  gint k = GPOINTER_TO_INT (cbd);
  icoords pos;
  glyphv g;

  /*-- fill in the background color --*/
  gdk_gc_set_foreground (xg.plot_GC, &xg.bg_color);
  gdk_draw_rectangle (w->window, xg.plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);

  /*-- draw the appropriate symbol in the appropriate color --*/
  gdk_gc_set_foreground (xg.plot_GC, &xg.default_color_table[k]);
  g.type = xg.clusv[k].glyphtype;
  g.size = xg.clusv[k].glyphsize;

  pos.x = pos.y = NGLYPHSIZES + 1 + 10;  /*-- use margin = 10 --*/
  draw_glyph (w->window, &g, &pos, 0);

  return FALSE;
}

static gint
exclusion_symbol_cb (GtkWidget *w, GdkEventExpose *event, gpointer cbd)
{
  return false;
}

void
exclusion_cluster_add (gint k) {
  gchar *str;
  gint dawidth = 2*NGLYPHSIZES+1 + 20;  /*-- use margin = 10 --*/

g_printerr ("add: k=%d rows=%d\n", k, GTK_TABLE (exclusion_table)->nrows);
  xg.clusv[k].da = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (xg.clusv[k].da),
    dawidth, dawidth);

  gtk_widget_set_events (xg.clusv[k].da, GDK_EXPOSURE_MASK
        | GDK_ENTER_NOTIFY_MASK
        | GDK_LEAVE_NOTIFY_MASK
        | GDK_BUTTON_PRESS_MASK);

  gtk_signal_connect (GTK_OBJECT (xg.clusv[k].da), "expose_event",
    GTK_SIGNAL_FUNC (exclusion_symbol_show), GINT_TO_POINTER (k));
  gtk_signal_connect (GTK_OBJECT (xg.clusv[k].da), "button_press_event",
    GTK_SIGNAL_FUNC (exclusion_symbol_cb), GINT_TO_POINTER (k));

  gtk_table_attach (GTK_TABLE (exclusion_table),
    xg.clusv[k].da,
    0, 1, k+1, k+2, GTK_FILL, GTK_FILL, 10, 10);  /*-- don't fill --*/

  str = g_strdup_printf ("%ld", xg.clusv[k].n);
  gtk_table_attach (GTK_TABLE (exclusion_table),
    gtk_label_new (str),
    1, 2, k+1, k+2, GTK_FILL, GTK_FILL, 10, 10);
  g_free (str);

  xg.clusv[k].hide_tgl = gtk_check_button_new ();
  GTK_TOGGLE_BUTTON (xg.clusv[k].hide_tgl)->active = xg.clusv[k].hidden;
  gtk_table_attach (GTK_TABLE (exclusion_table),
    xg.clusv[k].hide_tgl,
    2, 3, k+1, k+2, GTK_FILL, GTK_FILL, 10, 10);

  xg.clusv[k].exclude_tgl = gtk_check_button_new ();
  GTK_TOGGLE_BUTTON (xg.clusv[k].exclude_tgl)->active = !xg.clusv[k].included;
  gtk_table_attach (GTK_TABLE (exclusion_table),
    xg.clusv[k].exclude_tgl,
    3, 4, k+1, k+2, GTK_FILL, GTK_FILL, 10, 10);
}

void
exclusion_window_open (void) {

  GtkWidget *vbox, *ebox, *btn;
  gint k;

  clusters_set ();
g_printerr ("nclusters = %d\n", xg.nclust);

/*
 * Here, destroy the table and free all clusters.
*/

  if (exclusion_window == NULL) {

    exclusion_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (exclusion_window),
      "ggobi hide/exclude window");

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (exclusion_window), vbox);

    ebox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (vbox), ebox, false, false, 0);

    exclusion_table = gtk_table_new (xg.nclust+1, 4, false);
    gtk_container_add (GTK_CONTAINER (ebox), exclusion_table);

    /*-- add the row of titles --*/

    gtk_table_attach (GTK_TABLE (exclusion_table), gtk_label_new ("Symbol"),
      0, 1, 0, 1,  /*-- left, right, top, bottom --*/
      GTK_FILL, GTK_FILL, 10, 10);
    gtk_table_attach (GTK_TABLE (exclusion_table), gtk_label_new ("N elements"),
      1, 2, 0, 1, GTK_FILL, GTK_FILL, 10, 10);
    gtk_table_attach (GTK_TABLE (exclusion_table), gtk_label_new ("Hidden"),
      2, 3, 0, 1, GTK_FILL, GTK_FILL, 10, 10);
    gtk_table_attach (GTK_TABLE (exclusion_table), gtk_label_new ("Excluded"),
      3, 4, 0, 1, GTK_FILL, GTK_FILL, 10, 10);

    /*-- add the cluster rows, one by one --*/
/*    for (k=0; k<xg.nclust; k++)*/
    for (k=0; k<2; k++) {
      exclusion_cluster_add (k);
    }

    /*-- Close button --*/
    btn = gtk_button_new_with_label ("Close");
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 0);
  }

  gtk_widget_show_all (exclusion_window);
}
