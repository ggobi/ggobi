/*-- exclusion_ui.c  --*/

/*
 * I'm constructing one table for each datad.  The thing that's
 * still missing is some kind of labelling or separation to 
 * distinguish datad's from one another.  This also doesn't use
 * scrolling, and that could become a problem.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static gint
exclusion_symbol_show (GtkWidget *w, GdkEventExpose *event, gpointer cbd)
{
  gint k = GPOINTER_TO_INT (cbd);
  ggobid *gg = GGobiFromWidget (w, true);
  icoords pos;
  glyphv g;
  datad *d = gg->current_display->d;

  /*-- fill in the background color --*/
  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (w->window, gg->plot_GC,
                      true, 0, 0,
                      w->allocation.width, w->allocation.height);

  /*-- draw the appropriate symbol in the appropriate color --*/
  gdk_gc_set_foreground (gg->plot_GC,
                        &gg->default_color_table[d->clusv[k].color]);
  g.type = d->clusv[k].glyphtype;
  g.size = d->clusv[k].glyphsize;

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
  ggobid *gg = GGobiFromWidget (GTK_WIDGET (btn), true);
  datad *d = gg->current_display->d;

  d->clusv[k].hidden = btn->active;


  for (i=0; i<d->nrows; i++) {
    if (d->sampled[i]) {
      if (d->clusterids.vals[i] == k) {
        d->hidden[i] = d->hidden_now[i] = btn->active;
      }
    }
  }
  displays_plot (NULL, FULL, gg);

  return false;
}

static gint
exclude_cluster_cb (GtkToggleButton *btn, gpointer cbd)
{
  gint k = GPOINTER_TO_INT (cbd);
  ggobid *gg = GGobiFromWidget (GTK_WIDGET (btn), true);
  gint i;
  datad *d = gg->current_display->d;

  d->clusv[k].included = !btn->active;


  for (i=0; i<d->nrows; i++) {
    if (d->sampled[i])  {
      if (d->clusterids.vals[i] == k) {
        d->included[i] = d->included[i] = !btn->active;
      }
    }
  }

  rows_in_plot_set (d, gg);

  /*-- should be exactly what happens in subset_apply --*/
  vardata_lim_update (d, gg);
  tform_to_world (d, gg);
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
exclusion_cluster_add (gint k, datad *d, ggobid *gg) {
  gchar *str;
  gint dawidth = 2*NGLYPHSIZES+1 + 10;  /*-- use margin = 5 --*/

  d->clusv[k].da = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (d->clusv[k].da),
    dawidth, dawidth);

  gtk_widget_set_events (d->clusv[k].da,
      GDK_EXPOSURE_MASK
    | GDK_ENTER_NOTIFY_MASK
    | GDK_LEAVE_NOTIFY_MASK
    | GDK_BUTTON_PRESS_MASK);

  gtk_signal_connect (GTK_OBJECT (d->clusv[k].da), "expose_event",
    GTK_SIGNAL_FUNC (exclusion_symbol_show), GINT_TO_POINTER (k));
  gtk_signal_connect (GTK_OBJECT (d->clusv[k].da), "button_press_event",
    GTK_SIGNAL_FUNC (exclusion_symbol_cb), GINT_TO_POINTER (k));
  GGobi_widget_set (d->clusv[k].da, gg, true);
  gtk_table_attach (GTK_TABLE (d->exclusion_table),
    d->clusv[k].da,
    0, 1, k+1, k+2, 0, 0, 5, 2);  /*-- don't fill --*/

  d->clusv[k].hide_tgl = gtk_check_button_new ();
  GTK_TOGGLE_BUTTON (d->clusv[k].hide_tgl)->active = d->clusv[k].hidden;
  gtk_signal_connect (GTK_OBJECT (d->clusv[k].hide_tgl), "toggled",
    GTK_SIGNAL_FUNC (hide_cluster_cb), GINT_TO_POINTER (k));
  GGobi_widget_set (d->clusv[k].hide_tgl, gg, true);
  gtk_table_attach (GTK_TABLE (d->exclusion_table),
    d->clusv[k].hide_tgl,
    1, 2, k+1, k+2, GTK_FILL, GTK_FILL, 5, 2);

  d->clusv[k].exclude_tgl = gtk_check_button_new ();
  GTK_TOGGLE_BUTTON (d->clusv[k].exclude_tgl)->active = !d->clusv[k].included;
  gtk_signal_connect (GTK_OBJECT (d->clusv[k].exclude_tgl), "toggled",
    GTK_SIGNAL_FUNC (exclude_cluster_cb), GINT_TO_POINTER (k));
  GGobi_widget_set (d->clusv[k].exclude_tgl, gg, true);
  gtk_table_attach (GTK_TABLE (d->exclusion_table),
    d->clusv[k].exclude_tgl,
    2, 3, k+1, k+2, GTK_FILL, GTK_FILL, 5, 2);

  str = g_strdup_printf ("%ld", d->clusv[k].n);
  d->clusv[k].lbl = gtk_label_new (str);
  gtk_table_attach (GTK_TABLE (d->exclusion_table),
    d->clusv[k].lbl,
    3, 4, k+1, k+2, GTK_FILL, GTK_FILL, 5, 2);
  g_free (str);
}

