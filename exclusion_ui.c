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
  ggobid *gg = GGobiFromWidget(w, true);
  icoords pos;
  glyphv g;

  /*-- fill in the background color --*/
  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (w->window, gg->plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);

  /*-- draw the appropriate symbol in the appropriate color --*/
  gdk_gc_set_foreground (gg->plot_GC,
                        &gg->default_color_table[gg->clusv[k].color]);
  g.type = gg->clusv[k].glyphtype;
  g.size = gg->clusv[k].glyphsize;

  pos.x = w->allocation.width / 2;
  pos.y = w->allocation.height / 2;
  draw_glyph (w->window, &g, &pos, 0, gg);

  return FALSE;
}

static gint
hide_cluster_cb (GtkToggleButton *btn, gpointer cbd)
{
  gint k = GPOINTER_TO_INT (cbd);
  gint i;
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(btn), true);

  gg->clusv[k].hidden = btn->active;


  for (i=0; i<gg->nrows; i++) {
    if (gg->sampled[i]) {
      if (gg->clusterid.data[i] == k) {
        gg->hidden[i] = gg->hidden_now[i] = btn->active;
      }
    }
  }
  displays_plot (NULL, gg);

  return false;
}

static gint
exclude_cluster_cb (GtkToggleButton *btn, gpointer cbd)
{
  gint k = GPOINTER_TO_INT (cbd);
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(btn), true);
  gint i;

  gg->clusv[k].included = !btn->active;


  for (i=0; i<gg->nrows; i++) {
    if (gg->sampled[i])  {
      if (gg->clusterid.data[i] == k) {
        gg->included[i] = gg->included[i] = !btn->active;
      }
    }
  }

  rows_in_plot_set (gg);

  /*-- should be exactly what happens in subset_apply --*/
  vardata_lim_update (gg);
  tform_to_world (gg);
  displays_tailpipe (REDISPLAY_ALL, gg);
/*  assign_points_to_bins ();*/
  return false;
}

static gint
exclusion_symbol_cb (GtkWidget *w, GdkEventExpose *event, gpointer cbd)
{
  return false;
}

void
exclusion_cluster_add (gint k, ggobid *gg) {
  gchar *str;
  gint dawidth = 2*NGLYPHSIZES+1 + 10;  /*-- use margin = 5 --*/

  gg->clusv[k].da = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (gg->clusv[k].da),
    dawidth, dawidth);

  gtk_widget_set_events (gg->clusv[k].da,
      GDK_EXPOSURE_MASK
    | GDK_ENTER_NOTIFY_MASK
    | GDK_LEAVE_NOTIFY_MASK
    | GDK_BUTTON_PRESS_MASK);

  gtk_signal_connect (GTK_OBJECT (gg->clusv[k].da), "expose_event",
    GTK_SIGNAL_FUNC (exclusion_symbol_show), GINT_TO_POINTER (k));
  gtk_signal_connect (GTK_OBJECT (gg->clusv[k].da), "button_press_event",
    GTK_SIGNAL_FUNC (exclusion_symbol_cb), GINT_TO_POINTER (k));
  GGobi_widget_set (gg->clusv[k].da, gg, true);
  gtk_table_attach (GTK_TABLE (exclusion_table),
    gg->clusv[k].da,
    0, 1, k+1, k+2, 0, 0, 5, 2);  /*-- don't fill --*/

  gg->clusv[k].hide_tgl = gtk_check_button_new ();
  GTK_TOGGLE_BUTTON (gg->clusv[k].hide_tgl)->active = gg->clusv[k].hidden;
  gtk_signal_connect (GTK_OBJECT (gg->clusv[k].hide_tgl), "toggled",
    GTK_SIGNAL_FUNC (hide_cluster_cb), GINT_TO_POINTER (k));
  GGobi_widget_set (gg->clusv[k].hide_tgl, gg, true);
  gtk_table_attach (GTK_TABLE (exclusion_table),
    gg->clusv[k].hide_tgl,
    1, 2, k+1, k+2, GTK_FILL, GTK_FILL, 5, 2);

  gg->clusv[k].exclude_tgl = gtk_check_button_new ();
  GTK_TOGGLE_BUTTON (gg->clusv[k].exclude_tgl)->active = !gg->clusv[k].included;
  gtk_signal_connect (GTK_OBJECT (gg->clusv[k].exclude_tgl), "toggled",
    GTK_SIGNAL_FUNC (exclude_cluster_cb), GINT_TO_POINTER (k));
  GGobi_widget_set (gg->clusv[k].exclude_tgl, gg, true);
  gtk_table_attach (GTK_TABLE (exclusion_table),
    gg->clusv[k].exclude_tgl,
    2, 3, k+1, k+2, GTK_FILL, GTK_FILL, 5, 2);

  str = g_strdup_printf ("%ld", gg->clusv[k].n);
  gg->clusv[k].lbl = gtk_label_new (str);
  gtk_table_attach (GTK_TABLE (exclusion_table),
    gg->clusv[k].lbl,
    3, 4, k+1, k+2, GTK_FILL, GTK_FILL, 5, 2);
  g_free (str);
}

