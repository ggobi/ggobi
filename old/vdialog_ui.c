/* vardialog_ui.c */ 
/* let the user select from a list of variables */

#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static void OK_cb (GtkWidget *w, GtkWidget *list)
{
/*  ggobid *gg = GGobiFromWidget (w, true);*/
}
static void cancel_cb (GtkWidget *w, GtkWidget *list)
{
/*  ggobid *gg = GGobiFromWidget (w, true);*/
  gtk_widget_destroy (gtk_widget_get_toplevel (w));
}
static void clear_cb (GtkWidget *w, GtkWidget *list)
{
/*  ggobid *gg = GGobiFromWidget (w, true);*/
  gtk_clist_unselect_all (GTK_CLIST (list));
}

void
vardialog_row_append (gint j, GtkWidget *list, ggobid *gg)
{
  gint k;
  gchar **row = (gchar **) g_malloc (2 * sizeof (gchar *));
  datad *d = gg->current_display->d;

  row[0] = g_strdup_printf ("%d", j);
  row[1] = g_strdup (d->vardata[j].collab);

  gtk_clist_append ((GtkCList *) list, row);

  for (k=0; k<2; k++)
    g_free ((gpointer) row[k]);
  g_free ((gpointer) row);
}

void
vardialog_open (ggobid *gg, gchar *title)
{                                  
  gint j;
  GtkWidget *vbox, *hbox, *list, *lbl, *btn;
  GtkWidget *window, *scrolled_window;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Select variables");

  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  lbl =  gtk_label_new (title);
  gtk_box_pack_start (GTK_BOX (vbox), lbl, false, false, 0);
    
  /* Create a scrolled window to pack the CList widget into */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, true, true, 0);
  gtk_widget_show (scrolled_window);

  list = gtk_clist_new (2);
  gtk_clist_set_selection_mode (GTK_CLIST (list), GTK_SELECTION_MULTIPLE);

  /*-- set the column width automatically --*/
  gtk_clist_set_column_auto_resize (GTK_CLIST (list), 1, true);

  /*-- populate the table --*/
  for (j=0 ; j<gg->ncols ; j++)
    vardialog_row_append (j, list, gg);

  /* It isn't necessary to shadow the border, but it looks nice */
  gtk_clist_set_shadow_type (GTK_CLIST (list), GTK_SHADOW_OUT);

  gtk_container_add (GTK_CONTAINER (scrolled_window), list);
  gtk_widget_show (list);

  hbox = gtk_hbox_new (true, 5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 5);

  btn = gtk_button_new_with_label ("OK");
  g_object_set_data(G_OBJECT (btn), "GGobi", (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 10);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (OK_cb), list);
  btn = gtk_button_new_with_label ("Clear");
  g_object_set_data(G_OBJECT (btn), "GGobi", (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 10);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (clear_cb), list);
  btn = gtk_button_new_with_label ("Cancel");
  g_object_set_data(G_OBJECT (btn), "GGobi", (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 10);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (cancel_cb), list);

  gtk_widget_set_usize (GTK_WIDGET (scrolled_window),
    list->requisition.width + 3 +
    GTK_SCROLLED_WINDOW (scrolled_window)->vscrollbar->requisition.width,
    150);

  gtk_widget_show_all (window);
}