static void closeit (ggobid *gg) {
  gint n;
/* loop over all d's? */
  datad *d = gg->current_display->d;

  for (n=0; n<d->nclusters; n++)
    cluster_free (n, d, gg);

  gtk_widget_destroy (gg->exclusion_window);

  gg->exclusion_window = NULL;
}

void
reset_cluster_table (datad *d, ggobid *gg) {
  gint k, table_rows;

  /*-- destroy all the widgets that were in the table --*/
  table_rows = GTK_TABLE (d->exclusion_table)->nrows;
  for (k=0; k<table_rows-1; k++)
    cluster_free (k, d, gg);

  clusters_set (d, gg);

  gtk_table_resize (GTK_TABLE (d->exclusion_table), 1, 4);
  gtk_table_resize (GTK_TABLE (d->exclusion_table), d->nclusters+1, 4);

  /*-- add the cluster rows, one by one --*/
  for (k=0; k<d->nclusters; k++)
    exclusion_cluster_add (k, d, gg);
}

/*-- to reset it, just start fresh --*/
static void reset_cb (GtkWidget *w, ggobid *gg) {
  exclusion_window_open (gg);
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

  GtkWidget *vbox, *ebox, *btn, *hbox;
  gint k;
  GSList *l;
  datad *d;

  /*-- if it isn't NULL, then destroy it and start afresh --*/
  if (gg->exclusion_window != NULL) {
    closeit (gg);
  }

  gg->exclusion_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect (GTK_OBJECT (gg->exclusion_window), "delete_event",
                      GTK_SIGNAL_FUNC (delete_cb), (gpointer) gg);
  gtk_window_set_title (GTK_WINDOW (gg->exclusion_window),
    "ggobi hide/exclude window");

  vbox = gtk_vbox_new (false, 2);
  gtk_container_add (GTK_CONTAINER (gg->exclusion_window), vbox);

  ebox = gtk_event_box_new ();
  gtk_box_pack_start (GTK_BOX (vbox), ebox, false, false, 0);

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    clusters_set (d, gg);

    d->exclusion_table = gtk_table_new (d->nclusters+1, 4, false);
    gtk_container_add (GTK_CONTAINER (ebox), d->exclusion_table);

    /*-- add the row of titles --*/

    gtk_table_attach (GTK_TABLE (d->exclusion_table),
      gtk_label_new ("Symbol"),
      0, 1, 0, 1,  /*-- left, right, top, bottom --*/
      GTK_FILL, GTK_FILL, 5, 2);
    gtk_table_attach (GTK_TABLE (d->exclusion_table),
      gtk_label_new ("Hidden"),
      1, 2, 0, 1, GTK_FILL, GTK_FILL, 5, 2);
    gtk_table_attach (GTK_TABLE (d->exclusion_table),
      gtk_label_new ("Excluded"),
      2, 3, 0, 1, GTK_FILL, GTK_FILL, 5, 2);
    gtk_table_attach (GTK_TABLE (d->exclusion_table),
      gtk_label_new ("N elements"),
      3, 4, 0, 1, GTK_FILL, GTK_FILL, 5, 2);

    /*-- add the cluster rows, one by one --*/
    for (k=0; k<d->nclusters; k++)
      exclusion_cluster_add (k, d, gg);

    hbox = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 0);
  }

  /*-- Close button --*/
  btn = gtk_button_new_with_label ("Reset");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (reset_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (hbox), btn, true, true, 0);

  /*-- Close button --*/
  btn = gtk_button_new_with_label ("Close");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                 GTK_SIGNAL_FUNC (close_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (hbox), btn, true, true, 0);

  gtk_widget_show_all (gg->exclusion_window);
  gdk_window_raise (gg->exclusion_window->window);
}