static void closeit (ggobid *gg) {
  gint n;

  for (n=0; n<gg->nclust; n++)
    cluster_free (n, gg);

  gtk_widget_destroy (exclusion_window);

  exclusion_window = NULL;
}

/*-- called when closed from the close button --*/
static void close_cb (GtkWidget *w, ggobid *gg) {
  closeit (gg);
}

/*-- called when closed from the window manager --*/
static void delete_cb (GtkWidget *w, GdkEvent *event, ggobid *gg) {
  closeit (gg);
}

void
exclusion_window_open (ggobid *gg) {

  GtkWidget *vbox, *ebox, *btn;
  gint k;

  if (exclusion_window == NULL) {

    clusters_set (gg);

g_printerr ("gg=%d\n", (gint)gg);
    exclusion_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_signal_connect (GTK_OBJECT (exclusion_window), "delete_event",
                        GTK_SIGNAL_FUNC (delete_cb), (gpointer) gg);
    gtk_window_set_title (GTK_WINDOW (exclusion_window),
      "ggobi hide/exclude window");

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (exclusion_window), vbox);

    ebox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (vbox), ebox, false, false, 0);

    exclusion_table = gtk_table_new (gg->nclust+1, 4, false);
    gtk_container_add (GTK_CONTAINER (ebox), exclusion_table);

    /*-- add the row of titles --*/

    gtk_table_attach (GTK_TABLE (exclusion_table), gtk_label_new ("Symbol"),
      0, 1, 0, 1,  /*-- left, right, top, bottom --*/
      GTK_FILL, GTK_FILL, 5, 2);
    gtk_table_attach (GTK_TABLE (exclusion_table), gtk_label_new ("Hidden"),
      1, 2, 0, 1, GTK_FILL, GTK_FILL, 5, 2);
    gtk_table_attach (GTK_TABLE (exclusion_table), gtk_label_new ("Excluded"),
      2, 3, 0, 1, GTK_FILL, GTK_FILL, 5, 2);
    gtk_table_attach (GTK_TABLE (exclusion_table), gtk_label_new ("N elements"),
      3, 4, 0, 1, GTK_FILL, GTK_FILL, 5, 2);

    /*-- add the cluster rows, one by one --*/
    for (k=0; k<gg->nclust; k++)
      exclusion_cluster_add (k, gg);

    /*-- Close button --*/
    btn = gtk_button_new_with_label ("Close");
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                   GTK_SIGNAL_FUNC (close_cb), (gpointer) gg);
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 0);

/*
    *-- destroy all the widgets that were in the table --*
    gint table_rows = GTK_TABLE (exclusion_table)->nrows;

    for (k=0; k<table_rows-1; k++)
      cluster_free (k, gg);

    gtk_table_resize (GTK_TABLE (exclusion_table), 1, 4);
    gtk_table_resize (GTK_TABLE (exclusion_table), gg->nclust+1, 4);

    *-- add the cluster rows, one by one --*
    for (k=0; k<gg->nclust; k++)
      exclusion_cluster_add (k, gg);
*/
    gtk_widget_show_all (exclusion_window);
  }

  gdk_window_raise (exclusion_window->window);
}
